#!/usr/bin/env python3
"""Train the Stickshark policy/value ResNet on selfplay .bin files and export
weights in the SSNN v1 format that src/nn/network.cpp loads.

usage:
  python train.py                                  # train on data/selfplay/*.bin
  python train.py --glob 'data/selfplay/batch1_*.bin' --epochs 3
  python train.py --channels 64 --blocks 6 --out data/nets/net001.bin

Data format (see src/selfplay/selfplay.h): 8-byte header (magic 'SSSP', version),
then packed 104-byte records. Everything is side-to-move oriented.

Weight format (see src/nn/network.h): int32 magic 'SSNN', version 1, C, B, then
float32 tensors in PyTorch memory order; each conv is followed by its BatchNorm
gamma/beta/mean/var (folded into the conv by the C++ loader).
"""

import argparse
import glob as globmod
import os
import struct
import sys

import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F

PLANES = 19
POLICY_SIZE = 4096
DATA_MAGIC = 0x50535353
NET_MAGIC = 0x53534E4E

RECORD_DTYPE = np.dtype([
    ("bb",          "<u8", 12),
    ("flags",       "u1"),
    ("epColumn",    "u1"),
    ("policyIndex", "<i2"),
    ("scoreCp",     "<i2"),
    ("result",      "i1"),
    ("pad",         "u1"),
])
assert RECORD_DTYPE.itemsize == 104


# ---------------------------------------------------------------- data

def load_records(patterns):
    files = sorted(f for p in patterns for f in globmod.glob(p))
    if not files:
        sys.exit(f"no files match {patterns}")
    parts = []
    for f in files:
        with open(f, "rb") as fh:
            magic, version = struct.unpack("<Ii", fh.read(8))
        if magic != DATA_MAGIC or version != 1:
            sys.exit(f"{f}: bad magic/version")
        recs = np.fromfile(f, dtype=RECORD_DTYPE, offset=8)
        parts.append(recs)
        print(f"  {f}: {len(recs)} records")
    recs = np.concatenate(parts)

    # dedup across files on the position bytes (bb + flags + epColumn)
    pos_bytes = np.ascontiguousarray(recs[["bb", "flags", "epColumn"]])
    _, idx = np.unique(pos_bytes.view(f"V{pos_bytes.dtype.itemsize}").ravel(),
                       return_index=True)
    dropped = len(recs) - len(idx)
    recs = recs[np.sort(idx)]
    print(f"loaded {len(recs)} unique positions ({dropped} cross-file dups dropped)")
    return recs


def decode_batch(recs):
    """Structured array slice -> (planes[n,19,8,8], policy[n], value_target parts)."""
    n = len(recs)
    planes = np.zeros((n, PLANES, 64), dtype=np.float32)

    # planes 0-11: bit s of bb[i] -> plane i, square s (LSB-first within bytes)
    bits = np.unpackbits(recs["bb"].view(np.uint8).reshape(n, 12, 8),
                         axis=2, bitorder="little")
    planes[:, :12] = bits

    flags = recs["flags"]
    for b in range(4):                       # planes 12-15: castling, whole plane
        planes[:, 12 + b] = ((flags >> b) & 1).astype(np.float32)[:, None]

    ep = recs["epColumn"]                    # plane 16: en passant column
    valid = np.nonzero(ep != 255)[0]
    if len(valid):
        p16 = planes[:, 16].reshape(n, 8, 8)
        p16[valid, :, ep[valid].astype(np.int64)] = 1.0

    planes[:, 17] = ((flags >> 4) & 1).astype(np.float32)[:, None]   # repetition
    planes[:, 18] = 1.0                                              # all ones

    return planes.reshape(n, PLANES, 8, 8)


# ---------------------------------------------------------------- model
# Must mirror src/nn/network.h exactly, including where ReLU is applied:
# conv1x1 in network.cpp always applies ReLU after the (folded) BN.

class ResBlock(nn.Module):
    def __init__(self, c):
        super().__init__()
        self.conv1 = nn.Conv2d(c, c, 3, padding=1, bias=False)
        self.bn1 = nn.BatchNorm2d(c)
        self.conv2 = nn.Conv2d(c, c, 3, padding=1, bias=False)
        self.bn2 = nn.BatchNorm2d(c)

    def forward(self, x):
        y = F.relu(self.bn1(self.conv1(x)))
        y = self.bn2(self.conv2(y))
        return F.relu(x + y)


class Net(nn.Module):
    def __init__(self, channels, blocks):
        super().__init__()
        self.channels, self.blocks = channels, blocks
        self.stem = nn.Conv2d(PLANES, channels, 3, padding=1, bias=False)
        self.stem_bn = nn.BatchNorm2d(channels)
        self.tower = nn.ModuleList(ResBlock(channels) for _ in range(blocks))
        self.pol_conv = nn.Conv2d(channels, 2, 1, bias=False)
        self.pol_bn = nn.BatchNorm2d(2)
        self.pol_fc = nn.Linear(128, POLICY_SIZE)
        self.val_conv = nn.Conv2d(channels, 1, 1, bias=False)
        self.val_bn = nn.BatchNorm2d(1)
        self.val_fc1 = nn.Linear(64, 256)
        self.val_fc2 = nn.Linear(256, 1)

    def forward(self, x):
        x = F.relu(self.stem_bn(self.stem(x)))
        for block in self.tower:
            x = block(x)
        p = F.relu(self.pol_bn(self.pol_conv(x))).flatten(1)   # [n,2,8,8] -> [n,128]
        policy = self.pol_fc(p)
        v = F.relu(self.val_bn(self.val_conv(x))).flatten(1)   # [n,1,8,8] -> [n,64]
        v = F.relu(self.val_fc1(v))
        value = torch.tanh(self.val_fc2(v)).squeeze(1)
        return policy, value


# ---------------------------------------------------------------- export

def export_ssnn(model, path):
    """Write weights in the exact order network.cpp reads them."""
    model = model.to("cpu").eval()

    def w(t):
        f.write(t.detach().numpy().astype("<f4").tobytes())

    def conv_bn(conv, bn):
        w(conv.weight)
        w(bn.weight); w(bn.bias); w(bn.running_mean); w(bn.running_var)

    os.makedirs(os.path.dirname(path) or ".", exist_ok=True)
    with open(path, "wb") as f:
        f.write(struct.pack("<iiii", NET_MAGIC, 1, model.channels, model.blocks))
        conv_bn(model.stem, model.stem_bn)
        for blk in model.tower:
            conv_bn(blk.conv1, blk.bn1)
            conv_bn(blk.conv2, blk.bn2)
        conv_bn(model.pol_conv, model.pol_bn)
        w(model.pol_fc.weight); w(model.pol_fc.bias)
        conv_bn(model.val_conv, model.val_bn)
        w(model.val_fc1.weight); w(model.val_fc1.bias)
        w(model.val_fc2.weight); w(model.val_fc2.bias)
    print(f"exported {path} ({os.path.getsize(path)} bytes)")


# ---------------------------------------------------------------- training

def batches(recs, indices, batch_size, lam, device, shuffle):
    if shuffle:
        indices = indices[np.random.permutation(len(indices))]
    for i in range(0, len(indices), batch_size):
        chunk = recs[indices[i:i + batch_size]]
        planes = torch.from_numpy(decode_batch(chunk)).to(device)
        policy = torch.from_numpy(chunk["policyIndex"].astype(np.int64)).to(device)
        # value target: blend of game result and search eval, both in [-1, 1]
        vt = (lam * chunk["result"].astype(np.float32)
              + (1.0 - lam) * np.tanh(chunk["scoreCp"].astype(np.float32) / 350.0))
        yield planes, policy, torch.from_numpy(vt).to(device)


@torch.no_grad()
def evaluate(model, recs, indices, batch_size, lam, device):
    model.eval()
    ce = acc = mse = n = 0
    for planes, policy, vt in batches(recs, indices, batch_size, lam, device, False):
        pl, vl = model(planes)
        ce += F.cross_entropy(pl, policy, reduction="sum").item()
        acc += (pl.argmax(1) == policy).sum().item()
        mse += F.mse_loss(vl, vt, reduction="sum").item()
        n += len(policy)
    model.train()
    return ce / n, acc / n, mse / n


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--glob", nargs="*", default=["data/selfplay/*.bin"])
    ap.add_argument("--channels", type=int, default=64)
    ap.add_argument("--blocks", type=int, default=6)
    ap.add_argument("--epochs", type=int, default=3)
    ap.add_argument("--batch", type=int, default=512)
    ap.add_argument("--lr", type=float, default=1e-3)
    ap.add_argument("--lam", type=float, default=0.5,
                    help="value target = lam*result + (1-lam)*tanh(cp/350)")
    ap.add_argument("--val-frac", type=float, default=0.05)
    ap.add_argument("--seed", type=int, default=0)
    ap.add_argument("--out", default="data/nets/net001.bin")
    args = ap.parse_args()

    np.random.seed(args.seed)
    torch.manual_seed(args.seed)

    device = ("mps" if torch.backends.mps.is_available()
              else "cuda" if torch.cuda.is_available() else "cpu")
    print(f"device: {device}")

    recs = load_records(args.glob)
    perm = np.random.permutation(len(recs))
    n_val = max(1, int(len(recs) * args.val_frac))
    val_idx, train_idx = perm[:n_val], perm[n_val:]
    print(f"train {len(train_idx)}, val {len(val_idx)}")

    model = Net(args.channels, args.blocks).to(device)
    n_params = sum(p.numel() for p in model.parameters())
    print(f"model: C={args.channels} B={args.blocks}, {n_params/1e6:.2f}M params")

    opt = torch.optim.Adam(model.parameters(), lr=args.lr)
    sched = torch.optim.lr_scheduler.CosineAnnealingLR(
        opt, T_max=args.epochs * (len(train_idx) // args.batch + 1))

    best_val = float("inf")
    steps = 0
    for epoch in range(1, args.epochs + 1):
        for planes, policy, vt in batches(recs, train_idx, args.batch,
                                          args.lam, device, True):
            pl, vl = model(planes)
            loss = F.cross_entropy(pl, policy) + F.mse_loss(vl, vt)
            opt.zero_grad()
            loss.backward()
            opt.step()
            sched.step()
            steps += 1
            if steps % 200 == 0:
                print(f"epoch {epoch} step {steps}: loss {loss.item():.4f} "
                      f"lr {sched.get_last_lr()[0]:.2e}")

        ce, acc, mse = evaluate(model, recs, val_idx, args.batch, args.lam, device)
        val_loss = ce + mse
        print(f"epoch {epoch}: val policy CE {ce:.4f}, top-1 acc {acc:.3f}, "
              f"value MSE {mse:.4f}")
        if val_loss < best_val:
            best_val = val_loss
            export_ssnn(model, args.out)
            torch.save(model.state_dict(), args.out + ".pt")
            model.to(device)
        else:
            print("val loss did not improve; stopping early")
            break

    print(f"done. best val loss {best_val:.4f}; weights at {args.out}")


if __name__ == "__main__":
    main()

#!/bin/bash
# Run N selfplay workers in parallel, each writing its own file.
# usage: scripts/selfplay_parallel.sh [workers] [gamesPerWorker] [baseSeed] [moveTimeMs]
# output: data/selfplay/selfplay_w<i>.bin (one per worker)
set -euo pipefail
cd "$(dirname "$0")/.."

WORKERS=${1:-4}
GAMES=${2:-3000}
BASESEED=${3:-123}
MOVETIME=${4:-15}
BIN=build/stickshark

[ -x "$BIN" ] || { echo "missing $BIN — build first"; exit 1; }
mkdir -p data/selfplay

# refuse to clobber a previous run's output — rename it to batch<N>_w*.bin first
if ls data/selfplay/selfplay_w*.bin >/dev/null 2>&1; then
    echo "error: data/selfplay/selfplay_w*.bin already exists."
    echo "rename it first, e.g.: for f in data/selfplay/selfplay_w*; do mv \"\$f\" \"\${f/selfplay_w/batchN_w}\"; done"
    exit 1
fi

echo "launching $WORKERS workers x $GAMES games (moveTime ${MOVETIME}ms)"
pids=()
for i in $(seq 1 "$WORKERS"); do
    out="data/selfplay/selfplay_w${i}.bin"
    seed=$(( BASESEED + i ))
    "$BIN" --selfplay "$GAMES" "$out" "$seed" "$MOVETIME" > "data/selfplay/selfplay_w${i}.log" 2>&1 &
    pids+=($!)
done

trap 'kill "${pids[@]}" 2>/dev/null' INT TERM

fail=0
for pid in "${pids[@]}"; do
    wait "$pid" || fail=1
done

echo "--- results ---"
for i in $(seq 1 "$WORKERS"); do
    tail -1 "data/selfplay/selfplay_w${i}.log"
done

total=0
for i in $(seq 1 "$WORKERS"); do
    sz=$(stat -f %z "data/selfplay/selfplay_w${i}.bin")
    total=$(( total + (sz - 8) / 104 ))
done
echo "total: $total records across $WORKERS files"
[ "$fail" -eq 0 ] || { echo "warning: at least one worker exited nonzero"; exit 1; }

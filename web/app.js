import { Chess } from "./vendor/chess.js";

const PIECES = "assets/pieces/merida";
const FILES = ["a", "b", "c", "d", "e", "f", "g", "h"];

// ---- engine transport (UCI over the Python bridge) ----------------------
function send(cmd) {
  fetch("/send", { method: "POST", body: cmd }).catch(() => {});
}

// ---- app state ----------------------------------------------------------
const game = new Chess();
let humanColor = "w";
let orientation = "w";
let movesUci = [];
let startFen = null;           // null = startpos; otherwise the loaded FEN
let thinkMs = 1000;
let engineThinking = false;

let selected = null;
let legalTargets = new Map();   // toSquare -> verbose move
let lastMove = null;            // { from, to }
let drag = null;                // active drag descriptor

// ---- dom ----------------------------------------------------------------
const boardEl = document.getElementById("board");
const statusEl = document.getElementById("status");
const promotionEl = document.getElementById("promotion");
const moveListEl = document.getElementById("moveList");

const iDepth = document.getElementById("iDepth");
const iScore = document.getElementById("iScore");
const iNodes = document.getElementById("iNodes");
const iNps = document.getElementById("iNps");
const iPv = document.getElementById("iPv");
const engineInfoEl = document.querySelector(".engine-info");
const evalFill = document.getElementById("evalFill");
const evalLabel = document.getElementById("evalLabel");

// ---- board rendering ----------------------------------------------------
function orderedSquares() {
  const ranks = orientation === "w" ? [8, 7, 6, 5, 4, 3, 2, 1] : [1, 2, 3, 4, 5, 6, 7, 8];
  const files = orientation === "w" ? FILES : [...FILES].reverse();
  const out = [];
  for (let r = 0; r < 8; r++)
    for (let f = 0; f < 8; f++)
      out.push({ square: files[f] + ranks[r], row: r, col: f, file: files[f], rank: ranks[r] });
  return out;
}

function kingSquare(color) {
  for (const row of game.board())
    for (const p of row)
      if (p && p.type === "k" && p.color === color) return p.square;
  return null;
}

function render() {
  boardEl.innerHTML = "";
  const checkSq = game.inCheck() ? kingSquare(game.turn()) : null;

  for (const cell of orderedSquares()) {
    const div = document.createElement("div");
    const fileIdx = FILES.indexOf(cell.file);
    const light = (fileIdx + cell.rank - 1) % 2 === 1;
    div.className = "sq " + (light ? "light" : "dark");
    div.dataset.square = cell.square;

    if (selected === cell.square) div.classList.add("selected");
    if (lastMove && (lastMove.from === cell.square || lastMove.to === cell.square))
      div.classList.add("lastmove");
    if (checkSq === cell.square) div.classList.add("check");
    if (legalTargets.has(cell.square)) {
      div.classList.add("target");
      if (game.get(cell.square)) div.classList.add("capture");
    }

    if (cell.row === 7) div.insertAdjacentHTML("beforeend", `<span class="coord file">${cell.file}</span>`);
    if (cell.col === 0) div.insertAdjacentHTML("beforeend", `<span class="coord rank">${cell.rank}</span>`);

    const piece = game.get(cell.square);
    if (piece) {
      const img = document.createElement("img");
      img.src = `${PIECES}/${piece.color}${piece.type.toUpperCase()}.svg`;
      img.draggable = false;
      div.appendChild(img);
    }
    boardEl.appendChild(div);
  }
}

// ---- selection & interaction -------------------------------------------
function humansTurn() {
  return game.turn() === humanColor && !engineThinking && !game.isGameOver();
}

function selectSquare(sq) {
  selected = sq;
  legalTargets = new Map();
  for (const m of game.moves({ square: sq, verbose: true })) legalTargets.set(m.to, m);
  render();
}

function clearSelection() {
  selected = null;
  legalTargets = new Map();
  render();
}

function squareAt(x, y) {
  const el = document.elementFromPoint(x, y);
  const sq = el && el.closest(".sq");
  return sq ? sq.dataset.square : null;
}

boardEl.addEventListener("pointerdown", (e) => {
  const sq = squareAt(e.clientX, e.clientY);
  if (!sq) return;

  if (selected && legalTargets.has(sq)) {
    completeMove(selected, sq);
    return;
  }
  const piece = game.get(sq);
  if (piece && piece.color === humanColor && humansTurn()) {
    selectSquare(sq);
    startDrag(e, sq, piece);
  } else {
    clearSelection();
  }
});

function startDrag(e, from, piece) {
  const ghost = document.createElement("div");
  ghost.className = "drag-piece";
  const size = boardEl.getBoundingClientRect().width / 8;
  ghost.style.width = ghost.style.height = size + "px";
  ghost.innerHTML = `<img src="${PIECES}/${piece.color}${piece.type.toUpperCase()}.svg" />`;
  document.body.appendChild(ghost);
  drag = { from, ghost, moved: false };
  moveGhost(e.clientX, e.clientY);

  const srcImg = boardEl.querySelector(`.sq[data-square="${from}"] img`);
  if (srcImg) srcImg.style.opacity = "0";

  boardEl.setPointerCapture(e.pointerId);
  boardEl.addEventListener("pointermove", onDragMove);
  boardEl.addEventListener("pointerup", onDragEnd);
}

function moveGhost(x, y) {
  drag.ghost.style.left = x + "px";
  drag.ghost.style.top = y + "px";
}

function onDragMove(e) {
  if (!drag) return;
  drag.moved = true;
  moveGhost(e.clientX, e.clientY);
}

function onDragEnd(e) {
  if (!drag) return;
  const target = squareAt(e.clientX, e.clientY);
  drag.ghost.remove();
  const srcImg = boardEl.querySelector(`.sq[data-square="${drag.from}"] img`);
  if (srcImg) srcImg.style.opacity = "";   // restore if no re-render follows (click-select)
  boardEl.removeEventListener("pointermove", onDragMove);
  boardEl.removeEventListener("pointerup", onDragEnd);
  const from = drag.from;
  const moved = drag.moved;
  drag = null;

  if (target && target !== from && legalTargets.has(target)) {
    completeMove(from, target);
  } else if (moved && target !== from) {
    clearSelection();       // dragged off to nowhere
  }
  // releasing on the origin square (plain click, or drag returned home,
  // including tiny accidental jiggles) keeps the piece selected for click-to-move
}

// ---- making moves -------------------------------------------------------
function completeMove(from, to) {
  const candidate = legalTargets.get(to);
  if (candidate && candidate.promotion) {
    showPromotion(from, to);
  } else {
    doMove(from, to);
  }
}

function doMove(from, to, promotion) {
  const move = game.move({ from, to, promotion });
  if (!move) return;
  movesUci.push(from + to + (promotion || ""));
  lastMove = { from, to };
  clearSelection();
  rebuildMoveList();
  updateStatus();

  if (!game.isGameOver() && game.turn() !== humanColor) requestEngineMove();
}

function showPromotion(from, to) {
  const toFile = to[0];
  const colVisual = orientation === "w" ? FILES.indexOf(toFile) : 7 - FILES.indexOf(toFile);
  const promotingColor = game.turn();
  promotionEl.innerHTML = "";
  for (const t of ["q", "r", "n", "b"]) {
    const img = document.createElement("img");
    img.src = `${PIECES}/${promotingColor}${t.toUpperCase()}.svg`;
    img.onclick = () => {
      promotionEl.hidden = true;
      doMove(from, to, t);
    };
    promotionEl.appendChild(img);
  }
  promotionEl.style.left = colVisual * 12.5 + "%";
  promotionEl.style.top = "0";
  promotionEl.hidden = false;
}

// ---- engine flow --------------------------------------------------------
function positionCommand() {
  const base = startFen ? "position fen " + startFen : "position startpos";
  return base + (movesUci.length ? " moves " + movesUci.join(" ") : "");
}

// Watchdog: SSE delivery isn't guaranteed (tab sleep, connection drops can
// eat the bestmove line), so every request re-asks the engine if no usable
// reply lands in time. Duplicate replies are dropped by the guard in
// applyEngineMove, which makes re-asking always safe.
let engineWatchdog = null;
let engineRetries = 0;

function requestEngineMove() {
  engineThinking = true;
  engineRetries = 0;
  updateStatus();
  send(positionCommand());
  send("go movetime " + thinkMs);
  armEngineWatchdog();
}

function armEngineWatchdog() {
  clearTimeout(engineWatchdog);
  engineWatchdog = setTimeout(() => {
    if (!engineThinking) return;
    if (engineRetries >= 3) {
      engineThinking = false;
      updateStatus();
      statusEl.textContent = "Engine not responding — try New game";
      return;
    }
    engineRetries++;
    send(positionCommand());
    send("go movetime " + thinkMs);
    armEngineWatchdog();
  }, thinkMs + 8000);
}

function applyEngineMove(uci) {
  if (!engineThinking) return;   // unsolicited or duplicate reply — drop it
  if (!uci || uci === "(none)") {
    engineThinking = false;
    clearTimeout(engineWatchdog);
    updateStatus();
    return;
  }
  let move = null;
  try {
    move = game.move({
      from: uci.slice(0, 2),
      to: uci.slice(2, 4),
      promotion: uci.length > 4 ? uci[4] : undefined,
    });
  } catch { /* rejected reply — leave engineThinking set; watchdog re-asks */ }
  if (!move) return;
  engineThinking = false;
  clearTimeout(engineWatchdog);
  movesUci.push(uci);
  lastMove = { from: move.from, to: move.to };
  clearSelection();
  rebuildMoveList();
  updateStatus();
}

// ---- engine output (SSE) ------------------------------------------------
const events = new EventSource("/events");
events.onmessage = (e) => handleEngineLine(e.data);

function handleEngineLine(line) {
  if (line.startsWith("id name")) {
    document.getElementById("engineId").textContent = line.slice(8).trim();
  } else if (line.startsWith("info string movedist ")) {
    renderMoveDist(line.slice("info string movedist ".length));
  } else if (line.startsWith("info")) {
    parseInfo(line);
  } else if (line.startsWith("bestmove")) {
    applyEngineMove(line.split(/\s+/)[1]);
  }
}

// ---- move distribution (MCTS root visit counts) --------------------------
const moveDistEl = document.getElementById("moveDist");
const moveDistBarsEl = document.getElementById("moveDistBars");
const MAX_DIST_ROWS = 8;

function clearMoveDist() {
  moveDistEl.hidden = true;
  moveDistBarsEl.innerHTML = "";
}

function renderMoveDist(payload) {
  const entries = payload.trim().split(/\s+/).map((p) => {
    const [uci, v, q] = p.split(":");
    return { uci, visits: +v, q: q !== undefined ? +q : null };
  }).filter((e) => e.uci && e.visits > 0);
  if (!entries.length) { clearMoveDist(); return; }

  const total = entries.reduce((s, e) => s + e.visits, 0);
  const top = entries.slice(0, MAX_DIST_ROWS);
  const maxVisits = top[0].visits;

  // The engine searched the current game position, so SAN conversion is
  // valid here (this arrives before the bestmove line is applied).
  const probe = new Chess(game.fen());

  moveDistBarsEl.innerHTML = "";
  for (const e of top) {
    let label = e.uci;
    try {
      const mv = probe.move({ from: e.uci.slice(0, 2), to: e.uci.slice(2, 4), promotion: e.uci.length > 4 ? e.uci[4] : undefined });
      if (mv) { label = mv.san; probe.undo(); }
    } catch { /* fall back to coordinate notation */ }

    const pct = (e.visits / total) * 100;
    const row = document.createElement("div");
    row.className = "dist-row";
    // Q is from the engine's perspective; ~pawns via the inverse of tanh(cp/300)
    let qNote = "";
    if (e.q !== null && !Number.isNaN(e.q)) {
      const qc = Math.max(-0.9999, Math.min(0.9999, e.q));
      const pawns = (Math.atanh(qc) * 300) / 100;
      qNote = ` — Q ${e.q >= 0 ? "+" : ""}${e.q.toFixed(3)} (≈${pawns >= 0 ? "+" : ""}${pawns.toFixed(1)} pawns for engine)`;
    }
    row.title = `${label}: ${e.visits} of ${total} simulations${qNote}`;

    const moveSpan = document.createElement("span");
    moveSpan.className = "dist-move";
    moveSpan.textContent = label;

    const track = document.createElement("div");
    track.className = "dist-track";
    const fill = document.createElement("div");
    fill.className = "dist-fill";
    fill.style.width = ((e.visits / maxVisits) * 100).toFixed(1) + "%";
    track.appendChild(fill);

    const pctSpan = document.createElement("span");
    pctSpan.className = "dist-pct";
    pctSpan.textContent = pct.toFixed(1) + "%";

    row.append(moveSpan, track, pctSpan);
    moveDistBarsEl.appendChild(row);
  }
  moveDistEl.hidden = false;
}

function parseInfo(line) {
  engineInfoEl.classList.add("live");   // brighten the readout once data arrives
  const t = line.split(/\s+/);
  let depth, nodes, nps, scoreCp, mate, pv;
  for (let i = 1; i < t.length; i++) {
    if (t[i] === "depth") depth = +t[++i];
    else if (t[i] === "nodes") nodes = +t[++i];
    else if (t[i] === "nps") nps = +t[++i];
    else if (t[i] === "score") {
      if (t[i + 1] === "cp") scoreCp = +t[i + 2];
      else if (t[i + 1] === "mate") mate = +t[i + 2];
      i += 2;
    } else if (t[i] === "pv") { pv = t.slice(i + 1); break; }
  }
  if (depth !== undefined) iDepth.textContent = depth;
  if (nodes !== undefined) iNodes.textContent = fmtNum(nodes);
  if (nps !== undefined) iNps.textContent = fmtNum(nps) + "/s";
  if (pv) iPv.textContent = pv.join(" ");

  // Stickshark already reports scores from White's perspective (see
  // engine.cpp: scoreCp = board.turn ? boardEval : -boardEval), so use as-is.
  if (mate !== undefined) {
    iScore.textContent = "#" + (mate < 0 ? "-" : "") + Math.abs(mate);
    setEval(mate > 0 ? 100000 : -100000, "#" + (mate < 0 ? "-" : "") + Math.abs(mate));
  } else if (scoreCp !== undefined) {
    iScore.textContent = (scoreCp >= 0 ? "+" : "") + (scoreCp / 100).toFixed(2);
    setEval(scoreCp);
  }
}

function setEval(whiteCp, label) {
  const winProb = 1 / (1 + Math.pow(10, -whiteCp / 400));
  evalFill.style.height = (winProb * 100).toFixed(1) + "%";
  const blackAhead = whiteCp < 0;
  // Black ahead: label sits at the top (dark side) with a minus sign.
  // White ahead: label sits at the bottom (white side), no sign.
  evalLabel.classList.toggle("top", blackAhead);
  evalLabel.textContent = label || (blackAhead ? "-" : "") + (Math.abs(whiteCp) / 100).toFixed(1);
}

// ---- status & move list -------------------------------------------------
function updateStatus() {
  statusEl.classList.remove("over");
  if (game.isCheckmate()) {
    statusEl.classList.add("over");
    statusEl.textContent = (game.turn() === "w" ? "Black" : "White") + " wins by checkmate";
  } else if (game.isStalemate()) {
    statusEl.classList.add("over"); statusEl.textContent = "Draw by stalemate";
  } else if (game.isThreefoldRepetition()) {
    statusEl.classList.add("over"); statusEl.textContent = "Draw by repetition";
  } else if (game.isInsufficientMaterial()) {
    statusEl.classList.add("over"); statusEl.textContent = "Draw by insufficient material";
  } else if (game.isDraw()) {
    statusEl.classList.add("over"); statusEl.textContent = "Draw by fifty-move rule";
  } else if (engineThinking) {
    statusEl.textContent = "Stickshark is thinking…";
  } else if (game.turn() === humanColor) {
    statusEl.textContent = "Your move" + (game.inCheck() ? ", check!" : "");
  } else {
    statusEl.textContent = "Stickshark to move";
  }
}

function fmtNum(n) {
  if (n >= 1e6) return (n / 1e6).toFixed(1) + "M";
  if (n >= 1e3) return (n / 1e3).toFixed(1) + "k";
  return "" + n;
}

// ---- controls -----------------------------------------------------------
function resetInfoPanel() {
  iDepth.textContent = iScore.textContent = iNodes.textContent = iNps.textContent = "–";
  iPv.textContent = "–";
  engineInfoEl.classList.remove("live");   // back to muted, empty placeholders
  clearMoveDist();
  setEval(0);
}

// Shared setup for both a fresh game and a loaded FEN.
function startSession() {
  movesUci = [];
  lastMove = null;
  engineThinking = false;
  clearSelection();
  moveListEl.innerHTML = "";
  resetInfoPanel();
  orientation = humanColor;
  send("ucinewgame");
  send("isready");
  render();
  updateStatus();
}

// Restart from the current setup (loaded FEN, or startpos) for the current
// humanColor. Used by New game, Load FEN, and switching sides.
function restartCurrent() {
  if (startFen) game.load(startFen); else game.reset();
  startSession();
  if (game.turn() !== humanColor) requestEngineMove();  // engine is on move
}

function newGame() {
  startFen = null;
  if (presetSel) presetSel.value = "";
  if (fenInput) fenInput.value = "";
  restartCurrent();
}

function loadFen(fen) {
  fen = (fen || "").trim();
  if (!fen) { newGame(); return; }
  try {
    game.load(fen);              // validate before committing
  } catch (err) {
    alert("Invalid FEN:\n" + err.message);
    return;
  }
  startFen = fen;
  fenInput.value = fen;
  restartCurrent();
}

document.getElementById("newGame").onclick = newGame;
document.getElementById("flip").onclick = () => {
  orientation = orientation === "w" ? "b" : "w";
  render();
};
document.getElementById("undo").onclick = () => {
  if (engineThinking || !movesUci.length) return;
  // Undo back to the human's turn (engine reply + human move).
  const times = game.turn() === humanColor ? 2 : 1;
  for (let i = 0; i < times && movesUci.length; i++) { game.undo(); movesUci.pop(); }
  rebuildMoveList();
  lastMove = null;
  clearSelection();
  clearMoveDist();
  send(positionCommand());
  updateStatus();
};

document.querySelectorAll("#side button").forEach((btn) => {
  btn.onclick = () => {
    document.querySelectorAll("#side button").forEach((b) => b.classList.remove("active"));
    btn.classList.add("active");
    humanColor = btn.dataset.side;
    restartCurrent();   // keep the loaded position; just swap sides

  };
});

const thinkInput = document.getElementById("think");
const thinkVal = document.getElementById("thinkVal");
thinkInput.oninput = () => {
  thinkMs = +thinkInput.value;
  thinkVal.textContent = (thinkMs / 1000).toFixed(1) + "s";
};

const algoSel = document.getElementById("algo");
algoSel.onchange = () => {
  send("setoption name SearchAlgorithm value " + algoSel.value);
  resetInfoPanel();  // also clears the move distribution
  // MCTS reports no depth/score/nodes/pv, so the readout is dead weight there
  engineInfoEl.hidden = algoSel.value === "MCTS";
};

const presetSel = document.getElementById("preset");
const fenInput = document.getElementById("fenInput");
presetSel.onchange = () => {
  const v = presetSel.value;
  fenInput.value = v;
  if (v === "") newGame();      // "Starting position"
  else loadFen(v);
};
document.getElementById("loadFen").onclick = () => {
  presetSel.value = "";         // custom entry no longer matches a preset
  loadFen(fenInput.value);
};
fenInput.addEventListener("keydown", (e) => {
  if (e.key === "Enter") { presetSel.value = ""; loadFen(fenInput.value); }
});

function rebuildMoveList() {
  moveListEl.innerHTML = "";
  const replay = new Chess(startFen || undefined);
  let row = null;
  for (const u of movesUci) {
    const num = replay.moveNumber();
    const white = replay.turn() === "w";
    const mv = replay.move({ from: u.slice(0, 2), to: u.slice(2, 4), promotion: u.length > 4 ? u[4] : undefined });
    if (!mv) break;
    if (white) {
      row = document.createElement("tr");
      row.innerHTML = `<td class="num">${num}.</td><td class="mv">${mv.san}</td><td class="mv"></td>`;
      moveListEl.appendChild(row);
    } else {
      if (!row) {   // Black to move first (a loaded FEN): pad the White cell.
        row = document.createElement("tr");
        row.innerHTML = `<td class="num">${num}.</td><td class="mv">…</td><td class="mv"></td>`;
        moveListEl.appendChild(row);
      }
      row.children[2].textContent = mv.san;
      row = null;
    }
  }
  moveListEl.parentElement.parentElement.scrollTop = 1e9;
}

// ---- boot ---------------------------------------------------------------
let sseConnectedOnce = false;
events.onopen = () => {
  if (!sseConnectedOnce) {
    sseConnectedOnce = true;
    send("uci");
    newGame();
    return;
  }
  // Reconnected mid-session: the server may have respawned the engine, so
  // re-sync the option. A lost bestmove is handled by the engine watchdog.
  send("setoption name SearchAlgorithm value " + algoSel.value);
};
render();
updateStatus();

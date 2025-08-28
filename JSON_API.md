# Stickshark Chess Engine - JSON API Reference

## Usage

Run the engine in web mode:
```bash
./bin/main --web
```

Send commands via stdin, receive JSON responses via stdout.

## Commands

### Get Board State
```
getboard
```
**Response:**
```json
{
  "status": "success",
  "action": "getboard", 
  "board": [...],
  "turn": "white"
}
```

### New Game
```
newgame
```
**Response:**
```json
{
  "status": "success",
  "action": "newgame",
  "board": [...],
  "turn": "white"
}
```

### Make Player Move
```
move:e2-e4
```
**Response:**
```json
{
  "status": "success",
  "action": "move",
  "move": "e2-e4",
  "board": [...],
  "turn": "black"
}
```

### Engine Move
```
enginemove:1000
```
(Time in milliseconds)

**Response:**
```json
{
  "status": "success", 
  "action": "enginemove",
  "move": "d7-d5",
  "board": [...],
  "turn": "white"
}
```

### Exit
```
quit
```
or
```
exit
```

## Board Format

The board is a 2D array representing ranks 8-1 (top to bottom):
- `null` = empty square
- `{"type": 0-5, "color": "white|black"}` = piece

**Piece Types:**
- 0 = Pawn
- 1 = Bishop  
- 2 = Knight
- 3 = Rook
- 4 = Queen
- 5 = King

## Error Response
```json
{
  "status": "error",
  "message": "Error description"
}
```

## Example Session

```bash
echo -e "getboard\nmove:e2-e4\nenginemove:500\nquit" | ./bin/main --web
```

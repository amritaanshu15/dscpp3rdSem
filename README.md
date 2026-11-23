# EcoLife: Rabbit Run

EcoLife: Rabbit Run is a turn-based survival game set in a forest. Choose a small animal and try to survive for 50 days by finding food and water while avoiding predators. Each move advances the game by one day.

The browser displays the game and sends player actions to a local server. The game rules and state are handled by C and C++.

## Features

- Randomly generated forest maps
- Multiple playable animals and difficulty levels
- Predators that can chase the player
- Food and water survival mechanics
- Scheduled forest events, including drought, fire, and rain
- Danger radar and recent game messages
- High-score leaderboard
- Browser-based game and terminal mode

## Requirements

- GCC and G++
- GNU Make (`make` on macOS/Linux or `mingw32-make` with MinGW on Windows)
- A web browser for the browser version

## Build and Run

### macOS or Linux

From the project folder:

```bash
mkdir -p build
make
./build/rabbit_run

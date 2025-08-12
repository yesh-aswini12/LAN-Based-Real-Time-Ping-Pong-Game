# Ping-Pong Over TCP

## Description
This project implements a **two-player ping pong game** that runs over a **Local Area Network (LAN)** using **TCP sockets** in C.  
It features a **server-client architecture** where:
- **Player A (Server)** hosts the game and controls the bottom paddle.
- **Player B (Client)** connects to the server and controls the top paddle.

The game state — including paddle positions, ball position, and scores — is continuously exchanged between players to ensure **real-time synchronization**.  
The game graphics are implemented using the **ncurses** library.

---

## Features
- **Two-player gameplay** over LAN.
- Real-time **TCP socket communication** for game state updates.
- **ncurses-based** terminal graphics.
- Smooth paddle and ball movement.
- Live scoreboard showing penalties for each player.
- Quit option with a single key press.

---

## Game Rules
1. Both players use the **left** and **right arrow keys** to move their paddles.
2. The ball bounces between the paddles and walls.
3. If the ball passes a player’s paddle, the opponent scores a point.
4. The first to reach a set score (or after a decided time) can be declared the winner.
5. Press **`q`** at any time to quit the game.

---

## Compilation & Execution

### **Compile the Game**
```bash
gcc -o pingpong pingpong.c -pthread -lncurses
```

---

### **Run as Server (Player A)**
```bash
./pingpong server <PORT>
```

---

### **Run as Client (Player B)**
```bash
./pingpong client <SERVER_IP>
```

---

# 🕹️ OpenGL-Maze-Generator-Solver

A single-file, zero-dependency demo that creates a perfect maze with recursive back-tracking renders it in real-time using OpenGL 1.x / GLUT finds the shortest path via Breadth-First Search animates the solution step-by-step.

## 📦 Build & Run

| Platform | One-liner |
|----------|-----------|
| **macOS** (Xcode CLI tools required) | `clang++ maze.cc -framework OpenGL -framework GLUT -std=c++14 -o maze && ./maze` |
| **Linux** (Ubuntu / Debian) | `sudo apt install libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev && g++ maze.cc -lGL -lGLU -lglut -std=c++14 -o maze && ./maze` |
| **Windows** (MinGW-w64) | `g++ maze.cc -lopengl32 -lglu32 -lfreeglut -std=c++14 -o maze.exe && maze.exe` |
| **Windows** (Visual Studio) | `open Developer Command Prompt, link against freeglut (freeglut.lib opengl32.lib glu32.lib) and compile with cl /EHsc maze.cc.` |

## 🎮 Controls
|Key	|Action|
|----------|-----------|
|↑ ↓ ← →	| Move the blue player |
|Space	| Show shortest path (green) |
|A	|Start animated auto-solve|
|R	|Reset player & target|
|N	|Generate a brand-new maze|
|ESC	|Quit|

## 📸 Screenshot

<img width="1912" height="1228" alt="image" src="https://github.com/user-attachments/assets/d25522cd-e586-4c22-9d98-5f41f44cd3b1" />

## 📄 License
MIT – use it for education, game jams, or as a lightweight OpenGL starter project.

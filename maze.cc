/*
 * Random Maze Generator & Solver
 * Built with OpenGL/GLUT
 *
 * Controls:
 *  Arrow keys  – move the player
 *  Space       – show the shortest path (BFS)
 *  R           – reset current maze
 *  N           – generate a new maze
 *  A           – auto-solve (animated)
 *  ESC         – quit
 */

#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#include <vector>
#include <queue>
#include <stack>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <ctime>
#include <iostream>
#include <chrono>

const int CELL_SIZE   = 50;
const int MAZE_WIDTH  = 21;
const int MAZE_HEIGHT = 21;

enum CellType {
    WALL,
    PATH,
    PLAYER,
    TARGET
};

struct Cell {
    int x, y;
    CellType type;
    bool visited = false;
};

struct Vector2i {
    int x, y;
    Vector2i(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Vector2i& other) const {
        return x == other.x && y == other.y;
    }
};

struct Color {
    float r, g, b;
    Color(float r, float g, float b) : r(r), g(g), b(b) {}
};

class Maze {
public:
    Maze() {
        rng.seed(std::time(nullptr));
        cells.resize(MAZE_HEIGHT, std::vector<Cell>(MAZE_WIDTH));
        generateMaze();
        reset();
    }

    /* Generate a perfect maze using recursive back-tracking */
    void generateMaze() {
        for (int y = 0; y < MAZE_HEIGHT; y++) {
            for (int x = 0; x < MAZE_WIDTH; x++) {
                cells[y][x] = {x, y, WALL};
            }
        }

        cells[1][1].type = PATH;
        cells[MAZE_HEIGHT - 2][MAZE_WIDTH - 2].type = PATH;

        std::stack<Vector2i> stack;
        stack.push(Vector2i(1, 1));
        cells[1][1].visited = true;

        while (!stack.empty()) {
            Vector2i current = stack.top();
            int x = current.x;
            int y = current.y;

            std::vector<Vector2i> neighbors = getUnvisitedNeighbors(x, y);

            if (!neighbors.empty()) {
                std::uniform_int_distribution<int> dist(0, neighbors.size() - 1);
                Vector2i next = neighbors[dist(rng)];
                int nx = next.x;
                int ny = next.y;

                int midX = (x + nx) / 2;
                int midY = (y + ny) / 2;
                cells[midY][midX].type = PATH;
                cells[ny][nx].type = PATH;

                cells[ny][nx].visited = true;
                stack.push(Vector2i(nx, ny));
            } else {
                stack.pop();
            }
        }
    }

    /* Reset player & target positions, clear visited flags */
    void reset() {
        for (auto& row : cells) {
            for (auto& cell : row) {
                cell.visited = false;
                if (cell.type == PLAYER) cell.type = PATH;
            }
        }

        playerPos = Vector2i(1, 1);
        cells[playerPos.y][playerPos.x].type = PLAYER;

        targetPos = Vector2i(MAZE_WIDTH - 2, MAZE_HEIGHT - 2);
        cells[targetPos.y][targetPos.x].type = TARGET;

        pathFound        = false;
        autoMoving       = false;
        movePath.clear();
        currentMoveIndex = 0;
    }

    void generateNewMaze() {
        generateMaze();
        reset();
    }

    /* Attempt to move the player by (dx, dy) */
    bool movePlayer(int dx, int dy) {
        int newX = playerPos.x + dx;
        int newY = playerPos.y + dy;

        if (isValidPosition(newX, newY) && cells[newY][newX].type != WALL) {
            cells[playerPos.y][playerPos.x].type = PATH;
            playerPos = Vector2i(newX, newY);
            cells[newY][newX].type = PLAYER;
            return true;
        }
        return false;
    }

    /* Breadth-First Search to find the shortest path */
    void findPathBFS() {
        resetVisited();
        std::queue<Vector2i> queue;
        std::unordered_map<int, Vector2i> parentMap;

        queue.push(playerPos);
        cells[playerPos.y][playerPos.x].visited = true;

        while (!queue.empty()) {
            Vector2i current = queue.front();
            queue.pop();

            if (current == targetPos) {
                pathFound = true;
                reconstructPath(parentMap);
                return;
            }

            for (const auto& dir : directions) {
                int newX = current.x + dir.x;
                int newY = current.y + dir.y;

                if (isValidPosition(newX, newY) && !cells[newY][newX].visited &&
                    cells[newY][newX].type != WALL) {
                    cells[newY][newX].visited = true;
                    queue.push(Vector2i(newX, newY));
                    parentMap[newY * MAZE_WIDTH + newX] = current;
                }
            }
        }
    }

    /* Prepare the animated auto-solve */
    void prepareAutoMove() {
        findPathBFS();
        if (pathFound) {
            autoMoving       = true;
            currentMoveIndex = 0;
            movePath         = path;
            movePath.insert(movePath.begin(), playerPos);
        }
    }

    /* Execute one step of the auto-solve animation */
    bool autoMoveStep() {
        if (!autoMoving || currentMoveIndex >= movePath.size() - 1) {
            autoMoving = false;
            return false;
        }

        currentMoveIndex++;
        Vector2i nextPos = movePath[currentMoveIndex];

        cells[playerPos.y][playerPos.x].type = PATH;
        playerPos = nextPos;
        cells[playerPos.y][playerPos.x].type = PLAYER;

        if (playerPos == targetPos) autoMoving = false;
        return true;
    }

    /* Render the entire maze */
    void draw() {
        glClear(GL_COLOR_BUFFER_BIT);

        for (int y = 0; y < MAZE_HEIGHT; y++) {
            for (int x = 0; x < MAZE_WIDTH; x++) {
                Color color(0.3f, 0.3f, 0.3f);

                switch (cells[y][x].type) {
                    case WALL:   color = Color(0.3f, 0.3f, 0.3f); break;
                    case PATH:   color = Color(0.9f, 0.9f, 0.9f); break;
                    case PLAYER: color = Color(0.26f, 0.53f, 0.96f); break;
                    case TARGET: color = Color(0.96f, 0.26f, 0.26f); break;
                }
                drawCell(x, y, color);
            }
        }

        if (pathFound) {
            Color pathColor(0.26f, 0.96f, 0.68f);
            for (const auto& pos : path) {
                if (cells[pos.y][pos.x].type != PLAYER &&
                    cells[pos.y][pos.x].type != TARGET) {
                    drawPathCell(pos.x, pos.y, pathColor);
                }
            }
        }

        glutSwapBuffers();
    }

    bool isAutoMoving() const { return autoMoving; }

    static Maze* instance; // Global pointer for GLUT callbacks

private:
    std::vector<std::vector<Cell>> cells;
    std::vector<Vector2i> path;
    std::vector<Vector2i> movePath;
    Vector2i playerPos;
    Vector2i targetPos;
    bool pathFound  = false;
    bool autoMoving = false;
    int  currentMoveIndex = 0;
    std::mt19937 rng;

    std::vector<Vector2i> directions = {
        Vector2i(1, 0), Vector2i(-1, 0), Vector2i(0, 1), Vector2i(0, -1)
    };

    void drawCell(int x, int y, const Color& color) {
        glColor3f(color.r, color.g, color.b);
        glBegin(GL_QUADS);
        float x1 = x * CELL_SIZE;
        float y1 = y * CELL_SIZE;
        float x2 = x1 + CELL_SIZE - 1;
        float y2 = y1 + CELL_SIZE - 1;

        glVertex2f(x1, y1);
        glVertex2f(x2, y1);
        glVertex2f(x2, y2);
        glVertex2f(x1, y2);
        glEnd();
    }

    void drawPathCell(int x, int y, const Color& color) {
        glColor3f(color.r, color.g, color.b);
        glBegin(GL_QUADS);
        float centerX = x * CELL_SIZE + CELL_SIZE / 2.0f;
        float centerY = y * CELL_SIZE + CELL_SIZE / 2.0f;
        float halfSize = CELL_SIZE / 4.0f;

        glVertex2f(centerX - halfSize, centerY - halfSize);
        glVertex2f(centerX + halfSize, centerY - halfSize);
        glVertex2f(centerX + halfSize, centerY + halfSize);
        glVertex2f(centerX - halfSize, centerY + halfSize);
        glEnd();
    }

    bool isValidPosition(int x, int y) {
        return x >= 0 && x < MAZE_WIDTH && y >= 0 && y < MAZE_HEIGHT;
    }

    void resetVisited() {
        for (auto& row : cells)
            for (auto& cell : row)
                cell.visited = false;
        path.clear();
        pathFound = false;
    }

    void reconstructPath(std::unordered_map<int, Vector2i>& parentMap) {
        Vector2i current = targetPos;
        while (!(current == playerPos)) {
            path.push_back(current);
            current = parentMap[current.y * MAZE_WIDTH + current.x];
        }
        std::reverse(path.begin(), path.end());
    }

    std::vector<Vector2i> getUnvisitedNeighbors(int x, int y) {
        std::vector<Vector2i> neighbors;
        const std::vector<Vector2i> dirs = {
            Vector2i(2, 0), Vector2i(-2, 0), Vector2i(0, 2), Vector2i(0, -2)
        };

        for (const auto& dir : dirs) {
            int nx = x + dir.x;
            int ny = y + dir.y;

            if (nx >= 1 && nx < MAZE_WIDTH - 1 &&
                ny >= 1 && ny < MAZE_HEIGHT - 1 &&
                !cells[ny][nx].visited) {
                neighbors.push_back(Vector2i(nx, ny));
            }
        }
        std::shuffle(neighbors.begin(), neighbors.end(), rng);
        return neighbors;
    }
};

Maze* Maze::instance = nullptr;

/* Auto-solve animation timer */
auto lastAutoMoveTime = std::chrono::steady_clock::now();
const float autoMoveInterval = 0.02f; // 20 ms

/* GLUT callback functions */
void display() {
    if (Maze::instance) Maze::instance->draw();
}

void keyboard(unsigned char key, int x, int y) {
    if (!Maze::instance) return;

    switch (key) {
        case ' ': // Space
            Maze::instance->findPathBFS();
            std::cout << "Show shortest path" << std::endl;
            break;
        case 'r':
        case 'R':
            Maze::instance->reset();
            std::cout << "Reset maze" << std::endl;
            break;
        case 'n':
        case 'N':
            Maze::instance->generateNewMaze();
            std::cout << "Generate new maze" << std::endl;
            break;
        case 'a':
        case 'A':
            Maze::instance->prepareAutoMove();
            lastAutoMoveTime = std::chrono::steady_clock::now();
            std::cout << "Start auto-solve" << std::endl;
            break;
        case 27: // ESC
            exit(0);
    }
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    if (!Maze::instance || Maze::instance->isAutoMoving()) return;

    switch (key) {
        case GLUT_KEY_UP:    Maze::instance->movePlayer(0, -1); break;
        case GLUT_KEY_DOWN:  Maze::instance->movePlayer(0,  1); break;
        case GLUT_KEY_LEFT:  Maze::instance->movePlayer(-1, 0); break;
        case GLUT_KEY_RIGHT: Maze::instance->movePlayer( 1, 0); break;
    }
    glutPostRedisplay();
}

void timer(int value) {
    if (Maze::instance && Maze::instance->isAutoMoving()) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<float>(now - lastAutoMoveTime).count();

        if (elapsed >= autoMoveInterval) {
            Maze::instance->autoMoveStep();
            lastAutoMoveTime = now;
            glutPostRedisplay();
        }
    }
    glutTimerFunc(16, timer, 0); // ~60 FPS
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(CELL_SIZE * MAZE_WIDTH, CELL_SIZE * MAZE_HEIGHT);
    glutCreateWindow("Random Maze Generator (Pure OpenGL)");

    glViewport(0, 0, CELL_SIZE * MAZE_WIDTH, CELL_SIZE * MAZE_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, CELL_SIZE * MAZE_WIDTH, CELL_SIZE * MAZE_HEIGHT, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(0.16f, 0.16f, 0.16f, 1.0f);

    Maze maze;
    Maze::instance = &maze;

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(0, timer, 0);

    std::cout << "Maze controls:" << std::endl;
    std::cout << "Arrow keys - move player" << std::endl;
    std::cout << "Space      - show shortest path" << std::endl;
    std::cout << "R          - reset maze" << std::endl;
    std::cout << "N          - generate new maze" << std::endl;
    std::cout << "A          - auto-solve" << std::endl;
    std::cout << "ESC        - quit" << std::endl;

    glutMainLoop();
    return 0;
}

#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <vector>
#include <cmath>
#include <random>
#include <string>
#include <chrono>
#include <map>
#include <algorithm>

const int WIDTH = 1920, HEIGHT = 1080;
const float PLAYER_SPEED = 200.0f; // Pixels per second
const float CIRCLE_SPEED = 300.0f; // Pixels per second
const float TURN_SPEED = 2.0f * M_PI; // Radians per second
const int PLAYER_SIZE = 4; // Player pixel size (quad)
const int TRAIL_SIZE = 2; // Trail pixel size (quad)
const int CIRCLE_RADIUS = 45; // Enemy circle radius
const float COLLECTIBLE_SIZE = CIRCLE_RADIUS * 2; // Green square size
const float BLACK_CIRCLE_SIZE = COLLECTIBLE_SIZE; // Black circle radius (2x green square size)
const float BLACK_SQUARE_SIZE = COLLECTIBLE_SIZE * 5; // 5x larger black square
const int COLLISION_CHECK_SIZE = 5; // Size of square area to check for collisions (pixels)

struct Vec2 {
    float x, y;
    Vec2(float x = 0, float y = 0) : x(x), y(y) {}
    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
    Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
};

struct Player {
    Vec2 pos;
    Vec2 direction;
    SDL_Color color;
    std::vector<Vec2> trail;
    bool alive;
    bool willDie; // Flag for next-frame death
    bool hasMoved; // Flag for invincibility
};

struct Circle {
    Vec2 pos;
    Vec2 vel;
    float radius;
};

struct Collectible {
    Vec2 pos;
    float size; // Green square size
    float blackCircleSize; // Black circle radius
    float blackSquareSize; // Black square size
};

const std::map<char, std::vector<bool>> FONT = {
    {'0', {1,1,1,1,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,1,1,1,1}},
    {'1', {0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0}},
    {'2', {1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,1}},
    {'3', {1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1}},
    {'4', {1,0,0,0,1, 1,0,0,0,1, 1,1,1,1,1, 0,0,0,0,1, 0,0,0,0,1}},
    {'5', {1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1}},
    {'6', {1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1}},
    {'7', {1,1,1,1,1, 0,0,0,0,1, 0,0,0,0,1, 0,0,0,0,1, 0,0,0,0,1}},
    {'8', {1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1}},
    {'9', {1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1}},
    {'-', {0,0,0,0,0, 0,0,0,0,0, 1,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0}},
    {' ', {0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0}}
};

void drawSquare(float x, float y, float size, const SDL_Color& color) {
    glColor3ub(color.r, color.g, color.b);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + size, y);
    glVertex2f(x + size, y + size);
    glVertex2f(x, y + size);
    glEnd();
}

void drawCircle(float x, float y, float radius, const SDL_Color& color) {
    glColor3ub(color.r, color.g, color.b);
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < 360; i++) {
        float rad = i * M_PI / 180.0f;
        glVertex2f(x + cos(rad) * radius, y + sin(rad) * radius);
    }
    glEnd();
}

void drawText(const std::string& text, float x, float y, float squareSize, const SDL_Color& color) {
    glColor3ub(color.r, color.g, color.b);
    float charWidth = squareSize * 6;
    for (size_t i = 0; i < text.size(); ++i) {
        char c = text[i];
        if (FONT.find(c) == FONT.end()) continue;
        const auto& pattern = FONT.at(c);
        float startX = x + i * charWidth;
        for (int row = 0; row < 5; ++row) {
            for (int col = 0; col < 5; ++col) {
                if (pattern[row * 5 + col]) {
                    drawSquare(startX + col * squareSize, y + row * squareSize, squareSize, color);
                }
            }
        }
    }
}

void drawPlayer(const Player& player) {
    drawSquare(player.pos.x - PLAYER_SIZE / 2, player.pos.y - PLAYER_SIZE / 2, PLAYER_SIZE, player.color);
}

void drawTrail(const Player& player, int skipRecent = 0) {
    glColor3ub(player.color.r, player.color.g, player.color.b);
    glBegin(GL_QUADS);
    size_t start = std::max<size_t>(0, player.trail.size() - skipRecent);
    for (size_t i = 0; i < start; ++i) {
        const auto& p = player.trail[i];
        float halfSize = TRAIL_SIZE / 2.0f;
        glVertex2f(p.x - halfSize, p.y - halfSize);
        glVertex2f(p.x + halfSize, p.y - halfSize);
        glVertex2f(p.x + halfSize, p.y + halfSize);
        glVertex2f(p.x - halfSize, p.y + halfSize);
    }
    glEnd();
}

void drawCollectibleBlackSquare(const Collectible& collectible) {
    drawSquare(collectible.pos.x - collectible.blackSquareSize / 2, collectible.pos.y - collectible.blackSquareSize / 2, collectible.blackSquareSize, {0, 0, 0, 255});
}

void drawCollectibleBlackCircle(const Collectible& collectible) {
    drawCircle(collectible.pos.x, collectible.pos.y, collectible.blackCircleSize, {0, 0, 0, 255});
}

void drawCollectibleGreenSquare(const Collectible& collectible) {
    drawSquare(collectible.pos.x - collectible.size / 2, collectible.pos.y - collectible.size / 2, collectible.size, {0, 255, 0, 255});
}

bool checkPixelCollision(const Vec2& pos) {
    GLubyte pixel[3];
    glReadPixels((int)pos.x, HEIGHT - (int)pos.y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
    return !(pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0); // Not black
}

bool checkAreaCollision(const Vec2& center, int size) {
    int halfSize = size / 2;
    for (int dx = -halfSize; dx <= halfSize; dx++) {
        for (int dy = -halfSize; dy <= halfSize; dy++) {
            Vec2 checkPos(center.x + dx, center.y + dy);
            if (checkPos.x < 0 || checkPos.x >= WIDTH || checkPos.y < 0 || checkPos.y >= HEIGHT) continue;
            if (checkPixelCollision(checkPos)) return true;
        }
    }
    return false;
}

bool checkCollectibleCollision(const Vec2& playerPos, const Collectible& collectible) {
    float halfSize = collectible.size / 2;
    return playerPos.x >= collectible.pos.x - halfSize &&
           playerPos.x <= collectible.pos.x + halfSize &&
           playerPos.y >= collectible.pos.y - halfSize &&
           playerPos.y <= collectible.pos.y + halfSize;
}

Collectible spawnCollectible(std::mt19937& rng) {
    // Use black square size for spawn boundaries to ensure it fits
    std::uniform_real_distribution<float> distX(BLACK_SQUARE_SIZE / 2, WIDTH - BLACK_SQUARE_SIZE / 2);
    std::uniform_real_distribution<float> distY(BLACK_SQUARE_SIZE / 2, HEIGHT - BLACK_SQUARE_SIZE / 2);
    return Collectible{Vec2(distX(rng), distY(rng)), COLLECTIBLE_SIZE, BLACK_CIRCLE_SIZE, BLACK_SQUARE_SIZE};
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
    SDL_Window* window = SDL_CreateWindow("2 Player Lines Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1); // Enable VSync
    glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Controller setup
    SDL_GameController* controllers[2] = {nullptr, nullptr};
    int controllerCount = 0;
    for (int i = 0; i < SDL_NumJoysticks() && controllerCount < 2; ++i) {
        if (SDL_IsGameController(i)) {
            controllers[controllerCount] = SDL_GameControllerOpen(i);
            if (controllers[controllerCount]) controllerCount++;
        }
    }

    // Game state
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> distX(50, WIDTH - 50);
    std::uniform_real_distribution<float> distY(50, HEIGHT - 50);

    Player player1, player2;
    player1.pos = Vec2(200, HEIGHT / 2);
    player1.direction = Vec2(1, 0);
    player1.color = {0, 0, 255, 255}; // Blue
    player1.alive = true;
    player1.willDie = false;
    player1.hasMoved = false;

    player2.pos = Vec2(WIDTH - 200, HEIGHT / 2);
    player2.direction = Vec2(-1, 0);
    player2.color = {255, 0, 0, 255}; // Red
    player2.alive = true;
    player2.willDie = false;
    player2.hasMoved = false;

    std::vector<Circle> circles;
    float angle = std::uniform_real_distribution<float>(0, 2 * M_PI)(rng);
    circles.push_back(Circle{Vec2(distX(rng), distY(rng)), Vec2(CIRCLE_SPEED * cos(angle), CIRCLE_SPEED * sin(angle)), CIRCLE_RADIUS});

    Collectible collectible = spawnCollectible(rng);

    int score1 = 0, score2 = 0;
    bool gameOver = false;
    bool gameOverScreen = false; // Flag for score-only screen
    bool firstFrame = true; // Flag to show score on first frame
    auto lastCircleSpawn = std::chrono::steady_clock::now();
    auto gameOverTime = lastCircleSpawn;

    // Game loop
    bool running = true;
    auto lastTime = std::chrono::steady_clock::now();
    while (running) {
        auto currentTime = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Handle input
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                if (event.cbutton.button == SDL_CONTROLLER_BUTTON_X || event.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
                    // Toggle pause (not game over screen)
                }
            }
        }

        if (!gameOverScreen && !gameOver) {
            // Controller input for steering
            for (int i = 0; i < controllerCount; ++i) {
                if (!controllers[i]) continue;
                Player& player = (i == 0) ? player1 : player2;
                if (!player.alive) continue;

                Sint16 leftTrigger = SDL_GameControllerGetAxis(controllers[i], SDL_CONTROLLER_AXIS_TRIGGERLEFT);
                Sint16 rightTrigger = SDL_GameControllerGetAxis(controllers[i], SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
                if (leftTrigger > 0 || rightTrigger > 0) player.hasMoved = true; // Mark as moved on trigger press
                float turn = (rightTrigger - leftTrigger) / 32768.0f * TURN_SPEED * dt;
                float angle = atan2(player.direction.y, player.direction.x) + turn;
                player.direction = Vec2(cos(angle), sin(angle));
            }

            // Update players
            for (auto& player : {&player1, &player2}) {
                if (!player->alive) continue;

                // Check collision
                Vec2 nextPos = player->pos + player->direction * PLAYER_SPEED * dt;
                if (!player->willDie) {
                    // Check wall collision (always applies)
                    if (nextPos.x < 0 || nextPos.x > WIDTH || nextPos.y < 0 || nextPos.y > HEIGHT) {
                        player->willDie = true;
                    }
                    // Check trail/circle collision (only if hasMoved)
                    else if (player->hasMoved) {
                        glClear(GL_COLOR_BUFFER_BIT);
                        drawTrail(player1, player == &player1 ? 5 : 0); // Skip last 5 points for self
                        drawTrail(player2, player == &player2 ? 5 : 0);
                        for (const auto& circle : circles) drawCircle(circle.pos.x, circle.pos.y, circle.radius, {255, 255, 0, 255});
                        if (checkAreaCollision(nextPos, COLLISION_CHECK_SIZE)) {
                            player->willDie = true;
                        }
                    }
                } else {
                    player->alive = false;
                    continue;
                }

                // Move and add trail
                player->pos = nextPos;
                player->trail.push_back(player->pos);

                // Check collectible collision (allowed even if invincible)
                if (checkCollectibleCollision(player->pos, collectible)) {
                    if (player == &player1) score1++;
                    else score2++;
                    collectible = spawnCollectible(rng);
                }
            }

            // Update circles
            for (auto& circle : circles) {
                circle.pos = circle.pos + circle.vel * dt;
                if (circle.pos.x - circle.radius < 0 || circle.pos.x + circle.radius > WIDTH) {
                    circle.vel.x = -circle.vel.x;
                    circle.pos.x = std::max(circle.radius, std::min(WIDTH - circle.radius, circle.pos.x));
                }
                if (circle.pos.y - circle.radius < 0 || circle.pos.y + circle.radius > HEIGHT) {
                    circle.vel.y = -circle.vel.y;
                    circle.pos.y = std::max(circle.radius, std::min(HEIGHT - circle.radius, circle.pos.y));
                }

                // Clear trails
                for (auto& player : {&player1, &player2}) {
                    player->trail.erase(
                        std::remove_if(player->trail.begin(), player->trail.end(),
                            [&](const Vec2& p) {
                                float dx = circle.pos.x - p.x, dy = circle.pos.y - p.y;
                                return sqrt(dx * dx + dy * dy) < circle.radius;
                            }),
                        player->trail.end());
                }
            }

            // Spawn new yellow circle every 5 seconds
            if (std::chrono::duration<float>(currentTime - lastCircleSpawn).count() > 5.0f) {
                float angle = std::uniform_real_distribution<float>(0, 2 * M_PI)(rng);
                circles.push_back(Circle{Vec2(distX(rng), distY(rng)), Vec2(CIRCLE_SPEED * cos(angle), CIRCLE_SPEED * sin(angle)), CIRCLE_RADIUS});
                lastCircleSpawn = currentTime;
            }

            // Check game over
            if (!player1.alive || !player2.alive) {
                gameOver = true;
                gameOverScreen = true;
                gameOverTime = currentTime;
                if (!player1.alive && player2.alive) score2 += 3; // Player1 dies, Player2 gets 3 points
                else if (!player2.alive && player1.alive) score1 += 3; // Player2 dies, Player1 gets 3 points
                // No points if both die
            }
        } else if (gameOverScreen && std::chrono::duration<float>(currentTime - gameOverTime).count() > 5.0f) {
            // Reset game
            player1 = Player{Vec2(200, HEIGHT / 2), Vec2(1, 0), {0, 0, 255, 255}, {}, true, false, false}; // Blue
            player2 = Player{Vec2(WIDTH - 200, HEIGHT / 2), Vec2(-1, 0), {255, 0, 0, 255}, {}, true, false, false}; // Red
            float angle = std::uniform_real_distribution<float>(0, 2 * M_PI)(rng);
            circles = {Circle{Vec2(distX(rng), distY(rng)), Vec2(CIRCLE_SPEED * cos(angle), CIRCLE_SPEED * sin(angle)), CIRCLE_RADIUS}};
            collectible = spawnCollectible(rng);
            gameOver = false;
            gameOverScreen = false;
            lastCircleSpawn = currentTime;
        }

        // Render
        glClear(GL_COLOR_BUFFER_BIT);
        if (gameOverScreen) {
            // Show score and countdown during game over
            std::string scoreText = std::to_string(score1) + "-" + std::to_string(score2);
            float squareSize = 10.0f;
            float textWidth = scoreText.size() * squareSize * 6;
            drawText(scoreText, (WIDTH - textWidth) / 2, HEIGHT / 2 - 25, squareSize, {255, 255, 255, 255});
            int countdown = 5 - static_cast<int>(std::chrono::duration<float>(currentTime - gameOverTime).count());
            if (countdown >= 1) {
                drawText(std::to_string(countdown), (WIDTH - squareSize * 6) / 2, HEIGHT / 2 + 25, squareSize, {255, 255, 255, 255});
            }
        } else {
            // Normal rendering
            drawCollectibleBlackSquare(collectible); // Black square
            drawCollectibleBlackCircle(collectible); // Black circle
            drawCollectibleGreenSquare(collectible); // Green square
            for (const auto& circle : circles) drawCircle(circle.pos.x, circle.pos.y, circle.radius, {255, 255, 0, 255}); // Yellow circles
            drawTrail(player1); // Blue trail
            drawTrail(player2); // Red trail
            drawPlayer(player1); // Blue player
            drawPlayer(player2); // Red player
            if (firstFrame) {
                std::string scoreText = std::to_string(score1) + "-" + std::to_string(score2);
                float squareSize = 10.0f;
                float textWidth = scoreText.size() * squareSize * 6;
                drawText(scoreText, (WIDTH - textWidth) / 2, HEIGHT / 2 - 25, squareSize, {255, 255, 255, 255});
                firstFrame = false;
            }
        }
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    for (auto& controller : controllers) if (controller) SDL_GameControllerClose(controller);
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
// 2 Player Lines Game - Simple cross-platform SDL and OpenGL example
// Copyright (c) 2025 Zachary Geurts
// MIT License

// Platform detection
#if defined(__DJGPP__) || defined(__AMIGA__) || defined(__DREAMCAST__) || defined(__PSP__) || defined(__WII__)
#define USE_SDL1_2 1 // Use SDL1.2 for legacy platforms
#else
#define USE_SDL1_2 0 // Default to SDL2
#endif

#if defined(__ANDROID__) || defined(__IPHONEOS__) || defined(__VITA__) || defined(__3DS__) || defined(__SWITCH__) || defined(__OUYA__) || defined(__PS3__) || defined(__PS4__)
#define USE_OPENGL_ES 1 // OpenGL ES for mobile/consoles
#elif defined(__EMSCRIPTEN__)
#define USE_WEBGL 1 // WebGL (maps to OpenGL ES)
#elif defined(__DJGPP__)
#define USE_MINIGL 1 // MiniGL for DOS
#elif defined(__AMIGA__)
#define USE_WARP3D 1 // Warp3D for Amiga
#elif defined(__DREAMCAST__)
#define USE_GLDC 1 // GLdc for Dreamcast
#else
#define USE_OPENGL 1 // Desktop OpenGL
#endif

// Include SDL
#if USE_SDL1_2
#include <SDL/SDL.h>
#else
#include <SDL2/SDL.h>
#endif

// Include OpenGL
#if USE_OPENGL_ES
#include <GLES/gl.h>
#elif USE_WEBGL
#include <GLES2/gl2.h>
#elif USE_MINIGL
#include <GL/minigl.h>
#elif USE_WARP3D
#include <warp3d/warp3d.h>
#elif USE_GLDC
#include <GL/gl.h>
#include <GL/glkos.h>
#else
#include <GL/gl.h>
#endif

// Standard includes
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>

// C++11 fallbacks for legacy platforms
#if defined(__DJGPP__) || defined(__AMIGA__)
#include <time.h>
#include <stdlib.h>
#define NO_CXX11 1
#else
#include <random>
#include <chrono>
#include <map>
#endif

// Resolution definitions (closest to 320x200 with 4:3 aspect ratio)
#if defined(__3DS__)
const int WIDTH = 400, HEIGHT = 240; // Top screen
#elif defined(__AARCH64__) || defined(__ARMV6__) || defined(__ARMV7__) || defined(__ARMV8__)
const int WIDTH = 640, HEIGHT = 480; // Common ARM
#elif defined(__AMIGA__)
const int WIDTH = 320, HEIGHT = 200; // Exact 320x200
#elif defined(__ANDROID__) || defined(__OUYA__)
const int WIDTH = 800, HEIGHT = 480; // Mobile 4:3-ish
#elif defined(__DJGPP__)
const int WIDTH = 320, HEIGHT = 200; // DOS
#elif defined(__DREAMCAST__)
const int WIDTH = 640, HEIGHT = 480; // Standard 4:3
#elif defined(__EMSCRIPTEN__)
const int WIDTH = 640, HEIGHT = 480; // Web
#elif defined(__IPHONEOS__)
const int WIDTH = 480, HEIGHT = 320; // iOS classic
#elif defined(__PS3__) || defined(__PS4__)
const int WIDTH = 640, HEIGHT = 480; // Console fallback
#elif defined(__PSP__)
const int WIDTH = 480, HEIGHT = 272; // Closest to 4:3
#elif defined(__STEAMDECK__) || defined(__STEAMLINK__)
const int WIDTH = 800, HEIGHT = 600; // Steam 4:3
#elif defined(__SWITCH__)
const int WIDTH = 1280, HEIGHT = 720; // Handheld, scaled
#elif defined(__VITA__)
const int WIDTH = 960, HEIGHT = 544; // Scaled PSP
#elif defined(__WII__) || defined(__WIIU__)
const int WIDTH = 640, HEIGHT = 480; // Standard 4:3
#else // linux, macos, windows
const int WIDTH = 640, HEIGHT = 480; // Desktop default
#endif

// Scaling factors based on 320x200
const float SCALE_X = WIDTH / 320.0f;
const float SCALE_Y = HEIGHT / 200.0f;
const float SCALE = std::min(SCALE_X, SCALE_Y);

// Game constants (scaled)
const float PLAYER_SPEED = 100.0f * SCALE;
const float CIRCLE_SPEED = 150.0f * SCALE;
const float TURN_SPEED = 2.0f * 3.14159265358979323846f;
const float PLAYER_SIZE = 2.0f * SCALE;
const float TRAIL_SIZE = 1.0f; // ~1 pixel
const float CIRCLE_RADIUS = 22.5f * SCALE;
const float COLLECTIBLE_SIZE = CIRCLE_RADIUS * 2.0f;
const float BLACK_CIRCLE_SIZE = COLLECTIBLE_SIZE;
const float BLACK_SQUARE_SIZE = COLLECTIBLE_SIZE * 2.5f;
const float COLLISION_CHECK_SIZE = 2.5f * SCALE;

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
	bool willDie;
	bool hasMoved;
};

struct Circle {
	Vec2 pos;
	Vec2 vel;
	float radius;
};

struct Collectible {
	Vec2 pos;
	float size;
	float blackCircleSize;
	float blackSquareSize;
};

// Font data
#if NO_CXX11
static const char FONT_CHARS[] = "0123456789- ";
static const bool FONT_DATA[][25] = {
	{1,1,1,1,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,1,1,1,1}, // 0
	{0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,0,1,0,0}, // 1
	{1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,1}, // 2
	{1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1}, // 3
	{1,0,0,0,1, 1,0,0,0,1, 1,1,1,1,1, 0,0,0,0,1, 0,0,0,0,1}, // 4
	{1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1}, // 5
	{1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1}, // 6
	{1,1,1,1,1, 0,0,0,0,1, 0,0,0,0,1, 0,0,0,0,1, 0,0,0,0,1}, // 7
	{1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1}, // 8
	{1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1}, // 9
	{0,0,0,0,0, 0,0,0,0,0, 1,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0}, // -
	{0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0}  // space
};
#else
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
#endif

// Random number generation
float random_float(float min, float max) {
#if NO_CXX11
	return min + (max - min) * (rand() / (float)RAND_MAX);
#else
	static std::random_device rd;
	static std::mt19937 rng(rd());
	std::uniform_real_distribution<float> dist(min, max);
	return dist(rng);
#endif
}

// Time functions
#if NO_CXX11
float get_time_seconds() {
	return (float)clock() / CLOCKS_PER_SEC;
}
#else
auto get_time() { return std::chrono::steady_clock::now(); }
float get_dt(std::chrono::steady_clock::time_point& last) {
	auto now = get_time();
	float dt = std::chrono::duration<float>(now - last).count();
	last = now;
	return dt;
}
#endif

void drawSquare(float x, float y, float size, const SDL_Color& color) {
#if USE_OPENGL || USE_MINIGL || USE_GLDC
	glColor3ub(color.r, color.g, color.b);
	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x + size, y);
	glVertex2f(x + size, y + size);
	glVertex2f(x, y + size);
	glEnd();
#elif USE_OPENGL_ES
	GLfloat vertices[] = {x, y, x + size, y, x + size, y + size, x, y + size};
	GLubyte colors[] = {color.r, color.g, color.b, 255, color.r, color.g, color.b, 255,
						color.r, color.g, color.b, 255, color.r, color.g, color.b, 255};
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
#else // USE_WARP3D
	// Placeholder for Warp3D (users can replace)
#endif
}

void drawCircle(float x, float y, float radius, const SDL_Color& color) {
#if USE_OPENGL || USE_MINIGL || USE_GLDC
	glColor3ub(color.r, color.g, color.b);
	glBegin(GL_TRIANGLE_FAN);
	for (int i = 0; i < 360; i++) {
		float rad = i * 3.14159265358979323846f / 180.0f;
		glVertex2f(x + cos(rad) * radius, y + sin(rad) * radius);
	}
	glEnd();
#elif USE_OPENGL_ES
	const int segments = 32;
	GLfloat vertices[segments * 2];
	GLubyte colors[segments * 4];
	for (int i = 0; i < segments; i++) {
		float rad = i * 2 * 3.14159265358979323846f / segments;
		vertices[i * 2] = x + cos(rad) * radius;
		vertices[i * 2 + 1] = y + sin(rad) * radius;
		colors[i * 4] = color.r;
		colors[i * 4 + 1] = color.g;
		colors[i * 4 + 2] = color.b;
		colors[i * 4 + 3] = 255;
	}
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
	glDrawArrays(GL_TRIANGLE_FAN, 0, segments);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
#else // USE_WARP3D
	// Placeholder for Warp3D
#endif
}

void drawText(const std::string& text, float x, float y, float squareSize, const SDL_Color& color) {
	float charWidth = squareSize * 6;
	for (size_t i = 0; i < text.size(); ++i) {
		char c = text[i];
#if NO_CXX11
		int fontIdx = -1;
		for (int j = 0; FONT_CHARS[j]; j++) {
			if (FONT_CHARS[j] == c) { fontIdx = j; break; }
		}
		if (fontIdx == -1) continue;
		const bool* pattern = FONT_DATA[fontIdx];
#else
		if (FONT.find(c) == FONT.end()) continue;
		const auto& pattern = FONT.at(c);
#endif
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
#if USE_OPENGL || USE_MINIGL || USE_GLDC
	glColor3ub(player.color.r, player.color.g, player.color.b);
	glBegin(GL_LINES);
	size_t start = std::max<size_t>(0, player.trail.size() - skipRecent);
	for (size_t i = 1; i < start; ++i) {
		const auto& p1 = player.trail[i - 1];
		const auto& p2 = player.trail[i];
		glVertex2f(p1.x, p1.y);
		glVertex2f(p2.x, p2.y);
	}
	glEnd();
#elif USE_OPENGL_ES
	size_t start = std::max<size_t>(0, player.trail.size() - skipRecent);
	std::vector<GLfloat> vertices((start > 0 ? (start - 1) * 2 : 0) * 2);
	std::vector<GLubyte> colors((start > 0 ? (start - 1) * 2 : 0) * 4);
	for (size_t i = 1; i < start; ++i) {
		const auto& p1 = player.trail[i - 1];
		const auto& p2 = player.trail[i];
		size_t idx = (i - 1) * 4;
		vertices[idx] = p1.x; vertices[idx + 1] = p1.y;
		vertices[idx + 2] = p2.x; vertices[idx + 3] = p2.y;
		for (int j = 0; j < 2; j++) {
			colors[(i - 1) * 8 + j * 4] = player.color.r;
			colors[(i - 1) * 8 + j * 4 + 1] = player.color.g;
			colors[(i - 1) * 8 + j * 4 + 2] = player.color.b;
			colors[(i - 1) * 8 + j * 4 + 3] = 255;
		}
	}
	if (!vertices.empty()) {
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, vertices.data());
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors.data());
		glDrawArrays(GL_LINES, 0, (start - 1) * 2);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
	}
#else // USE_WARP3D
	// Placeholder for Warp3D
#endif
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
#if USE_OPENGL || USE_OPENGL_ES || USE_MINIGL || USE_GLDC
	GLubyte pixel[3];
	glReadPixels((int)pos.x, HEIGHT - (int)pos.y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
	return !(pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0);
#else
	return false; // Placeholder for Warp3D
#endif
}

bool checkAreaCollision(const Vec2& center, float size) {
	float halfSize = size / 2.0f;
	for (float dx = -halfSize; dx <= halfSize; dx += 1.0f) {
		for (float dy = -halfSize; dy <= halfSize; dy += 1.0f) {
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

Collectible spawnCollectible() {
	Vec2 pos(random_float(BLACK_SQUARE_SIZE / 2, WIDTH - BLACK_SQUARE_SIZE / 2),
			 random_float(BLACK_SQUARE_SIZE / 2, HEIGHT - BLACK_SQUARE_SIZE / 2));
	return Collectible{pos, COLLECTIBLE_SIZE, BLACK_CIRCLE_SIZE, BLACK_SQUARE_SIZE};
}

int main(int argc, char* argv[]) {
#if USE_SDL1_2
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
	SDL_SetVideoMode(WIDTH, HEIGHT, 0, SDL_OPENGL);
#else
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
	SDL_Window* window = SDL_CreateWindow("2 Player Lines Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	SDL_GL_SetSwapInterval(1);
#endif

#if USE_GLDC
	glKosInit();
#endif
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glLineWidth(1.0f); // 1-pixel trails

#if USE_SDL1_2
	SDL_Joystick* joysticks[2] = {nullptr, nullptr};
	int joystickCount = 0;
	for (int i = 0; i < SDL_NumJoysticks() && joystickCount < 2; ++i) {
		joysticks[joystickCount] = SDL_JoystickOpen(i);
		if (joysticks[joystickCount]) joystickCount++;
	}
#else
	SDL_GameController* controllers[2] = {nullptr, nullptr};
	int controllerCount = 0;
	for (int i = 0; i < SDL_NumJoysticks() && controllerCount < 2; ++i) {
		if (SDL_IsGameController(i)) {
			controllers[controllerCount] = SDL_GameControllerOpen(i);
			if (controllers[controllerCount]) controllerCount++;
		}
	}
#endif

#if NO_CXX11
	srand(time(nullptr));
#endif

	Player player1{Vec2(100 * SCALE_X, HEIGHT / 2), Vec2(1, 0), {0, 0, 255, 255}, {}, true, false, false};
	Player player2{Vec2(WIDTH - 100 * SCALE_X, HEIGHT / 2), Vec2(-1, 0), {255, 0, 0, 255}, {}, true, false, false};

	std::vector<Circle> circles;
	float angle = random_float(0, 2 * 3.14159265358979323846f);
	circles.push_back(Circle{Vec2(random_float(25 * SCALE_X, WIDTH - 25 * SCALE_X), random_float(25 * SCALE_Y, HEIGHT - 25 * SCALE_Y)),
							 Vec2(CIRCLE_SPEED * cos(angle), CIRCLE_SPEED * sin(angle)), CIRCLE_RADIUS});

	Collectible collectible = spawnCollectible();

	int score1 = 0, score2 = 0;
	bool gameOver = false;
	bool gameOverScreen = false;
	bool firstFrame = true;
#if NO_CXX11
	float lastCircleSpawn = get_time_seconds();
	float gameOverTime = lastCircleSpawn;
	float lastTime = lastCircleSpawn;
#else
	auto lastCircleSpawn = get_time();
	auto gameOverTime = lastCircleSpawn;
	auto lastTime = lastCircleSpawn;
#endif

	bool running = true;
	while (running) {
#if NO_CXX11
		float dt = get_time_seconds() - lastTime;
		lastTime = get_time_seconds();
#else
		float dt = get_dt(lastTime);
#endif

#if USE_SDL1_2
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) running = false;
			if (event.type == SDL_JOYBUTTONDOWN) {
				if (event.jbutton.button == 0 || event.jbutton.button == 1) {
					// Toggle pause
				}
			}
		}
#else
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) running = false;
			if (event.type == SDL_CONTROLLERBUTTONDOWN) {
				if (event.cbutton.button == SDL_CONTROLLER_BUTTON_X || event.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
					// Toggle pause
				}
			}
		}
#endif

		if (!gameOverScreen && !gameOver) {
#if USE_SDL1_2
			for (int i = 0; i < joystickCount; ++i) {
				if (!joysticks[i]) continue;
				Player& player = (i == 0) ? player1 : player2;
				if (!player.alive) continue;
				int left = SDL_JoystickGetAxis(joysticks[i], 2);
				int right = SDL_JoystickGetAxis(joysticks[i], 5);
				if (left > 0 || right > 0) player.hasMoved = true;
				float turn = (right - left) / 32768.0f * TURN_SPEED * dt;
				float angle = atan2(player.direction.y, player.direction.x) + turn;
				player.direction = Vec2(cos(angle), sin(angle));
			}
#else
			for (int i = 0; i < controllerCount; ++i) {
				if (!controllers[i]) continue;
				Player& player = (i == 0) ? player1 : player2;
				if (!player.alive) continue;
				Sint16 leftTrigger = SDL_GameControllerGetAxis(controllers[i], SDL_CONTROLLER_AXIS_TRIGGERLEFT);
				Sint16 rightTrigger = SDL_GameControllerGetAxis(controllers[i], SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
				if (leftTrigger > 0 || rightTrigger > 0) player.hasMoved = true;
				float turn = (rightTrigger - leftTrigger) / 32768.0f * TURN_SPEED * dt;
				float angle = atan2(player.direction.y, player.direction.x) + turn;
				player.direction = Vec2(cos(angle), sin(angle));
			}
#endif

			for (auto& player : {&player1, &player2}) {
				if (!player->alive) continue;

				Vec2 nextPos = player->pos + player->direction * PLAYER_SPEED * dt;
				if (!player->willDie) {
					if (nextPos.x < 0 || nextPos.x > WIDTH || nextPos.y < 0 || nextPos.y > HEIGHT) {
						player->willDie = true;
					} else if (player->hasMoved) {
						glClear(GL_COLOR_BUFFER_BIT);
						drawTrail(player1, player == &player1 ? 5 : 0);
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

				player->pos = nextPos;
				player->trail.push_back(player->pos);

				if (checkCollectibleCollision(player->pos, collectible)) {
					if (player == &player1) score1++;
					else score2++;
					collectible = spawnCollectible();
				}
			}

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

				for (auto& player : {&player1, &player2}) {
					player->trail.erase(
						std::remove_if(player.trail.begin(), player.trail.end(),
							[&](const Vec2& p) {
								float dx = circle.pos.x - p.x, dy = circle.pos.y - p.y;
								return sqrt(dx * dx + dy * dy) < circle.radius;
							}),
						player->trail.end());
				}
			}

#if NO_CXX11
			if (get_time_seconds() - lastCircleSpawn > 5.0f) {
				float angle = random_float(0, 2 * 3.14159265358979323846f);
				circles.push_back(Circle{Vec2(random_float(25 * SCALE_X, WIDTH - 25 * SCALE_X), random_float(25 * SCALE_Y, HEIGHT - 25 * SCALE_Y)),
										 Vec2(CIRCLE_SPEED * cos(angle), CIRCLE_SPEED * sin(angle)), CIRCLE_RADIUS});
				lastCircleSpawn = get_time_seconds();
			}
#else
			if (std::chrono::duration<float>(get_time() - lastCircleSpawn).count() > 5.0f) {
				float angle = random_float(0, 2 * 3.14159265358979323846f);
				circles.push_back(Circle{Vec2(random_float(25 * SCALE_X, WIDTH - 25 * SCALE_X), random_float(25 * SCALE_Y, HEIGHT - 25 * SCALE_Y)),
										 Vec2(CIRCLE_SPEED * cos(angle), CIRCLE_SPEED * sin(angle)), CIRCLE_RADIUS});
				lastCircleSpawn = get_time();
			}
#endif

			if (!player1.alive || !player2.alive) {
				gameOver = true;
				gameOverScreen = true;
#if NO_CXX11
				gameOverTime = get_time_seconds();
#else
				gameOverTime = get_time();
#endif
				if (!player1.alive && player2.alive) score2 += 3;
				else if (!player2.alive && player1.alive) score1 += 3;
			}
		}
#if NO_CXX11
		else if (gameOverScreen && (get_time_seconds() - gameOverTime) > 5.0f) {
#else
		else if (gameOverScreen && std::chrono::duration<float>(get_time() - gameOverTime).count() > 5.0f) {
#endif
			player1 = Player{Vec2(100 * SCALE_X, HEIGHT / 2), Vec2(1, 0), {0, 0, 255, 255}, {}, true, false, false};
			player2 = Player{Vec2(WIDTH - 100 * SCALE_X, HEIGHT / 2), Vec2(-1, 0), {255, 0, 0, 255}, {}, true, false, false};
			circles.clear();
			float angle = random_float(0, 2 * 3.14159265358979323846f);
			circles.push_back(Circle{Vec2(random_float(25 * SCALE_X, WIDTH - 25 * SCALE_X), random_float(25 * SCALE_Y, HEIGHT - 25 * SCALE_Y)),
									 Vec2(CIRCLE_SPEED * cos(angle), CIRCLE_SPEED * sin(angle)), CIRCLE_RADIUS});
			collectible = spawnCollectible();
			gameOver = false;
			gameOverScreen = false;
#if NO_CXX11
			lastCircleSpawn = get_time_seconds();
#else
			lastCircleSpawn = get_time();
#endif
		}

		glClear(GL_COLOR_BUFFER_BIT);
		if (gameOverScreen) {
			std::string scoreText = std::to_string(score1) + "-" + std::to_string(score2);
			float squareSize = 5.0f * SCALE;
			float textWidth = scoreText.size() * squareSize * 6;
			drawText(scoreText, (WIDTH - textWidth) / 2, HEIGHT / 2 - 12.5f * SCALE, squareSize, {255, 255, 255, 255});
#if NO_CXX11
			int countdown = 5 - static_cast<int>(get_time_seconds() - gameOverTime);
#else
			int countdown = 5 - static_cast<int>(std::chrono::duration<float>(get_time() - gameOverTime).count());
#endif
			if (countdown >= 1) {
				drawText(std::to_string(countdown), (WIDTH - squareSize * 6) / 2, HEIGHT / 2 + 12.5f * SCALE, squareSize, {255, 255, 255, 255});
			}
		} else {
			drawCollectibleBlackSquare(collectible);
			drawCollectibleBlackCircle(collectible);
			drawCollectibleGreenSquare(collectible);
			for (const auto& circle : circles) drawCircle(circle.pos.x, circle.pos.y, circle.radius, {255, 255, 0, 255});
			drawTrail(player1);
			drawTrail(player2);
			drawPlayer(player1);
			drawPlayer(player2);
			if (firstFrame) {
				std::string scoreText = std::to_string(score1) + "-" + std::to_string(score2);
				float squareSize = 5.0f * SCALE;
				float textWidth = scoreText.size() * squareSize * 6;
				drawText(scoreText, (WIDTH - textWidth) / 2, HEIGHT / 2 - 12.5f * SCALE, squareSize, {255, 255, 255, 255});
				firstFrame = false;
			}
		}
#if USE_SDL1_2
		SDL_GL_SwapBuffers();
#else
		SDL_GL_SwapWindow(window);
#endif
	}

#if USE_SDL1_2
	for (auto& joystick : joysticks) if (joystick) SDL_JoystickClose(joystick);
#else
	for (auto& controller : controllers) if (controller) SDL_GameControllerClose(controller);
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(window);
#endif
	SDL_Quit();
	return 0;
}
// 2 Player Lines Game - Cross-platform SDL and OpenGL example
// Copyright (c) 2025 Zachary Geurts
// MIT License
// Builds a 2-player game where players move at fixed speed (7s to cross widest resolution),
// collect green squares (10% shortest side) for 1 point, and avoid yellow circles (10% diameter),
// opposing trails, or edges (death awards 3 points to opponent). Controls: triggers, D-pad,
// joystick, or keyboard (P1).

// Platform detection
#if defined(__DJGPP__) || defined(__AMIGA__) || defined(__DREAMCAST__) || defined(__PSP__) || defined(__WII__)
#define USE_SDL1_2 1 // SDL1.2 for legacy platforms
#else
#define USE_SDL1_2 0 // SDL2 default
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
#if USE_OPENGL_ES || USE_WEBGL
#include <GLES/gl.h>
#elif USE_MINIGL
#include <GL/minigl.h>
#elif USE_WARP3D
// Warp3D stub
#define W3D_Vertex float
struct W3D_Vertex { float x, y, z, w; float u, v; unsigned char r, g, b, a; };
extern void W3D_DrawArrays(int mode, W3D_Vertex* vertices, int count);
#define GL_QUADS 0x0007
#define GL_LINES 0x0003
#define GL_TRIANGLE_FAN 0x0006
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
#endif

// Resolution definitions
#if defined(__3DS__)
const int WIDTH = 400, HEIGHT = 240; // 3DS top screen, 5:3
#elif defined(__AARCH64__) || defined(__ARMV6__) || defined(__ARMV7__) || defined(__ARMV8__)
const int WIDTH = 1280, HEIGHT = 720; // ARM mobile/console 16:9
#elif defined(__AMIGA__)
const int WIDTH = 320, HEIGHT = 200; // Amiga classic 4:3
#elif defined(__ANDROID__) || defined(__OUYA__)
const int WIDTH = 1280, HEIGHT = 720; // Mobile 16:9
#elif defined(__DJGPP__)
const int WIDTH = 320, HEIGHT = 200; // DOS 4:3
#elif defined(__DREAMCAST__)
const int WIDTH = 640, HEIGHT = 480; // Dreamcast 4:3
#elif defined(__EMSCRIPTEN__)
const int WIDTH = 1280, HEIGHT = 720; // Web 16:9
#elif defined(__IPHONEOS__)
const int WIDTH = 1136, HEIGHT = 640; // iOS 16:9
#elif defined(__PS3__)
const int WIDTH = 1280, HEIGHT = 720; // PS3 16:9
#elif defined(__PS4__)
const int WIDTH = 1920, HEIGHT = 1080; // PS4 16:9
#elif defined(__PSP__)
const int WIDTH = 480, HEIGHT = 272; // PSP 16:9
#elif defined(__STEAMDECK__) || defined(__STEAMLINK__)
const int WIDTH = 1280, HEIGHT = 800; // Steam Deck 16:10
#elif defined(__SWITCH__)
const int WIDTH = 1280, HEIGHT = 720; // Switch 16:9
#elif defined(__VITA__)
const int WIDTH = 960, HEIGHT = 544; // Vita 16:9
#elif defined(__WII__) || defined(__WIIU__)
const int WIDTH = 854, HEIGHT = 480; // Wii/Wii U 16:9
#else // Windows, Linux, MacOS
const int WIDTH = 1920, HEIGHT = 1080; // Desktop 16:9
#endif

// Game constants
const float SHORTEST_SIDE = std::min(WIDTH, HEIGHT);
const float PLAYER_SPEED = WIDTH / 7.0f; // 7s to cross WIDTH
const float CIRCLE_SPEED = PLAYER_SPEED * 1.5f;
const float TURN_SPEED = 2.0f * 3.14159265358979323846f;
const float PLAYER_SIZE = 2.0f; // ~2 pixels
const float TRAIL_SIZE = 1.0f; // 1 pixel
const float CIRCLE_RADIUS = 0.05f * SHORTEST_SIDE; // 5% for 10% diameter
const float COLLECTIBLE_SIZE = 0.1f * SHORTEST_SIDE; // 10% width
const float BLACK_CIRCLE_SIZE = COLLECTIBLE_SIZE * 2.0f;
const float BLACK_SQUARE_SIZE = COLLECTIBLE_SIZE * 2.5f;

// Lookup table for AmigaOS m68k
#if defined(__AMIGA__)
const int ANGLE_STEPS = 256;
float COS_TABLE[ANGLE_STEPS], SIN_TABLE[ANGLE_STEPS];
void initTrig() {
	for (int i = 0; i < ANGLE_STEPS; i++) {
		float rad = i * 2 * 3.14159265358979323846f / ANGLE_STEPS;
		COS_TABLE[i] = cos(rad);
		SIN_TABLE[i] = sin(rad);
	}
}
float fastCos(float rad) { return COS_TABLE[(int)(rad * ANGLE_STEPS / (2 * 3.14159265358979323846f)) % ANGLE_STEPS]; }
float fastSin(float rad) { return SIN_TABLE[(int)(rad * ANGLE_STEPS / (2 * 3.14159265358979323846f)) % ANGLE_STEPS]; }
#else
#define fastCos cos
#define fastSin sin
#endif

struct Vec2 {
	float x, y;
	Vec2(float x = 0, float y = 0) : x(x), y(y) {}
	Vec2 operator+(const Vec2& o) const { return Vec2(x + o.x, y + o.y); }
	Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
};

struct Player {
	Vec2 pos, dir;
	SDL_Color color;
	Vec2 trail[1000]; // Fixed-size for AmigaOS
	int trailCount;
	bool alive, willDie, hasMoved;
};

struct Circle {
	Vec2 pos, vel;
	float radius;
};

struct Collectible {
	Vec2 pos;
	float size, blackCircleSize, blackSquareSize;
};

// Font data
const char FONT_CHARS[] = "0123456789- ";
const bool FONT_DATA[][25] = {
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
float get_time() { return (float)clock() / CLOCKS_PER_SEC; }
float get_dt(float& last) { float now = get_time(); float dt = now - last; last = now; return dt; }
#else
auto get_time() { return std::chrono::steady_clock::now(); }
float get_dt(std::chrono::steady_clock::time_point& last) {
	auto now = get_time();
	float dt = std::chrono::duration<float>(now - last).count();
	last = now;
	return dt;
}
#endif

// Drawing functions
void drawSquare(float x, float y, float size, const SDL_Color& c) {
#if USE_WARP3D
	W3D_Vertex v[4] = {
		{x, y, 0, 1, 0, 0, c.r, c.g, c.b, c.a},
		{x + size, y, 0, 1, 0, 0, c.r, c.g, c.b, c.a},
		{x + size, y + size, 0, 1, 0, 0, c.r, c.g, c.b, c.a},
		{x, y + size, 0, 1, 0, 0, c.r, c.g, c.b, c.a}
	};
	W3D_DrawArrays(GL_QUADS, v, 4);
#elif USE_OPENGL || USE_MINIGL || USE_GLDC
	glColor3ub(c.r, c.g, c.b);
	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x + size, y);
	glVertex2f(x + size, y + size);
	glVertex2f(x, y + size);
	glEnd();
#else // USE_OPENGL_ES || USE_WEBGL
	GLfloat v[] = {x, y, x + size, y, x + size, y + size, x, y + size};
	GLubyte col[] = {c.r, c.g, c.b, 255, c.r, c.g, c.b, 255, c.r, c.g, c.b, 255, c.r, c.g, c.b, 255};
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, v);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, col);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
#endif
}

void drawCircle(float x, float y, float r, const SDL_Color& c) {
#if USE_WARP3D
	const int seg = 16;
	W3D_Vertex v[seg];
	for (int i = 0; i < seg; i++) {
		float rad = i * 2 * 3.14159265358979323846f / seg;
		v[i] = {x + cos(rad) * r, y + sin(rad) * r, 0, 1, 0, 0, c.r, c.g, c.b, c.a};
	}
	W3D_DrawArrays(GL_TRIANGLE_FAN, v, seg);
#elif USE_OPENGL || USE_MINIGL || USE_GLDC
	glColor3ub(c.r, c.g, c.b);
	glBegin(GL_TRIANGLE_FAN);
	for (int i = 0; i < 32; i++) {
		float rad = i * 2 * 3.14159265358979323846f / 32;
		glVertex2f(x + fastCos(rad) * r, y + fastSin(rad) * r);
	}
	glEnd();
#else // USE_OPENGL_ES || USE_WEBGL
	const int seg = 32;
	GLfloat v[seg * 2];
	GLubyte col[seg * 4];
	for (int i = 0; i < seg; i++) {
		float rad = i * 2 * 3.14159265358979323846f / seg;
		v[i * 2] = x + fastCos(rad) * r;
		v[i * 2 + 1] = y + fastSin(rad) * r;
		col[i * 4] = c.r; col[i * 4 + 1] = c.g; col[i * 4 + 2] = c.b; col[i * 4 + 3] = 255;
	}
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, v);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, col);
	glDrawArrays(GL_TRIANGLE_FAN, 0, seg);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
#endif
}

void drawTrail(const Player& p, int skipRecent = 0) {
#if USE_WARP3D
	W3D_Vertex v[p.trailCount * 2];
	for (int i = 1; i < p.trailCount - skipRecent; i++) {
		v[(i - 1) * 2] = {p.trail[i - 1].x, p.trail[i - 1].y, 0, 1, 0, 0, p.color.r, p.color.g, p.color.b, p.color.a};
		v[(i - 1) * 2 + 1] = {p.trail[i].x, p.trail[i].y, 0, 1, 0, 0, p.color.r, p.color.g, p.color.b, p.color.a};
	}
	W3D_DrawArrays(GL_LINES, v, (p.trailCount - skipRecent - 1) * 2);
#elif USE_OPENGL || USE_MINIGL || USE_GLDC
	glColor3ub(p.color.r, p.color.g, p.color.b);
	glBegin(GL_LINES);
	for (int i = 1; i < p.trailCount - skipRecent; i++) {
		glVertex2f(p.trail[i - 1].x, p.trail[i - 1].y);
		glVertex2f(p.trail[i].x, p.trail[i].y);
	}
	glEnd();
#else // USE_OPENGL_ES || USE_WEBGL
	int count = std::max(0, p.trailCount - skipRecent - 1);
	GLfloat v[count * 4];
	GLubyte col[count * 8];
	for (int i = 1; i < p.trailCount - skipRecent; i++) {
		int idx = (i - 1) * 4;
		v[idx] = p.trail[i - 1].x; v[idx + 1] = p.trail[i - 1].y;
		v[idx + 2] = p.trail[i].x; v[idx + 3] = p.trail[i].y;
		for (int j = 0; j < 2; j++) {
			col[(i - 1) * 8 + j * 4] = p.color.r;
			col[(i - 1) * 8 + j * 4 + 1] = p.color.g;
			col[(i - 1) * 8 + j * 4 + 2] = p.color.b;
			col[(i - 1) * 8 + j * 4 + 3] = 255;
		}
	}
	if (count > 0) {
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, v);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, col);
		glDrawArrays(GL_LINES, 0, count * 2);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
	}
#endif
}

void drawText(const std::string& text, float x, float y, float size, const SDL_Color& c) {
	float charWidth = size * 6;
	for (size_t i = 0; i < text.size(); i++) {
		int idx = -1;
		for (int j = 0; FONT_CHARS[j]; j++) if (FONT_CHARS[j] == text[i]) { idx = j; break; }
		if (idx == -1) continue;
		float startX = x + i * charWidth;
		for (int row = 0; row < 5; row++) {
			for (int col = 0; col < 5; col++) {
				if (FONT_DATA[idx][row * 5 + col]) {
					drawSquare(startX + col * size, y + row * size, size, c);
				}
			}
		}
	}
}

// Game logic
Collectible spawnCollectible() {
	Vec2 pos(random_float(BLACK_SQUARE_SIZE / 2, WIDTH - BLACK_SQUARE_SIZE / 2),
			 random_float(BLACK_SQUARE_SIZE / 2, HEIGHT - BLACK_SQUARE_SIZE / 2));
	return {pos, COLLECTIBLE_SIZE, BLACK_CIRCLE_SIZE, BLACK_SQUARE_SIZE};
}

bool checkCollision(const Vec2& p, const Player& other, const std::vector<Circle>& circles) {
	if (p.x < 0 || p.x > WIDTH || p.y < 0 || p.y > HEIGHT) return true;
	for (int i = 1; i < other.trailCount; i++) {
		Vec2 a = other.trail[i - 1], b = other.trail[i];
		Vec2 ab = b + (a * -1), ap = p + (a * -1);
		float t = (ap.x * ab.x + ap.y * ab.y) / (ab.x * ab.x + ab.y * ab.y);
		if (t < 0 || t > 1) continue;
		Vec2 proj = a + ab * t;
		float dx = p.x - proj.x, dy = p.y - proj.y;
		if (dx * dx + dy * dy < PLAYER_SIZE * PLAYER_SIZE) return true;
	}
	for (const auto& c : circles) {
		float dx = p.x - c.pos.x, dy = p.y - c.pos.y;
		if (dx * dx + dy * dy < (c.radius + PLAYER_SIZE) * (c.radius + PLAYER_SIZE)) return true;
	}
	return false;
}

bool checkCollectibleCollision(const Vec2& p, const Collectible& c) {
	float half = c.size / 2;
	return p.x >= c.pos.x - half && p.x <= c.pos.x + half && p.y >= c.pos.y - half && p.y <= c.pos.y + half;
}

int main(int argc, char* argv[]) {
#if USE_SDL1_2
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
	SDL_SetVideoMode(WIDTH, HEIGHT, 0, SDL_OPENGL | SDL_FULLSCREEN);
#else
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
	SDL_Window* window = SDL_CreateWindow("Lines", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	SDL_GL_SetSwapInterval(1);
#endif

#if USE_GLDC
	glKosInit();
#endif
#if defined(__AMIGA__)
	initTrig();
#endif
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glClearColor(0, 0, 0, 1);
	glLineWidth(TRAIL_SIZE);

#if USE_SDL1_2
	SDL_Joystick* joysticks[2] = {nullptr, nullptr};
	int joyCount = 0;
	for (int i = 0; i < SDL_NumJoysticks() && joyCount < 2; i++) {
		joysticks[joyCount] = SDL_JoystickOpen(i);
		if (joysticks[joyCount]) joyCount++;
	}
#else
	SDL_GameController* controllers[2] = {nullptr, nullptr};
	int ctrlCount = 0;
	for (int i = 0; i < SDL_NumJoysticks() && ctrlCount < 2; i++) {
		if (SDL_IsGameController(i)) {
			controllers[ctrlCount] = SDL_GameControllerOpen(i);
			if (controllers[ctrlCount]) ctrlCount++;
		}
	}
#endif

#if NO_CXX11
	srand(time(nullptr));
#endif

	Player p1{{100, HEIGHT / 2}, {1, 0}, {0, 0, 255, 255}, {}, 0, true, false, false};
	Player p2{{WIDTH - 100, HEIGHT / 2}, {-1, 0}, {255, 0, 0, 255}, {}, 0, true, false, false};
	std::vector<Circle> circles;
	float angle = random_float(0, 2 * 3.14159265358979323846f);
	circles.push_back({{random_float(CIRCLE_RADIUS, WIDTH - CIRCLE_RADIUS), random_float(CIRCLE_RADIUS, HEIGHT - CIRCLE_RADIUS)},
					   {CIRCLE_SPEED * fastCos(angle), CIRCLE_SPEED * fastSin(angle)}, CIRCLE_RADIUS});
	Collectible collectible = spawnCollectible();
	int score1 = 0, score2 = 0;
	bool gameOver = false, gameOverScreen = false, firstFrame = true;
#if NO_CXX11
	float lastCircle = get_time(), gameOverTime = lastCircle, lastTime = lastCircle;
#else
	auto lastCircle = get_time(), gameOverTime = lastCircle, lastTime = lastCircle;
#endif

	bool running = true;
	while (running) {
		float dt = get_dt(lastTime);

		const Uint8* keys = SDL_GetKeyboardState(nullptr);
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) running = false;
		}

		if (!gameOverScreen && !gameOver) {
			// Controls
			float turn1 = 0, turn2 = 0;
			if (keys[SDL_SCANCODE_LEFT]) { turn1 = -TURN_SPEED * dt; p1.hasMoved = true; }
			if (keys[SDL_SCANCODE_RIGHT]) { turn1 = TURN_SPEED * dt; p1.hasMoved = true; }
#if USE_SDL1_2
			for (int i = 0; i < joyCount; i++) {
				if (!joysticks[i]) continue;
				Player& p = i == 0 ? p1 : p2;
				if (!p.alive) continue;
				Sint16 axis = SDL_JoystickGetAxis(joysticks[i], 0);
				Sint16 trigL = SDL_JoystickGetAxis(joysticks[i], 2), trigR = SDL_JoystickGetAxis(joysticks[i], 5);
				if (axis < -10000 || axis > 10000 || trigL > 0 || trigR > 0) p.hasMoved = true;
				float turn = ((trigR - trigL) / 32768.0f + axis / 32768.0f) * TURN_SPEED * dt;
				(i == 0 ? turn1 : turn2) += turn;
			}
#else
			for (int i = 0; i < ctrlCount; i++) {
				if (!controllers[i]) continue;
				Player& p = i == 0 ? p1 : p2;
				if (!p.alive) continue;
				Sint16 axis = SDL_GameControllerGetAxis(controllers[i], SDL_CONTROLLER_AXIS_LEFTX);
				Sint16 trigL = SDL_GameControllerGetAxis(controllers[i], SDL_CONTROLLER_AXIS_TRIGGERLEFT);
				Sint16 trigR = SDL_GameControllerGetAxis(controllers[i], SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
				Uint8 dpadL = SDL_GameControllerGetButton(controllers[i], SDL_CONTROLLER_BUTTON_DPAD_LEFT);
				Uint8 dpadR = SDL_GameControllerGetButton(controllers[i], SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
				if (axis < -10000 || axis > 10000 || trigL > 0 || trigR > 0 || dpadL || dpadR) p.hasMoved = true;
				float turn = ((trigR - trigL) / 32768.0f + (dpadR - dpadL) + axis / 32768.0f) * TURN_SPEED * dt;
				(i == 0 ? turn1 : turn2) += turn;
			}
#endif
			for (auto p : {&p1, &p2}) {
				if (!p->alive) continue;
				float angle = atan2(p->dir.y, p->dir.x) + (p == &p1 ? turn1 : turn2);
				p->dir = {fastCos(angle), fastSin(angle)};
				Vec2 next = p->pos + p->dir * PLAYER_SPEED * dt;
				if (!p->willDie && p->hasMoved) {
					if (checkCollision(next, p == &p1 ? p2 : p1, circles)) p->willDie = true;
				}
				if (p->willDie) { p->alive = false; continue; }
				p->pos = next;
				if (p->trailCount < 1000) p->trail[p->trailCount++] = p->pos;
				if (checkCollectibleCollision(p->pos, collectible)) {
					(p == &p1 ? score1 : score2)++;
					collectible = spawnCollectible();
				}
			}

			// Update circles
			for (auto& c : circles) {
				c.pos = c.pos + c.vel * dt;
				if (c.pos.x - c.radius < 0 || c.pos.x + c.radius > WIDTH) {
					c.vel.x = -c.vel.x;
					c.pos.x = std::max(c.radius, std::min(WIDTH - c.radius, c.pos.x));
				}
				if (c.pos.y - c.radius < 0 || c.pos.y + c.radius > HEIGHT) {
					c.vel.y = -c.vel.y;
					c.pos.y = std::max(c.radius, std::min(HEIGHT - c.radius, c.pos.y));
				}
				for (auto p : {&p1, &p2}) {
					int i = 0;
					while (i < p->trailCount) {
						float dx = c.pos.x - p->trail[i].x, dy = c.pos.y - p->trail[i].y;
						if (dx * dx + dy * dy < c.radius * c.radius) {
							p->trail[i] = p->trail[--p->trailCount];
						} else i++;
					}
				}
			}

			if (get_dt(lastCircle) > 5.0f) {
				float angle = random_float(0, 2 * 3.14159265358979323846f);
				circles.push_back({{random_float(CIRCLE_RADIUS, WIDTH - CIRCLE_RADIUS), random_float(CIRCLE_RADIUS, HEIGHT - CIRCLE_RADIUS)},
								   {CIRCLE_SPEED * fastCos(angle), CIRCLE_SPEED * fastSin(angle)}, CIRCLE_RADIUS});
#if NO_CXX11
				lastCircle = get_time();
#else
				lastCircle = get_time();
#endif
			}

			if (!p1.alive || !p2.alive) {
				gameOver = true;
				gameOverScreen = true;
				if (!p1.alive && p2.alive) score2 += 3;
				else if (!p2.alive && p1.alive) score1 += 3;
#if NO_CXX11
				gameOverTime = get_time();
#else
				gameOverTime = get_time();
#endif
			}
		} else if (gameOverScreen && get_dt(gameOverTime) > 5.0f) {
			p1 = {{100, HEIGHT / 2}, {1, 0}, {0, 0, 255, 255}, {}, 0, true, false, false};
			p2 = {{WIDTH - 100, HEIGHT / 2}, {-1, 0}, {255, 0, 0, 255}, {}, 0, true, false, false};
			circles.clear();
			float angle = random_float(0, 2 * 3.14159265358979323846f);
			circles.push_back({{random_float(CIRCLE_RADIUS, WIDTH - CIRCLE_RADIUS), random_float(CIRCLE_RADIUS, HEIGHT - CIRCLE_RADIUS)},
							   {CIRCLE_SPEED * fastCos(angle), CIRCLE_SPEED * fastSin(angle)}, CIRCLE_RADIUS});
			collectible = spawnCollectible();
			gameOver = false;
			gameOverScreen = false;
#if NO_CXX11
			lastCircle = get_time();
#else
			lastCircle = get_time();
#endif
		}

		// Render
		glClear(GL_COLOR_BUFFER_BIT);
		if (gameOverScreen) {
			std::string score = std::to_string(score1) + "-" + std::to_string(score2);
			float size = 5.0f;
			drawText(score, (WIDTH - score.size() * size * 6) / 2, HEIGHT / 2 - 12.5f, size, {255, 255, 255, 255});
			int countdown = 5 - (int)get_dt(gameOverTime);
			if (countdown >= 1) drawText(std::to_string(countdown), (WIDTH - size * 6) / 2, HEIGHT / 2 + 12.5f, size, {255, 255, 255, 255});
		} else {
			drawSquare(collectible.pos.x - collectible.blackSquareSize / 2, collectible.pos.y - collectible.blackSquareSize / 2, collectible.blackSquareSize, {0, 0, 0, 255});
			drawCircle(collectible.pos.x, collectible.pos.y, collectible.blackCircleSize, {0, 0, 0, 255});
			drawSquare(collectible.pos.x - collectible.size / 2, collectible.pos.y - collectible.size / 2, collectible.size, {0, 255, 0, 255});
			for (const auto& c : circles) drawCircle(c.pos.x, c.pos.y, c.radius, {255, 255, 0, 255});
			drawTrail(p1); drawTrail(p2);
			drawSquare(p1.pos.x - PLAYER_SIZE / 2, p1.pos.y - PLAYER_SIZE / 2, PLAYER_SIZE, p1.color);
			drawSquare(p2.pos.x - PLAYER_SIZE / 2, p2.pos.y - PLAYER_SIZE / 2, PLAYER_SIZE, p2.color);
			if (firstFrame) {
				std::string score = std::to_string(score1) + "-" + std::to_string(score2);
				float size = 5.0f;
				drawText(score, (WIDTH - score.size() * size * 6) / 2, HEIGHT / 2 - 12.5f, size, {255, 255, 255, 255});
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
	for (auto j : joysticks) if (j) SDL_JoystickClose(j);
#else
	for (auto c : controllers) if (c) SDL_GameControllerClose(c);
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(window);
#endif
	SDL_Quit();
	return 0;
}

// Warp3D stubs
#if USE_WARP3D
void W3D_DrawArrays(int mode, W3D_Vertex* v, int count) {
	// Placeholder: Link against Warp3D for actual rendering
}
#endif
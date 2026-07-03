#include <fmt/base.h>
#include <raylib.h>

constexpr int windowWidth = 1280;
constexpr int windowHeight = 800;

struct Ball {
	int x, y;
	int speed_x, speed_y;
	float radius;

	void Draw() {
		DrawCircle(x, y, radius, WHITE);
	}

	void Update() {
		y += speed_y;
		x += speed_x;

		if (y + static_cast<int>(radius) >= GetScreenHeight() || y - static_cast<int>(radius) <= 0) {
			speed_y *= -1;
		}
		if (x + static_cast<int>(radius) >= GetScreenWidth() || x - static_cast<int>(radius) <= 0) {
			speed_x *= -1;
		}
	}
};

Ball ball{};

auto main() -> int {
	fmt::println("Hello, clangd + CMake!");
	InitWindow(windowWidth, windowHeight, "Pong Game!");
	SetTargetFPS(60);

	ball.radius = 20;
	ball.x = windowWidth / 2;
	ball.y = windowHeight / 2;
	ball.speed_x = 7;
	ball.speed_y = 7;

	while (WindowShouldClose() == false) {
		BeginDrawing();
		ClearBackground(BLACK);

		DrawLine(windowWidth / 2, 0, windowWidth / 2, windowHeight, WHITE);
		// DrawCircle(windowWidth / 2, windowHeight / 2, 20, WHITE);
		ball.Update();
		ball.Draw();
		DrawRectangle(10, windowHeight / 2 - 60, 25, 120, WHITE);
		DrawRectangle(windowWidth - 35, windowHeight / 2 - 60, 25, 120, WHITE);
		EndDrawing();
	}

	CloseWindow();
	return 0;
}

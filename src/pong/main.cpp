#include <fmt/base.h>
#include <raylib.h>

constexpr int windowWidth = 1280;
constexpr int windowHeight = 800;

auto main() -> int {
	fmt::println("Hello, clangd + CMake!");
	InitWindow(windowWidth, windowHeight, "Pong Game!");
	SetTargetFPS(60);

	while (WindowShouldClose() == false) {
		BeginDrawing();
		ClearBackground(BLACK);

		DrawCircle(windowWidth / 2, windowHeight / 2, 20, WHITE);
		DrawRectangle(0, 0, 25, 120, WHITE);
		EndDrawing();
	}

	CloseWindow();
	return 0;
}

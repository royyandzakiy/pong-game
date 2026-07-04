#include <fmt/base.h>
#include <raylib.h>
#include <type_traits>

constexpr int windowWidth = 1280;
constexpr int windowHeight = 800;

struct PlayerPaddle {};
struct PcPaddle {};

template <typename T> struct Paddle {
	Paddle(int _x, int _y, int _w, int _h, int _speed) : x(_x), y(_y), w(_w), h(_h), speed(_speed) {
	}

	void Draw() {
		DrawRectangle(x, y, w, h, WHITE);
	}

	void Update() {
		if constexpr (std::is_same_v<T, PlayerPaddle>) {
			// respond to keypresses
			if (IsKeyDown(KEY_UP))
				y -= speed;
			if (IsKeyDown(KEY_DOWN))
				y += speed;
		} else if constexpr (std::is_same_v<T, PcPaddle>) {
			if (y + h / 2 > ball_y)
				y -= speed;
			if (y + h / 2 <= ball_y)
				y += speed;
		}

		limitMovement();
	}

	void Notify(int x, int y) {
		ball_x = x;
		ball_y = y;
	}

  private:
	int x, y, w, h, speed;
	int ball_x{}, ball_y{};

	void limitMovement() {
		if (y <= 0)
			y = 0;
		if (y + h >= GetScreenHeight())
			y = GetScreenHeight() - h;
	}
};

template <typename TCallback> struct Ball {
	Ball(int _x, int _y, float _radius, int _speed_x, int _speed_y, TCallback _callback)
		: radius(_radius), x(_x), y(_y), speed_x(_speed_x), speed_y(_speed_y), OnBallMoved(std::move(_callback)) {
	}

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

		OnBallMoved(x, y);
	}

  private:
	float radius;
	int x, y;
	int speed_x, speed_y;
	TCallback OnBallMoved;
};

auto main() -> int {
	fmt::println("Hello, clangd + CMake!");
	InitWindow(windowWidth, windowHeight, "Pong Game!");
	SetTargetFPS(60);

	Paddle<PlayerPaddle> playerPaddle{windowWidth - 35, windowHeight / 2 - 60, 25, 120, 7};
	Paddle<PcPaddle> pcPaddle{10, windowHeight / 2 - 60, 25, 120, 7};
	Ball ball{windowWidth / 2, windowHeight / 2, 20, 7, 7, [&pcPaddle](int x, int y) {
		pcPaddle.Notify(x, y); }};

	while (WindowShouldClose() == false) {
		BeginDrawing();
		ClearBackground(BLACK);

		DrawLine(windowWidth / 2, 0, windowWidth / 2, windowHeight, WHITE);
		ball.Update();
		playerPaddle.Update();
		pcPaddle.Update();

		ball.Draw();
		playerPaddle.Draw();
		pcPaddle.Draw();
		EndDrawing();
	}

	CloseWindow();
	return 0;
}

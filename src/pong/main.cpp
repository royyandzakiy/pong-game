#include <fmt/base.h>
#include <raylib.h>
#include <type_traits>

constexpr float windowWidth = 1280;
constexpr float windowHeight = 800;

template <typename TCallback> struct Ball {
	Ball(float _x, float _y, float _radius, float _speed_x, float _speed_y, TCallback _callback)
		: radius(_radius), x(_x), y(_y), speed_x(_speed_x), speed_y(_speed_y), OnBallMovedCb(std::move(_callback)) {
	}

	void Draw() {
		DrawCircle(x, y, radius, WHITE);
	}

	void Update() {
		y += speed_y;
		x += speed_x;

		if (y + radius >= static_cast<float>(GetScreenHeight()) || y - radius <= 0) {
			speed_y *= -1;
		}
		if (x + radius >= static_cast<float>(GetScreenWidth()) || x - radius <= 0) {
			speed_x *= -1;
		}

		OnBallMovedCb(x, y, radius);
	}

	float radius;
	float x, y;
	float speed_x, speed_y;

  private:
	TCallback OnBallMovedCb;
};

struct PlayerPaddle {};
struct PcPaddle {};

template <typename T> struct Paddle {
	Paddle(float _x, float _y, float _w, float _h, float _speed) : x(_x), y(_y), w(_w), h(_h), speed(_speed) {
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
			// follows center of the ball with the given speed
			if (y + h / 2 > ball_y)
				y -= speed;
			if (y + h / 2 <= ball_y)
				y += speed;
		}

		limitMovement();
	}

	void Notify(float x, float y, float r) {
		ball_x = x;
		ball_y = y;
		ball_r = r;
	}

	bool IsHit() {
		return CheckCollisionCircleRec(Vector2{.x = ball_x, .y = ball_y}, ball_r,
									   Rectangle{.x = x, .y = y, .width = w, .height = h});
	}

	float x, y, w, h, speed;

  private:
	float ball_x{}, ball_y{}, ball_r{};

	void limitMovement() {
		if (y <= 0)
			y = 0;
		if (y + h >= static_cast<float>(GetScreenHeight()))
			y = static_cast<float>(GetScreenHeight()) - h;
	}
};

auto main() -> int {
	fmt::println("Hello, clangd + CMake!");
	InitWindow(windowWidth, windowHeight, "Pong Game!");
	SetTargetFPS(60);

	Paddle<PlayerPaddle> playerPaddle{windowWidth - 35, windowHeight / 2 - 60, 25, 120, 7};
	Paddle<PcPaddle> pcPaddle{10, windowHeight / 2 - 60, 25, 120, 7};
	Ball ball{windowWidth / 2, windowHeight / 2, 20, 7, 7, [&pcPaddle, &playerPaddle](float x, float y, float r) {
				  pcPaddle.Notify(x, y, r);
				  playerPaddle.Notify(x, y, r);
			  }};

	while (WindowShouldClose() == false) {
		BeginDrawing();
		ClearBackground(BLACK);

		DrawLine(windowWidth / 2, 0, windowWidth / 2, windowHeight, WHITE);
		ball.Update();
		playerPaddle.Update();
		pcPaddle.Update();

		if (playerPaddle.IsHit() || pcPaddle.IsHit()) {
			ball.speed_x *= -1;
		}

		ball.Draw();
		playerPaddle.Draw();
		pcPaddle.Draw();
		EndDrawing();
	}

	CloseWindow();
	return 0;
}

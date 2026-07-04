#include <array>
#include <fmt/base.h>
#include <raylib.h>
#include <type_traits>

constexpr float windowWidth = 1280;
constexpr float windowHeight = 800;
int cpuScore = 0;
int playerScore = 0;

template <typename T> struct Paddle;

template <typename TCallback> struct Ball {
	Ball(float _x, float _y, float _radius, float _speed_x, float _speed_y, TCallback _callback)
		: radius(_radius), x(_x), y(_y), speed_x(_speed_x), speed_y(_speed_y), OnBallMovedCb(std::move(_callback)) {
	}

	void Draw() {
		DrawCircle(static_cast<int>(x), static_cast<int>(y), radius, WHITE);
	}

	void Update() {
		y += speed_y;
		x += speed_x;

		// wall collision
		if (y + radius >= static_cast<float>(GetScreenHeight()) || y - radius <= 0) {
			speed_y *= -1;
		}
		if (x + radius >= static_cast<float>(GetScreenWidth())) {
			cpuScore++;
			ResetBall();
		}

		if (x - radius <= 0) {
			playerScore++;
			ResetBall();
		}

		OnBallMovedCb(x, y, radius);
	}

	template <typename TPaddle> void CheckCollide(Paddle<TPaddle> *paddle) {
		bool isCollide =
			CheckCollisionCircleRec(Vector2{.x = x, .y = y}, radius,
									Rectangle{.x = paddle->x, .y = paddle->y, .width = paddle->w, .height = paddle->h});
		if (isCollide) {
			speed_x *= -1;
		}
	}

	void ResetBall() {
		x = static_cast<float>(GetScreenWidth()) / 2;
		y = static_cast<float>(GetScreenHeight()) / 2;

		std::array<int, 2> speed_choices{-1, 1};
		speed_x = static_cast<float>(speed_choices.at(GetRandomValue(0, 1)));
		speed_y = static_cast<float>(speed_choices.at(GetRandomValue(0, 1)));
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

		// call this window collision
		limitMovement();
	}

	void Notify(float x, float y, float r) {
		ball_x = x;
		ball_y = y;
		ball_r = r;
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
	Ball ball{windowWidth / 2,
			  windowHeight / 2,
			  20,
			  7,
			  7,
			  [&pcPaddle](float x, float y, float r) {
				  pcPaddle.Notify(x, y, r); }};

	while (WindowShouldClose() == false) {
		BeginDrawing();
		ClearBackground(BLACK);

		DrawLine(windowWidth / 2, 0, windowWidth / 2, windowHeight, WHITE);
		ball.Update();
		playerPaddle.Update();
		pcPaddle.Update();
		ball.CheckCollide(&pcPaddle);
		ball.CheckCollide(&playerPaddle);

		DrawCircle(windowWidth / 2, windowHeight / 2, 150.0f, GREEN);
		ball.Draw();
		playerPaddle.Draw();
		pcPaddle.Draw();
		DrawText(TextFormat("%i", cpuScore), windowWidth / 4 - 20, 20, 80, WHITE);
		DrawText(TextFormat("%i", playerScore), (windowWidth / 4) * 3 - 20, 20, 80, WHITE);
		EndDrawing();
	}

	CloseWindow();
	return 0;
}

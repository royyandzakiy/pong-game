#include <array>
#include <fmt/base.h>
#include <raylib.h>
#include <type_traits>

constexpr float windowWidth = 1280;
constexpr float windowHeight = 800;
int g_cpuScore = 0;
int g_playerScore = 0;

template <typename T> struct Paddle;
template <typename TCallback> struct Ball {
	Ball(float _posX, float _posY, float _radius, float _speedX, float _speedY, TCallback _callback)
		: m_radius(_radius), m_posX(_posX), m_posY(_posY), m_speedX(_speedX), m_speedY(_speedY),
		  m_onBallMovedCb(std::move(_callback)) {
	}

	void Draw() {
		DrawCircleV(Vector2{.x = m_posX, .y = m_posY}, m_radius, WHITE);
	}

	void Update() {
		m_posY += m_speedY;
		m_posX += m_speedX;

		// wall collision
		if (m_posY + m_radius >= static_cast<float>(GetScreenHeight()) || m_posY - m_radius <= 0) {
			m_speedY *= -1;
		}
		if (m_posX + m_radius >= static_cast<float>(GetScreenWidth())) {
			g_cpuScore++;
			ResetBall();
		}
		if (m_posX - m_radius <= 0) {
			g_playerScore++;
			ResetBall();
		}

		m_onBallMovedCb(m_posX, m_posY, m_radius);
	}

	template <typename TPaddle> void CheckCollide(Paddle<TPaddle> *paddle) {
		bool isCollide = CheckCollisionCircleRec(Vector2{.x = m_posX, .y = m_posY}, m_radius,
												 Rectangle{.x = paddle->GetPosX(),
														   .y = paddle->GetPosY(),
														   .width = paddle->GetWidth(),
														   .height = paddle->GetHeight()});
		if (isCollide) {
			m_speedX *= -1;
		}
	}

	void ResetBall() {
		m_posX = static_cast<float>(GetScreenWidth()) / 2;
		m_posY = static_cast<float>(GetScreenHeight()) / 2;

		std::array<int, 2> directions{-1, 1};
		m_speedX = static_cast<float>(directions.at(GetRandomValue(0, 1)));
		m_speedY = static_cast<float>(directions.at(GetRandomValue(0, 1)));
	}

  private:
	float m_radius;
	float m_posX, m_posY;
	float m_speedX, m_speedY;
	TCallback m_onBallMovedCb;
};

struct PlayerPaddle {};
struct PcPaddle {};

template <typename T> struct Paddle {
	Paddle(float _posX, float _posY, float _width, float _height, float _speed)
		: m_posX(_posX), m_posY(_posY), m_width(_width), m_height(_height), m_speed(_speed) {
	}

	void Draw() {
		DrawRectangleRec(Rectangle{.x = m_posX, .y = m_posY, .width = m_width, .height = m_height}, WHITE);
	}

	void Update() {
		if constexpr (std::is_same_v<T, PlayerPaddle>) {
			// respond to keypresses
			if (IsKeyDown(KEY_UP))
				m_posY -= m_speed;
			if (IsKeyDown(KEY_DOWN))
				m_posY += m_speed;
		} else if constexpr (std::is_same_v<T, PcPaddle>) {
			// follows center of the ball with the given speed
			if (m_posY + m_height / 2 > m_ballPosY)
				m_posY -= m_speed;
			if (m_posY + m_height / 2 <= m_ballPosY)
				m_posY += m_speed;
		}

		WindowCollision();
	}

	void Notify(float posX, float posY, float radius) {
		m_ballPosX = posX;
		m_ballPosY = posY;
		m_ballRadius = radius;
	}

	// Getters
	[[nodiscard]] float GetPosX() const {
		return m_posX;
	}
	[[nodiscard]] float GetPosY() const {
		return m_posY;
	}
	[[nodiscard]] float GetWidth() const {
		return m_width;
	}
	[[nodiscard]] float GetHeight() const {
		return m_height;
	}

  private:
	float m_posX, m_posY;
	float m_width, m_height;
	float m_speed;
	float m_ballPosX{}, m_ballPosY{}, m_ballRadius{};

	void WindowCollision() {
		if (m_posY <= 0)
			m_posY = 0;
		if (m_posY + m_height >= static_cast<float>(GetScreenHeight()))
			m_posY = static_cast<float>(GetScreenHeight()) - m_height;
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
		DrawText(TextFormat("%i", g_cpuScore), windowWidth / 4 - 20, 20, 80, WHITE);
		DrawText(TextFormat("%i", g_playerScore), (windowWidth / 4) * 3 - 20, 20, 80, WHITE);
		EndDrawing();
	}

	CloseWindow();
	return 0;
}

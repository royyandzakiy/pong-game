#include <array>
#include <concepts>
#include <fmt/base.h>
#include <functional>
#include <raylib.h>
#include <string_view>
#include <type_traits>
#include <utility>

namespace GameConfig {
constexpr std::string_view gameTitle{"Pong Game"};
constexpr float windowWidth{1280};
constexpr float windowHeight{800};
}; // namespace GameConfig

int g_cpuScore = 0;
int g_playerScore = 0;

constexpr Color BallColor{230, 247, 0, 255};
constexpr Color PaddleColor{66, 10, 252, 255};
constexpr Color BgColor{211, 218, 229, 255};	 // d3dae5
constexpr Color BgLeftColor{225, 230, 239, 255}; // e1e6ef
constexpr Color BgCircleColor{255, 255, 255, 255};
constexpr Color ScoreColor{255, 255, 255, 255};

template <typename T> struct Paddle;

struct Ball {
	using BallMoveCb = std::function<void(float, float, float)>;

	Ball(float posX, float posY, float radius, float speedX, float speedY)
		: m_radius(radius), m_posX(posX), m_posY(posY), m_speedX(speedX), m_speedY(speedY) {
	}

	~Ball() = default;

	Ball(Ball &&other) noexcept
		: m_radius(other.m_radius), m_posX(other.m_posX), m_posY(other.m_posY), m_speedX(other.m_speedX),
		  m_speedY(other.m_speedY), m_onBallMovedCb(std::move(other.m_onBallMovedCb)) {
		other.m_posX = 0;
		other.m_posY = 0;
		other.m_radius = 0;
		other.m_speedX = 0;
		other.m_speedY = 0;
		other.m_onBallMovedCb = nullptr;
	}

	Ball &operator=(Ball &&other) noexcept = delete;
	Ball(const Ball &other) = delete;
	Ball &operator=(const Ball &other) = delete;

	void SetCallback(BallMoveCb onBallMoveCb) {
		m_onBallMovedCb = std::move(onBallMoveCb);
	}

	void Draw() {
		DrawCircleV(Vector2{.x = m_posX, .y = m_posY}, m_radius, BallColor);
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

		if (m_onBallMovedCb)
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
		m_speedX = 7.0f * static_cast<float>(directions.at(GetRandomValue(0, 1)));
		m_speedY = 7.0f * static_cast<float>(directions.at(GetRandomValue(0, 1)));
	}

  private:
	float m_radius;
	float m_posX, m_posY;
	float m_speedX, m_speedY;
	BallMoveCb m_onBallMovedCb;
};

struct PlayerPaddle {};
struct PcPaddle {};

template <typename T> struct Paddle {
	explicit Paddle(float posX, float posY, float width, float height, float speed)
		: m_posX(posX), m_posY(posY), m_width(width), m_height(height), m_speed(speed) {
	}

	void Draw() {
		DrawRectangleRec(Rectangle{.x = m_posX, .y = m_posY, .width = m_width, .height = m_height}, PaddleColor);
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

template <typename T>
concept PaddleLike = requires(T t) {
	{ t.Draw() } -> std::same_as<void>;
};

// todo: move all logic inside this game class
template <typename T, typename U> class Game {
  public:
	Game(T &&playerPaddle, U &&pcPaddle, Ball ball)
		: m_playerPaddle(std::forward<T>(playerPaddle)), m_pcPaddle(std::forward<U>(pcPaddle)),
		  m_ball(std::move(ball)) {
		m_ball.SetCallback([this](float posX, float posY, float radius) { m_pcPaddle.Notify(posX, posY, radius); });
		InitWindow(GameConfig::windowWidth, GameConfig::windowHeight, GameConfig::gameTitle.data());
		SetTargetFPS(60);
	}

	~Game() {
		CloseWindow();
	}

	Game(Game &&other) noexcept = delete;
	Game &operator=(Game &&other) noexcept = delete;
	Game(const Game &&other) noexcept = delete;
	Game &operator=(const Game &&other) noexcept = delete;

	void Run() {
		while (WindowShouldClose() == false) {
			BeginDrawing();
			ClearBackground(BgColor);

			Update_prv();
			Draw_prv();

			EndDrawing();
		}
	}

  private:
	T m_playerPaddle;
	U m_pcPaddle;
	Ball m_ball;

	void Update_prv() {
		m_ball.Update();
		m_playerPaddle.Update();
		m_pcPaddle.Update();
		m_ball.CheckCollide(&m_pcPaddle);
		m_ball.CheckCollide(&m_playerPaddle);
	}

	void Draw_prv() {
		DrawRectangle(0, 0, GameConfig::windowWidth / 2, GameConfig::windowHeight, BgLeftColor);
		DrawCircle(GameConfig::windowWidth / 2, GameConfig::windowHeight / 2, 150.0f, BgCircleColor);
		DrawText(TextFormat("%i", g_cpuScore), GameConfig::windowWidth / 4 - 20, 20, 80, ScoreColor);
		DrawText(TextFormat("%i", g_playerScore), (GameConfig::windowWidth / 4) * 3 - 20, 20, 80, ScoreColor);

		m_ball.Draw();
		m_playerPaddle.Draw();
		m_pcPaddle.Draw();
	}
};

auto main() -> int {
	Paddle<PlayerPaddle> playerPaddle{GameConfig::windowWidth - 35, GameConfig::windowHeight / 2 - 60, 25, 120, 7};
	Paddle<PcPaddle> pcPaddle{10, GameConfig::windowHeight / 2 - 60, 25, 120, 7};
	Ball ball{GameConfig::windowWidth / 2, GameConfig::windowHeight / 2, 20, 7, 7};
	Game game{std::move(playerPaddle), std::move(pcPaddle), std::move(ball)};

	game.Run();
	return 0;
}

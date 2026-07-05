#include <array>
#include <concepts>
#include <fmt/base.h>
#include <functional>
#include <raylib.h>
#include <string>
#include <type_traits>
#include <utility>

namespace GameConfig {
constexpr const char *gameTitle{"Pong Game"};
constexpr float windowWidth{1280};
constexpr float windowHeight{800};
}; // namespace GameConfig

namespace GameColors {
constexpr Color BallColor{.r = 230, .g = 247, .b = 0, .a = 255};
constexpr Color PaddleColor{.r = 66, .g = 10, .b = 252, .a = 255};
constexpr Color BgColor{.r = 211, .g = 218, .b = 229, .a = 255};	 // d3dae5
constexpr Color BgLeftColor{.r = 225, .g = 230, .b = 239, .a = 255}; // e1e6ef
constexpr Color BgCircleColor{.r = 255, .g = 255, .b = 255, .a = 255};
constexpr Color ScoreColor{.r = 255, .g = 255, .b = 255, .a = 255};
} // namespace GameColors

template <typename T> class Paddle;
template <typename T, typename U> class Game;

class Ball {
  public:
	using BallMoveCb = std::function<void(float, float, float)>;

	explicit Ball(float posX, float posY, float radius, float speedX, float speedY)
		: m_radius(radius), m_posX(posX), m_posY(posY), m_speedX(speedX), m_speedY(speedY) {
	}

	Ball() = delete;   // remove default ctor
	~Ball() = default; // use default dtor

	// move ctor, non-const rval (std::move)/asgn only needs to be defined IF it uses raw pointer. for primitives
	// default is enough
	Ball(const Ball &other) = delete;				  // copy ctor, const lval
	Ball &operator=(const Ball &other) = delete;	  // copy assg, const lval
	Ball(Ball &&other) noexcept = default;			  // move ctor, non-const rval (std::move)
	Ball &operator=(Ball &&other) noexcept = default; // move assg, non-const rval (std::move)

	void SetCallback(BallMoveCb onBallMoveCb) {
		m_onBallMovedCb = std::move(onBallMoveCb);
	}

	void Draw() {
		DrawCircleV(Vector2{.x = m_posX, .y = m_posY}, m_radius, GameColors::BallColor);
	}

	void Update() {
		m_posY += m_speedY;
		m_posX += m_speedX;

		// wall collision
		bool isCollideTopBotWall = m_posY + m_radius >= static_cast<float>(GetScreenHeight()) || m_posY - m_radius <= 0;
		if (isCollideTopBotWall) {
			m_speedY *= -1;
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

template <typename T> class Paddle {
  public:
	explicit Paddle(float posX, float posY, float width, float height, float speed)
		: m_posX(posX), m_posY(posY), m_width(width), m_height(height), m_speed(speed) {
	}

	Paddle() = delete;	 // remove default ctor
	~Paddle() = default; // use default dtor

	// move ctor, non-const rval (std::move)/asgn only needs to be defined IF it uses raw pointer. for primitives
	// default is enough
	Paddle(const Paddle &) noexcept = delete;			 // copy ctor, const lval
	Paddle &operator=(const Paddle &) noexcept = delete; // copy assg, const lval
	Paddle(Paddle &&) noexcept = default;				 // move ctor, non-const rval (std::move)
	Paddle &operator=(Paddle &&) noexcept = default;	 // move assg, non-const rval (std::move)

	void Draw() {
		DrawRectangleRec(Rectangle{.x = m_posX, .y = m_posY, .width = m_width, .height = m_height},
						 GameColors::PaddleColor);
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
			bool isBallGoUp = m_posY + m_height / 2 > m_ballPosY;
			bool isBallGoDown = m_posY + m_height / 2 <= m_ballPosY;
			if (isBallGoUp)
				m_posY -= m_speed;
			if (isBallGoDown)
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
		bool isCollideTop = m_posY <= 0;
		bool isCollideBottom = m_posY + m_height >= static_cast<float>(GetScreenHeight());

		if (isCollideTop)
			m_posY = 0;
		if (isCollideBottom)
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
		: m_playerPaddle(std::move(playerPaddle)), m_pcPaddle(std::move(pcPaddle)), m_ball(std::move(ball)) {
		m_ball.SetCallback([this](float posX, float posY, float radius) {
			m_pcPaddle.Notify(posX, posY, radius);

			bool isBallHitRightScreen = posX + radius >= static_cast<float>(GetScreenWidth());
			bool isBallHitLeftScreen = posX - radius <= 0;

			if (isBallHitRightScreen) {
				m_cpuScore++;
				m_ball.ResetBall();
			}
			if (isBallHitLeftScreen) {
				m_playerScore++;
				m_ball.ResetBall();
			}
		});
		InitWindow(GameConfig::windowWidth, GameConfig::windowHeight, GameConfig::gameTitle);
		SetTargetFPS(60);
	}

	Game() = delete; // remove default ctor
	~Game() {
		CloseWindow();
	}

	// copy & move operator
	Game(const Game &) noexcept = delete;			 // copy ctor, const lval
	Game &operator=(const Game &) noexcept = delete; // copy assg, const lval
	Game(Game &&) noexcept = delete;				 // move ctor, non-const rval (std::move)
	Game &operator=(Game &&) noexcept = delete;		 // move assg, non-const rval (std::move)

	void Run() {
		while (WindowShouldClose() == false) {
			BeginDrawing();
			ClearBackground(GameColors::BgColor);

			Update_prv();
			Draw_prv();

			EndDrawing();
		}
	}

  private:
	T m_playerPaddle;
	U m_pcPaddle;
	Ball m_ball;
	int m_cpuScore{0};
	int m_playerScore{0};

	void Update_prv() {
		m_ball.Update();
		m_playerPaddle.Update();
		m_pcPaddle.Update();
		m_ball.CheckCollide(&m_pcPaddle);
		m_ball.CheckCollide(&m_playerPaddle);
	}

	void Draw_prv() {
		DrawRectangle(0, 0, GameConfig::windowWidth / 2, static_cast<int>(GameConfig::windowHeight),
					  GameColors::BgLeftColor);
		DrawCircle(static_cast<int>(GameConfig::windowWidth) / 2, static_cast<int>(GameConfig::windowHeight) / 2,
				   150.0f, GameColors::BgCircleColor);
		DrawText(std::to_string(m_cpuScore).c_str(), static_cast<int>(GameConfig::windowWidth) / 4 - 20, 20, 80,
				 GameColors::ScoreColor);
		DrawText(std::to_string(m_playerScore).c_str(), static_cast<int>(GameConfig::windowWidth) / 4 * 3 - 20, 20, 80,
				 GameColors::ScoreColor);

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

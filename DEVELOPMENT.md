- g_score move to game
- re add ball padddle game move assignment (rule of5)
- solve drawtext error
- use paddle like properly with static assert
  - enable swap player to left/right of screen

Paddle(Paddle &&) noexcept = default;
Paddle &operator=(Paddle &&) noexcept = default;
Paddle(const Paddle &) noexcept = delete;
Paddle &operator=(const Paddle &) noexcept = delete;

~Ball() = default;
Ball(Ball &&) noexcept = default;
Ball &operator=(Ball &&) noexcept = default;
Ball(const Ball &) = delete;
Ball &operator=(const Ball &) = delete;

## dump
- restructure files
- animate if ball scores
- animate if paddles hit
- possible multiple balls
- easily chge pc paddle speed & ball size for difficulty

## ??
- add imgui (or any kind) menu - settings menu
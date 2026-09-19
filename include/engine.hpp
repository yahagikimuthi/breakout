#pragma once

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <ranges>

#include "setting.hpp"
#include "text.hpp"
#include "type.hpp"
#include "util.hpp"

namespace game {

class Paddle final {
  public:
    explicit Paddle() noexcept {
        paddle_.setOrigin(static_cast<sf::Vector2f>(setting::paddle::size) / 2.f);
        paddle_.setPosition(setting::paddle::position);
        paddle_.setFillColor(sf::Color::White);
    }
    [[nodiscard]] auto position() const noexcept -> sf::Vector2f { return paddle_.getPosition(); }
    [[nodiscard]] auto globalBounds() const noexcept -> sf::FloatRect {
        return paddle_.getGlobalBounds();
    }
    [[nodiscard]] auto size() const noexcept -> sf::Vector2f { return paddle_.getSize(); }
    [[nodiscard]] auto top() const noexcept -> sf::Vector2f {
        const auto y = position().y - (size().y / 2.f);
        return {position().x, y};
    }
    void followMouse(const sf::RenderWindow& window) noexcept {
        const auto mouse = sf::Vector2f{sf::Mouse::getPosition(window)};
        paddle_.setPosition(mouse.x, position().y);
    }
    [[nodiscard]] auto velocity(f32 deltaTime) const noexcept -> sf::Vector2f {
        return (position() - pastPosition_) / deltaTime;
    }

    void draw(sf::RenderWindow& window) noexcept {
        window.draw(paddle_);
        pastPosition_ = position();
    }

    void reset() noexcept {
        paddle_.setPosition(setting::paddle::position);
        pastPosition_ = setting::paddle::position;
    }

  private:
    sf::RectangleShape paddle_{setting::paddle::size};
    sf::Vector2f       pastPosition_{setting::paddle::position};
};

class Ball final {
  public:
    explicit Ball() noexcept {
        ball_.setOrigin(setting::ball::radius, setting::ball::radius);
        ball_.setFillColor(sf::Color::White);
        ball_.setPosition(
            setting::paddle::position.x,
            setting::paddle::position.y - (setting::paddle::size.y / 2) - setting::ball::radius
        );
    }
    [[nodiscard]] auto globalBounds() const noexcept -> sf::FloatRect {
        return ball_.getGlobalBounds();
    }
    [[nodiscard]] auto position() const noexcept -> sf::Vector2f { return ball_.getPosition(); }

    void handleEvent(const sf::Event& event) noexcept {
        if (event.type == sf::Event::MouseButtonPressed and
            event.mouseButton.button == sf::Mouse::Left)
            launch();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) launch();
    }
    void draw(sf::RenderWindow& window) noexcept { window.draw(ball_); }

    void move(f32 deltaTime, const Paddle& paddle) {
        if (not isLaunch_) {
            const auto x = paddle.position().x;
            const auto y = paddle.position().y - (paddle.size().y / 2) - radius();
            ball_.setPosition(x, y);
            return;
        }
        ball_.move(velocity_ * deltaTime);
        const auto pos = position();
        const auto r   = radius();
        if (isFlourCollision())
            isLaunch_ = false;
        else if (isLeftCollision()) {
            velocity_.x = -velocity_.x;
            position(r, pos.y);
        } else if (isRightCollision()) {
            velocity_.x = -velocity_.x;
            position(setting::window::size.x - r, pos.y);
        } else if (isCeilingCollision()) {
            velocity_.y = -velocity_.y;
            position(pos.x, r);
        }
    }

    void collisionPaddle(const f32 deltaTime, const Paddle& paddle) noexcept {
        if (not isLaunch_) return;

        auto _ = ScopeExit{[&]() noexcept -> void {
            const auto paddleTop = paddle.top().y;
            ball_.setPosition(position().x, paddleTop - radius());

            const auto currentSpeed = std::hypot(velocity_.x, velocity_.y);
            if (currentSpeed > setting::ball::maxSpeed)
                velocity_ = (velocity_ / currentSpeed) * setting::ball::maxSpeed;
            if (velocity_.y < 0)
                velocity_.y = std::min(-setting::ball::minYSpeed, velocity_.y);
            else if (velocity_.y > 0)
                velocity_.y = std::max(setting::ball::minYSpeed, velocity_.y);
        }};
        if (velocity_.x != 0) {
            const auto vx =
                velocity_.x + (paddle.velocity(deltaTime).x * setting::ball::paddleInfluence);
            const auto vy = -velocity_.y;
            velocity_     = {vx, vy};
            return;
        }
        const auto speed      = setting::ball::defaultSpeed;
        const auto hitOffset  = position().x - paddle.position().x;
        const auto normalized = hitOffset / (paddle.size().x / 2.f);
        const auto vx         = normalized * speed * 0.75f;
        const auto vy         = -std::sqrt((speed * speed) - (vx * vx));
        velocity_             = {vx, vy};
    }
    void collisionLeftRight() noexcept { velocity_.x = -velocity_.x; }
    void collisionFlourCeiling() noexcept { velocity_.y = -velocity_.y; }

    void reset() noexcept {
        ball_.setPosition(
            setting::paddle::position.x,
            setting::paddle::position.y - (setting::paddle::size.y / 2) - setting::ball::radius
        );
        velocity_ = {0.f, 0.f};
        isLaunch_ = false;
    }

  private:
    [[nodiscard]] auto radius() const noexcept -> f32 { return ball_.getRadius(); }
    [[nodiscard]] auto isLeftCollision() const noexcept -> bool {
        const auto pos = position();
        const auto r   = ball_.getRadius();
        return pos.x - r < 0.f;
    }
    [[nodiscard]] auto isRightCollision() const noexcept -> bool {
        const auto pos = position();
        const auto r   = ball_.getRadius();
        return pos.x + r > setting::window::size.x;
    }
    [[nodiscard]] auto isCeilingCollision() const noexcept -> bool {
        const auto pos = position();
        const auto r   = ball_.getRadius();
        return pos.y - r < 0;
    }
    [[nodiscard]] auto isFlourCollision() const noexcept -> bool {
        const auto pos = position();
        const auto r   = ball_.getRadius();
        return pos.y - r > setting::window::size.y;
    }

    void position(f32 x, f32 y) noexcept { ball_.setPosition(x, y); }

    void launch() noexcept {
        if (isLaunch_) return;
        const auto mouse = sf::Vector2f{sf::Mouse::getPosition()};
        if (mouse.y >= position().y) return;
        if (mouse == position()) return;
        velocity_ = {0.f, -setting::ball::defaultSpeed};
        isLaunch_ = true;
    }

    sf::CircleShape ball_{setting::ball::radius};
    sf::Vector2f    velocity_{0.f, 0.f};
    bool            isLaunch_{false};
};

struct Block final {
    const sf::Vector2f position;
    bool               isActive{true};
};

class Blocks final {
  public:
    explicit Blocks() noexcept {
        namespace set = setting::block;

        blocks.reserve(static_cast<i32>(set::rows * set::cols));
        for (const auto r : std::views::indices(set::rows)) {
            for (const auto c : std::views::indices(set::cols)) {
                const auto x = set::offsetX + (static_cast<f32>(c) * (set::width + set::padding));
                const auto y = set::offsetY + (static_cast<f32>(r) * (set::height + set::padding));
                blocks.emplace_back(sf::Vector2f{x, y});
            }
        }
    }

    [[nodiscard]] auto hasBlock() const noexcept -> bool {
        return std::ranges::any_of(blocks, &Block::isActive);
    }

    void draw(sf::RenderWindow& window) noexcept {
        namespace set  = setting::block;
        auto drawBlock = sf::RectangleShape{{set::width, set::height}};
        drawBlock.setOrigin(set::width / 2, set::height / 2);
        drawBlock.setFillColor(sf::Color::White);

        for (auto& block : blocks) {
            if (not block.isActive) continue;
            drawBlock.setPosition(block.position);
            window.draw(drawBlock);
        }
    }

    void reset() noexcept {
        for (auto& block : blocks) {
            block.isActive = true;
        }
    }

    std::vector<Block> blocks;
};

class Engine final {
  public:
    explicit Engine() noexcept { window_.setFramerateLimit(60); }

    void run() noexcept {
        while (window_.isOpen()) {
            deltaTime_ = clock_.restart().asSeconds();
            if (state_ == GameState::Playing) {
                handleEventPlaying();
                paddle_.followMouse(window_);
                ball_.move(deltaTime_, paddle_);
                if (ball_.position().y > static_cast<f32>(window_.getSize().y)) {
                    state_ = GameState::GameOver;
                }

                collisionBallBlock();
                if (isCollisionBallAndPaddle()) {
                    ball_.collisionPaddle(deltaTime_, paddle_);
                }
                if (not blocks_.hasBlock()) {
                    state_ = GameState::GameClear;
                }
                textManager_.listenState(state_);
                textManager_.setScore(score_);
            }
            handleEventNoPlaying();
            update();
        }
    }

  private:
    [[nodiscard]] auto isCollisionBallAndPaddle() const noexcept -> bool {
        return ball_.globalBounds().intersects(paddle_.globalBounds());
    }

    void collisionBallBlock() noexcept {
        static auto blockShape = []() noexcept -> sf::RectangleShape {
            auto out = sf::RectangleShape{{setting::block::width, setting::block::height}};
            out.setOrigin({setting::block::width / 2, setting::block::height / 2});
            return out;
        }();

        for (auto& block : blocks_.blocks) {
            if (not block.isActive) continue;
            blockShape.setPosition(block.position);
            if (not ball_.globalBounds().intersects(blockShape.getGlobalBounds())) continue;
            block.isActive = false;
            score_ += 10;

            const auto diff = ball_.position() - block.position;

            if (std::abs(diff.x) / setting::block::width >
                std::abs(diff.y) / setting::block::height) {
                ball_.collisionLeftRight();
            } else {
                ball_.collisionFlourCeiling();
            }
            break;
        }
    }

    void handleEventPlaying() noexcept {
        auto event = sf::Event{};
        while (window_.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window_.close();
            }
            ball_.handleEvent(event);
        }
    }

    void handleEventNoPlaying() noexcept {
        auto event = sf::Event{};
        while (window_.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window_.close();
            }
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
            paddle_.reset();
            ball_.reset();
            blocks_.reset();
            state_ = GameState::Playing;
            textManager_.reset();
            score_ = 0;
        }
    }

    void update() noexcept {
        window_.clear();
        paddle_.draw(window_);
        ball_.draw(window_);
        blocks_.draw(window_);
        textManager_.draw(window_);
        window_.display();
    }

    sf::RenderWindow window_{
        sf::VideoMode(setting::window::size.x, setting::window::size.y), "Window"
    };
    sf::Clock                 clock_;
    Paddle                    paddle_;
    Ball                      ball_;
    Blocks                    blocks_;
    f32                       deltaTime_{0.f};
    GameState                 state_{GameState::Playing};
    std::unique_ptr<sf::Font> font_ = std::make_unique<sf::Font>();
    TextManager               textManager_{*font_};
    i32                       score_{};
};
}  // namespace game
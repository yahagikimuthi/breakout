#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>
#include <memory>
#include <string>

#include "type.hpp"

namespace game {

enum class GameState : u8 { Playing, GameOver, GameClear };

class TextManager final {
  public:
    explicit TextManager(sf::Font& font) noexcept {
        font.loadFromFile("../include/DejaVuSans.ttf");
        score_->setFont(font);
        score_->setCharacterSize(24);
        score_->setFillColor(sf::Color::White);
        score_->setPosition(sf::Vector2f{10.f, 10.f});

        result_->setFont(font);
        result_->setCharacterSize(40);
    }
    TextManager(const TextManager&)                             = delete;
    auto operator=(const TextManager&) noexcept -> TextManager& = delete;
    TextManager(TextManager&&)                                  = delete;
    auto operator=(TextManager&&) noexcept -> TextManager&      = delete;
    ~TextManager() noexcept                                     = default;

    void setScore(const i32 score) noexcept {
        score_->setString("Score: " + std::to_string(score));
    }

    void listenState(const GameState state) noexcept {
        if (state == GameState::Playing) return;
        hasResult = true;

        if (state == GameState::GameClear) {
            result_->setString("GAME CLEAR!");
            result_->setFillColor(sf::Color::Green);
            return;
        }
        result_->setString("GAME OVER");
        result_->setFillColor(sf::Color::Red);
    }

    void draw(sf::RenderWindow& window) noexcept {
        window.draw(*score_);
        if (not hasResult) return;
        const auto bounds = result_->getLocalBounds();
        result_->setOrigin(
            sf::Vector2f{bounds.left + (bounds.width / 2.f), bounds.top + (bounds.height / 2.f)}
        );
        result_->setPosition(static_cast<sf::Vector2f>(window.getSize()) / 2.f);
        window.draw(*result_);
    }

    void reset() noexcept {
        setScore(0);
        hasResult = false;
    }

  private:
    std::unique_ptr<sf::Text> score_  = std::make_unique<sf::Text>();
    std::unique_ptr<sf::Text> result_ = std::make_unique<sf::Text>();
    bool                      hasResult{false};
};
}  // namespace game
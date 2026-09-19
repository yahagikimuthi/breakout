#pragma once

#include <SFML/System/Vector2.hpp>
#include <concepts>
#include <functional>
#include <utility>

namespace game {
template <typename T>
concept Numeric = std::integral<T> or std::floating_point<T>;

template <Numeric T>
struct Vector2 final {
    T x;
    T y;

    template <typename U>
    constexpr operator sf::Vector2<U>() const noexcept {
        return sf::Vector2<U>{static_cast<U>(x), static_cast<U>(y)};
    }
};

template <std::invocable<> F>
class ScopeExit final {
  public:
    [[nodiscard]] explicit constexpr ScopeExit(F f) noexcept : f_{std::move(f)} {}
    ScopeExit(const ScopeExit&) noexcept                    = delete;
    ScopeExit(ScopeExit&&) noexcept                         = delete;
    auto operator=(const ScopeExit&) noexcept -> ScopeExit& = delete;
    auto operator=(ScopeExit&&) noexcept -> ScopeExit&      = delete;
    ~ScopeExit() noexcept { std::invoke(f_); }

  private:
    F f_;
};

template <std::invocable<> F>
[[nodiscard]] auto makeScopeExit(F&& f) noexcept -> ScopeExit<F> {
    return ScopeExit<F>{f};
}
}  // namespace game
#pragma once

#include "util.hpp"

namespace game::setting::window {
static constexpr auto size = Vector2{.x = 800, .y = 600};
}

namespace game::setting::paddle {
static constexpr auto position = Vector2{.x = 300.f, .y = 500.f};
static constexpr auto size     = Vector2{.x = 100.f, .y = 20.f};
}  // namespace game::setting::paddle

namespace game::setting::ball {
static constexpr auto radius          = 10.f;
static constexpr auto defaultSpeed    = 300.f;
static constexpr auto maxSpeed        = 500.f;
static constexpr auto minYSpeed       = 50.f;
static constexpr auto paddleInfluence = 0.5f;
}  // namespace game::setting::ball

namespace game::setting::block {
constexpr auto rows    = 5;
constexpr auto cols    = 10;
constexpr auto width   = 60.f;
constexpr auto height  = 20.f;
constexpr auto padding = 10.f;
constexpr auto offsetX =
    ((window::size.x - ((cols * (width + padding)) - padding)) / 2.f) + (width / 2.f);
constexpr auto offsetY = 60.f + (height / 2.f);
}  // namespace game::setting::block
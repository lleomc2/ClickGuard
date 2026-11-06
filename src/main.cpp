#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <array>
#include <chrono>

using namespace geode::prelude;

// Filter only very fast repeated DOWN events. UP events always pass through.
class $modify(ClickGuardBaseLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        if (down && Mod::get()->getSettingValue<bool>("enabled")) {
            const int windowMs = Mod::get()->getSettingValue<int>("debounce-ms");
            if (windowMs > 0 && button >= 0 && button < 16) {
                using Clock = std::chrono::steady_clock;
                static std::array<std::array<Clock::time_point, 16>, 2> lastAccepted{};
                const auto now = Clock::now();
                const size_t player = isPlayer1 ? 0u : 1u;
                const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - lastAccepted[player][static_cast<size_t>(button)]
                ).count();
                if (elapsed < windowMs) return;
                lastAccepted[player][static_cast<size_t>(button)] = now;
            }
        }
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }
};

$on_mod(Loaded) {
    log::info("ClickGuard loaded for Geometry Dash 2.2081.");
}

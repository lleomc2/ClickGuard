#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <algorithm>
#include <deque>
#include <string>
#include <unordered_map>

using namespace geode::prelude;

namespace clickguard {

struct PendingInput {
    bool down;
    int button;
    bool isPlayer1;
    int ticksLeft;
};

struct LayerState {
    std::deque<PendingInput> pending;
    bool injecting = false;
};

// Keyed by the real game-layer pointer so both GJBaseGameLayer and PlayLayer
// hooks can safely access and clear the same queue.
inline std::unordered_map<GJBaseGameLayer*, LayerState> layerStates;

inline int configuredTicks() {
    if (!Mod::get()->getSettingValue<bool>("enabled")) {
        return 0;
    }
    return std::clamp(Mod::get()->getSettingValue<int>("buffer-ticks"), 0, 240);
}

inline int nextTicks(int current) {
    switch (current) {
        case 2: return 4;
        case 4: return 8;
        case 8: return 16;
        case 16: return 32;
        case 32: return 60;
        case 60: return 120;
        case 120: return 180;
        case 180: return 240;
        case 240: return 0;
        default: return 2;
    }
}

void showBufferPanel() {
    const int ticks = configuredTicks();
    const std::string current = ticks == 0
        ? "OFF (raw input)"
        : std::to_string(ticks) + " ticks";

    createQuickPopup(
        "ClickGuard",
        "INPUT BUFFER\n\nCurrent window: " + current +
        "\n\nNEXT cycles through 2, 4, 8, 16, 32, 60, 120, 180, 240 ticks and OFF. "
        "One tick is 1/240 second. Larger values add more delay.\n\n"
        "This is a tick-based input queue, not AfterFlow's full ghost-player prediction.",
        "CLOSE",
        "NEXT",
        [](FLAlertLayer*, bool chooseNext) {
            if (!chooseNext) {
                return;
            }

            auto* mod = Mod::get();
            const int current = mod->getSettingValue<bool>("enabled")
                ? std::clamp(mod->getSettingValue<int>("buffer-ticks"), 0, 240)
                : 0;
            const int next = nextTicks(current);
            mod->setSettingValue<int>("buffer-ticks", next);
            mod->setSettingValue<bool>("enabled", next != 0);

            Loader::get()->queueInMainThread([] {
                showBufferPanel();
            });
        }
    );
}

} // namespace clickguard

class $modify(ClickGuardBaseLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        auto& fields = clickguard::layerStates[this];
        const int ticks = clickguard::configuredTicks();

        if (fields.injecting || ticks <= 0) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }

        // Apply the selected tick window in every mode, including Wave.
        const int delay = ticks;

        // Avoid unbounded growth if an input source floods the queue.
        if (fields.pending.size() >= 512) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }

        fields.pending.push_back({ down, button, isPlayer1, delay });
    }

    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        auto& fields = clickguard::layerStates[this];

        // If the user turns the buffer off while paused, flush queued events
        // in their original order instead of leaving an input stuck.
        if (clickguard::configuredTicks() <= 0 && !fields.pending.empty()) {
            fields.injecting = true;
            for (const auto& input : fields.pending) {
                GJBaseGameLayer::handleButton(input.down, input.button, input.isPlayer1);
            }
            fields.pending.clear();
            fields.injecting = false;
        }

        for (auto it = fields.pending.begin(); it != fields.pending.end();) {
            --it->ticksLeft;
            if (it->ticksLeft <= 0) {
                const auto input = *it;
                it = fields.pending.erase(it);

                fields.injecting = true;
                GJBaseGameLayer::handleButton(input.down, input.button, input.isPlayer1);
                fields.injecting = false;
            } else {
                ++it;
            }
        }

        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
    }
};

class $modify(ClickGuardPlayLayer, PlayLayer) {
    void resetLevel() {
        clickguard::layerStates.erase(static_cast<GJBaseGameLayer*>(this));
        PlayLayer::resetLevel();
    }

    void onExit() {
        clickguard::layerStates.erase(static_cast<GJBaseGameLayer*>(this));
        PlayLayer::onExit();
    }
};

class $modify(ClickGuardPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto* menu = CCMenu::create();
        auto* icon = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
        if (!menu || !icon) {
            return;
        }

        auto* button = CCMenuItemSpriteExtra::create(
            icon,
            this,
            menu_selector(ClickGuardPauseLayer::onClickGuard)
        );
        if (!button) {
            return;
        }

        const auto winSize = CCDirector::sharedDirector()->getWinSize();
        button->setScale(0.72f);
        button->setPosition(CCPointZero);
        menu->setPosition({ winSize.width - 35.f, winSize.height - 43.f });
        menu->addChild(button);
        this->addChild(menu, 1000);
    }

    void onClickGuard(CCObject*) {
        clickguard::showBufferPanel();
    }
};

$on_mod(Loaded) {
    log::info("ClickGuard tick buffer loaded for Geometry Dash 2.2081 Android64.");
}

#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

class $modify(MyLevelInfoLayer, LevelInfoLayer) {
    struct Fields {
        CCLabelBMFont* placementLabel = nullptr;
        async::TaskHolder<web::WebResponse> m_listener;
    };

    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) {
            return false;
        }

        this->createPlacementLabel();
        this->fetchPlacementForLevel(level);

        return true;
    }

    void createPlacementLabel() {
        if (this->m_fields->placementLabel) {
            return;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto label = CCLabelBMFont::create("Loading...", "goldFont.fnt");
        label->setPosition({ winSize.width / 2.f + 140.f, winSize.height / 2.f - 55.f });
        label->setScale(0.5f);

        this->addChild(label);
        this->m_fields->placementLabel = label;
    }

    void fetchPlacementForLevel(GJGameLevel* level) {
        if (!level) return;

        int levelId = level->m_levelID;

        auto req = web::WebRequest();

        m_fields->m_listener.spawn(
            req.get("https://script.google.com/macros/s/AKfycbxS9jpaqI_nY6FfLUOZD-SYQt-8i5OFa-1M5P0tJ8bhKupavdA7JMbGNhD4kAhmvDZj/exec"),
            [this, levelId](web::WebResponse res) {
                if (!m_fields->placementLabel) return;

                if (!res.ok()) {
                    m_fields->placementLabel->setString("Failed!");
                    m_fields->placementLabel->setColor({ 255, 80, 80 });
                    return;
                }

                auto parsed = res.json();
                if (!parsed.isOk()) {
                    m_fields->placementLabel->setString("Failed!");
                    m_fields->placementLabel->setColor({ 255, 80, 80 });
                    return;
                }

                auto json = parsed.unwrap();
                int placement = -1;

                if (auto arrRes = json.asArray(); arrRes.isOk()) {
                    for (auto& item : arrRes.unwrap()) {
                        if (!item.isObject()) continue;

                        auto idRes = item["ID"].template as<int>();
                        if (!idRes.isOk()) continue;

                        if (idRes.unwrap() == levelId) {
                            auto placementRes = item[" Ranking"].template as<int>();
                            if (placementRes.isOk()) {
                                placement = placementRes.unwrap();
                            }
                            break;
                        }
                    }
                } else {
                    m_fields->placementLabel->setString("Failed!");
                    m_fields->placementLabel->setColor({ 255, 80, 80 });
                    return;
                }

                if (placement > 0) {
                    auto str = fmt::format("No. {}", placement);
                    m_fields->placementLabel->setString(str.c_str());
                } else {
                    m_fields->placementLabel->setString("N/A");
                }
            }
        );
    }
};

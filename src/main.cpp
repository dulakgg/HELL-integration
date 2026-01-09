#include <Geode/Geode.hpp>
#include <Geode/modify/LevelCell.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>

using namespace geode::prelude;

class $modify(MyLevelInfoLayer, LevelInfoLayer) {
    struct Fields {
        EventListener<web::WebTask> placementListener;
        CCLabelBMFont* placementLabel = nullptr;
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

        auto label = CCLabelBMFont::create("N/A", "goldFont.fnt");
        label->setPosition({ winSize.width / 2.f + 140.f, winSize.height / 2.f - 55.f });
        label->setScale(0.5f);

        this->addChild(label);
        this->m_fields->placementLabel = label;
    }

    void fetchPlacementForLevel(GJGameLevel* level) {
        if (!level) return;

        int levelId = level->m_levelID;

        auto url = std::string("https://script.google.com/macros/s/AKfycbxS9jpaqI_nY6FfLUOZD-SYQt-8i5OFa-1M5P0tJ8bhKupavdA7JMbGNhD4kAhmvDZj/exec");

        this->m_fields->placementListener.bind([self = Ref(this), levelId](web::WebTask::Event* e) {
            if (!self) return;

            auto* res = e->getValue();
            if (!res) {
                queueInMainThread([self] {
                    if (!self || !self->m_fields->placementLabel) return;
                    self->m_fields->placementLabel->setString("...");
                });
                return;
            }

            auto parsed = res->json();
            if (!parsed.isOk()) {
                queueInMainThread([self] {
                    if (!self || !self->m_fields->placementLabel) return;
                    self->m_fields->placementLabel->setString("N/A");
                });
                return;
            }

            auto json = parsed.unwrap();

            int placement = -1;

            if (json.isArray()) {
                auto arrRes = json.asArray();
                if (arrRes.isOk()) {
                    auto& arr = arrRes.unwrap();
                    for (auto& item : arr) {
                        if (!item.isObject()) continue;

                        auto idRes = item["ID"].as<int>();
                        if (!idRes.isOk()) continue;

                        if (idRes.unwrap() == levelId) {
                            auto placementRes = item["Placement"].as<int>();
                            if (placementRes.isOk()) {
                                placement = placementRes.unwrap();
                            }
                            break;
                        }
                    }
                }
            }

            queueInMainThread([self, placement] {
                if (!self || !self->m_fields->placementLabel) return;

                if (placement > 0) {
                    auto str = CCString::createWithFormat("No. %d", placement);
                    self->m_fields->placementLabel->setString(str->getCString());
                } else {
                    self->m_fields->placementLabel->setString("N/A");
                }
            });
        });

        auto req = web::WebRequest();
        this->m_fields->placementListener.setFilter(req.get(url));
    }
};

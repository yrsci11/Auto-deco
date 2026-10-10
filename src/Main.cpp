#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <random>
#include <set>
#include <utility>
#include <vector>

using namespace geode::prelude;

namespace {
	constexpr float GRID = 30.f;
	using Cell = std::pair<int, int>;

	Cell toCell(CCPoint p) {
		return { (int)std::round(p.x / GRID), (int)std::round(p.y / GRID) };
	}
}

class $modify(AutoDecoUI, EditorUI) {
	bool init(LevelEditorLayer* lel) {
		if (!EditorUI::init(lel)) return false;

		auto spr = ButtonSprite::create("Deco");
		auto btn = CCMenuItemSpriteExtra::create(
			spr, this, menu_selector(AutoDecoUI::onAutoDeco)
		);

		auto menu = CCMenu::create();
		menu->addChild(btn);
		auto win = CCDirector::get()->getWinSize();
		menu->setPosition({ win.width - 45.f, win.height - 70.f });
		this->addChild(menu);

		return true;
	}

	void onAutoDeco(CCObject*) {
		auto objs = this->getSelectedObjects();
		if (!objs || objs->count() == 0) {
			FLAlertLayer::create("Auto Deco", "Select some blocks first!", "OK")->show();
			return;
		}

		int decoID = static_cast<int>(Mod::get()->getSettingValue<int64_t>("deco-id"));
		double chance = Mod::get()->getSettingValue<double>("chance");

		// Collect the grid cells occupied by the selection.
		std::set<Cell> occupied;
		std::vector<CCPoint> positions;
		CCObject* it;
		CCARRAY_FOREACH(objs, it) {
			auto obj = static_cast<GameObject*>(it);
			occupied.insert(toCell(obj->getPosition()));
			positions.push_back(obj->getPosition());
		}

		static std::mt19937 rng{ std::random_device{}() };
		std::uniform_real_distribution<double> roll(0.0, 1.0);

		struct Side { int dx, dy; float rot; };
		const Side sides[] = {
			{  0,  1,   0.f },  // top
			{  1,  0,  90.f },  // right
			{  0, -1, 180.f },  // bottom
			{ -1,  0, 270.f },  // left
		};

		int placed = 0;
		for (auto const& pos : positions) {
			Cell c = toCell(pos);
			for (auto const& s : sides) {
				// Only decorate edges that touch empty space.
				if (occupied.count({ c.first + s.dx, c.second + s.dy })) continue;
				if (roll(rng) > chance) continue;

				CCPoint target = {
					pos.x + s.dx * GRID,
					pos.y + s.dy * GRID
				};
				auto deco = m_editorLayer->createObject(decoID, target, true);
				if (deco) {
					deco->setRotation(s.rot);
					placed++;
				}
			}
		}

		FLAlertLayer::create(
			"Auto Deco",
			fmt::format("Placed {} decoration objects.", placed),
			"OK"
		)->show();
	}
};

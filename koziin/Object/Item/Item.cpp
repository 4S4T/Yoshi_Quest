#include "Item.h"
#include <algorithm>	 // erase/remove_if 用
#include <unordered_set> // 取得済みIDの永続領域

// ------------- 追加: 取得済みIDの永続ストレージ（プロセス内で維持） -------------
namespace {
	static std::unordered_set<int> g_collectedItemIds;
}

namespace ItemPersistence {
	bool IsCollected(int id) {
		return g_collectedItemIds.find(id) != g_collectedItemIds.end();
	}
	void MarkCollected(int id) {
		g_collectedItemIds.insert(id);
	}
}
// --------------------------------------------------------------

Item::Item(int id_, const std::string& name, const Vector2D& pos,
	ItemType type_, EquipCategory category_,
	int atkAddValue, int defAddValue, int hpAddValue, int healAmount)
	: id(id_), itemName(name), position(pos), collected(false),
	  type(type_), category(category_),
	  attackAddValue(atkAddValue), defenseAddValue(defAddValue),
	  hpAddValue(hpAddValue), healAmount(healAmount) {
}

int Item::GetId() const {
	return id;
}

const std::string& Item::GetName() const {
	return itemName;
}

const Vector2D& Item::GetPosition() const {
	return position;
}

bool Item::IsCollected() const {
	return collected;
}

void Item::Collect() {
	collected = true;
	// 取得済みに記録 → 次回生成時に出現させない
	ItemPersistence::MarkCollected(id);
}

ItemType Item::GetType() const {
	return type;
}

EquipCategory Item::GetCategory() const {
	return category;
}

int Item::GetAttackAddValue() const {
	return attackAddValue;
}

int Item::GetDefenseAddValue() const {
	return defenseAddValue;
}

int Item::GetHpAddValue() const {
	return hpAddValue;
}

int Item::GetHealAmount() const {
	return healAmount;
}

// マップに配置する初期アイテム
std::vector<std::shared_ptr<Item>> GenerateMapItems() {
	std::vector<std::shared_ptr<Item>> items;

	// 消費アイテム
	items.push_back(std::make_shared<Item>(1, "回復薬", Vector2D(500, 600),ItemType::Consumable, EquipCategory::None,0, 0, 0, 20));

	// Item( id,   名前,        位置,                 種類,                装備種別,                 攻撃補正, 防御補正, HP補正, 回復量 )
	// 装備アイテム
	items.push_back(std::make_shared<Item>(2, "錆びた剣", Vector2D(300, 600),ItemType::Equipment, EquipCategory::Weapon,10, 0, 0, 0));
	

	items.push_back(std::make_shared<Item>(3, "盾", Vector2D(550, 420),ItemType::Equipment, EquipCategory::Shield,0, 3, 0, 0));

	items.push_back(std::make_shared<Item>(4, "鎧", Vector2D(580, 450),ItemType::Equipment, EquipCategory::Armor,0, 0, 10, 0));

	items.push_back(std::make_shared<Item>(5, "ヘルメット+20", Vector2D(600, 480),ItemType::Equipment, EquipCategory::Helmet,0, 2, 0, 0));


	items.erase(
		std::remove_if(items.begin(), items.end(),
			[](const std::shared_ptr<Item>& it) {
				return ItemPersistence::IsCollected(it->GetId());
			}),
		items.end());
	

	return items;
}

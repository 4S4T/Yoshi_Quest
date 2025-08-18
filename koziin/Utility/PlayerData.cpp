#include "PlayerData.h"

// コンストラクタ
PlayerData::PlayerData()
	: hp(30), maxHp(30), attack(10), defense(5),
	  experience(0), level(1),
	  equippedWeaponId(-1), equippedShieldId(-1),
	  equippedArmorId(-1), equippedHelmetId(-1) {}

PlayerData::~PlayerData() {}

// ステータス設定
void PlayerData::SetAttack(int value) { attack = value; }
void PlayerData::SetHp(int value) {
	hp = value;
	if (hp > maxHp)
		hp = maxHp;
	if (hp < 0)
		hp = 0;
}
void PlayerData::SetDefense(int value) { defense = value; }

// 経験値追加
void PlayerData::AddExperience(int exp) {
	if (exp > 0) {
		experience += exp;
		while (experience >= GetExperienceRequiredForLevel(level)) {
			experience -= GetExperienceRequiredForLevel(level);
			LevelUp();
		}
	}
}

// レベルアップ
void PlayerData::LevelUp() {
	++level;
	attack += 5;
	defense += 3;
	maxHp += 10;
	hp = maxHp; // 全回復
}

// ステータス取得
int PlayerData::GetHp() const { return hp; }
int PlayerData::GetAttack() const { return attack; }
int PlayerData::GetDefense() const { return defense; }
int PlayerData::GetExperience() const { return experience; }
int PlayerData::GetLevel() const { return level; }
int PlayerData::GetMaxHp() const { return maxHp; }

// 必要経験値計算
int PlayerData::GetExperienceRequiredForLevel(int currentLevel) const {
	return currentLevel * 100;
}

// ===== アイテム管理 =====
void PlayerData::AddItem(const Item& item) {
	// 既存IDがあれば追加失敗して上書きしない運用（そのままでOK）
	// 上書きしたいなら: ownedItems[item.GetId()] = item;
	ownedItems.insert({ item.GetId(), item });
}

bool PlayerData::IsCollected(int id) const {
	return ownedItems.find(id) != ownedItems.end();
}

std::map<std::string, int> PlayerData::GetConsumableCounts() const {
	std::map<std::string, int> counts;
	for (auto& kv : ownedItems) {
		if (kv.second.GetType() == ItemType::Consumable) {
			counts[kv.second.GetName()]++;
		}
	}
	return counts;
}

std::map<std::string, int> PlayerData::GetAllItemCounts() const {
	std::map<std::string, int> counts;
	for (auto& kv : ownedItems) {
		counts[kv.second.GetName()]++;
	}
	return counts;
}

const std::map<int, Item>& PlayerData::GetOwnedItems() const {
	return ownedItems;
}

// ===== 装備管理 =====
void PlayerData::EquipItem(EquipCategory category, int itemId) {
	if (!IsCollected(itemId))
		return;
	switch (category) {
	case EquipCategory::Weapon:
		equippedWeaponId = itemId;
		break;
	case EquipCategory::Shield:
		equippedShieldId = itemId;
		break;
	case EquipCategory::Armor:
		equippedArmorId = itemId;
		break;
	case EquipCategory::Helmet:
		equippedHelmetId = itemId;
		break;
	default:
		break;
	}
}

std::string PlayerData::GetEquippedName(EquipCategory category) const {
	int id = -1;
	switch (category) {
	case EquipCategory::Weapon:
		id = equippedWeaponId;
		break;
	case EquipCategory::Shield:
		id = equippedShieldId;
		break;
	case EquipCategory::Armor:
		id = equippedArmorId;
		break;
	case EquipCategory::Helmet:
		id = equippedHelmetId;
		break;
	default:
		return "なし";
	}
	auto it = ownedItems.find(id);
	return (it != ownedItems.end()) ? it->second.GetName() : "なし";
}

// ===== 所持品クリア =====
void PlayerData::ClearCollectedItems() {
	ownedItems.clear();
	equippedWeaponId = equippedShieldId = equippedArmorId = equippedHelmetId = -1;
}


int PlayerData::GetEquippedId(EquipCategory category) const {
	switch (category) {
	case EquipCategory::Weapon:
		return equippedWeaponId;
	case EquipCategory::Shield:
		return equippedShieldId;
	case EquipCategory::Armor:
		return equippedArmorId;
	case EquipCategory::Helmet:
		return equippedHelmetId;
	default:
		return -1;
	}
}

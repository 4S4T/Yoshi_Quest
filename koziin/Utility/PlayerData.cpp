#include "PlayerData.h"

// コンストラクタ
PlayerData::PlayerData()
	: hp(30), maxHp(30), attack(10), defense(5),
	  mp(10), maxMp(10), // ★ MP 初期化
	  experience(0), level(1),
	  equippedWeaponId(-1), equippedShieldId(-1),
	  equippedArmorId(-1), equippedHelmetId(-1) {
}

PlayerData::~PlayerData() {
}

// =========================
// ステータス設定／取得
// =========================

void PlayerData::SetAttack(int value) { attack = value; }

void PlayerData::SetHp(int value) {
	hp = value;
	int cap = GetMaxHp(); // 装備補正込み
	if (hp > cap)
		hp = cap;
	if (hp < 0)
		hp = 0;
}

void PlayerData::SetDefense(int value) { defense = value; }

void PlayerData::AddExperience(int exp) {
	if (exp > 0) {
		experience += exp;
		while (experience >= GetExperienceRequiredForLevel(level)) {
			experience -= GetExperienceRequiredForLevel(level);
			LevelUp();
		}
	}
}

void PlayerData::LevelUp() {
	++level;
	attack += 5;
	defense += 3;
	maxHp += 10;
	hp = maxHp;

	// ★ MP 成長＆全回復
	maxMp += 4;
	mp = maxMp;

	// ★ 魔法習得タイミング（例）
	if (level == 2)
		LearnMagic(MagicType::Fire);
	if (level == 3)
		LearnMagic(MagicType::Heal);
	if (level == 4)
		LearnMagic(MagicType::Flare); // ★ 追加：Lv4で習得
	if (level == 5)
		LearnMagic(MagicType::Thunder);
}


int PlayerData::GetHp() const { return hp; }
int PlayerData::GetAttack() const { return attack + GetEquipAttackBonus(); }
int PlayerData::GetDefense() const { return defense + GetEquipDefenseBonus(); }
int PlayerData::GetExperience() const { return experience; }
int PlayerData::GetLevel() const { return level; }
int PlayerData::GetMaxHp() const { return maxHp + GetEquipHpBonus(); }

// ★ MP アクセサ
void PlayerData::SetMp(int value) {
	mp = value;
	if (mp < 0)
		mp = 0;
	if (mp > GetMaxMp())
		mp = GetMaxMp();
}
int PlayerData::GetMp() const { return mp; }
int PlayerData::GetMaxMp() const { return maxMp; } // 装備補正を乗せる場合はここで加算
bool PlayerData::ConsumeMp(int cost) {
	if (cost <= 0)
		return true;
	if (mp >= cost) {
		mp -= cost;
		return true;
	}
	return false;
}
bool PlayerData::HasMp(int cost) const {
	return (cost <= 0) || (mp >= cost);
}

// ★装備補正の合算（攻撃）
int PlayerData::GetEquipAttackBonus() const {
	int bonus = 0;
	auto add = [&](int id) {
		auto it = ownedItems.find(id);
		if (it != ownedItems.end())
			bonus += it->second.GetAttackAddValue();
	};
	add(equippedWeaponId);
	add(equippedShieldId);
	add(equippedArmorId);
	add(equippedHelmetId);
	return bonus;
}

// ★装備補正の合算（防御）
int PlayerData::GetEquipDefenseBonus() const {
	int bonus = 0;
	auto add = [&](int id) {
		auto it = ownedItems.find(id);
		if (it != ownedItems.end())
			bonus += it->second.GetDefenseAddValue();
	};
	add(equippedWeaponId);
	add(equippedShieldId);
	add(equippedArmorId);
	add(equippedHelmetId);
	return bonus;
}

// ★装備補正の合算（最大HP）
int PlayerData::GetEquipHpBonus() const {
	int bonus = 0;
	auto add = [&](int id) {
		auto it = ownedItems.find(id);
		if (it != ownedItems.end())
			bonus += it->second.GetHpAddValue();
	};
	add(equippedWeaponId);
	add(equippedShieldId);
	add(equippedArmorId);
	add(equippedHelmetId);
	return bonus;
}

int PlayerData::GetExperienceRequiredForLevel(int currentLevel) const {
	return currentLevel * 100;
}

// =========================
// アイテム管理（在庫）
// =========================

void PlayerData::AddItem(const Item& item) {
	// 既存IDがあれば追加失敗して上書きしない運用（必要に応じて変更）
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

bool PlayerData::UseItem(int itemId) {
	auto it = ownedItems.find(itemId);
	if (it == ownedItems.end())
		return false; // 所持してない

	Item& item = it->second;
	if (item.GetType() != ItemType::Consumable)
		return false; // 消費アイテムじゃない

	int heal = item.GetHealAmount();
	if (heal > 0 && hp < GetMaxHp()) {
		SetHp(hp + heal);	  // HPを回復
		ownedItems.erase(it); // 一度きり使用可能にする場合は削除（在庫のみ）
		return true;
	}
	return false;
}

// =========================
// 装備管理
// =========================

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

// =========================
// 所持品クリア（在庫・装備）
// =========================

void PlayerData::ClearCollectedItems() {
	ownedItems.clear();
	equippedWeaponId = equippedShieldId = equippedArmorId = equippedHelmetId = -1;

	// ※拾得“実績”は消さない（必要なら外部から消す）
}

// =========================
// ★ 拾得実績
// =========================
bool PlayerData::HasEverCollected(int id) const {
	return collectedItemIds.count(id) > 0;
}
void PlayerData::MarkMapPickupCollected(int id) {
	collectedItemIds.insert(id);
}

// =========================
// ★ 魔法習得管理
// =========================
bool PlayerData::HasMagic(MagicType t) const { return learnedMagics.count(t) > 0; }
void PlayerData::LearnMagic(MagicType t) { learnedMagics.insert(t); }
std::vector<PlayerData::MagicType> PlayerData::GetLearnedMagics() const {
	return std::vector<MagicType>(learnedMagics.begin(), learnedMagics.end());
}

// 追加：装備IDの取得
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

//装備外し
void PlayerData::Unequip(EquipCategory category) {
	switch (category) {
	case EquipCategory::Weapon:
		equippedWeaponId = -1;
		break;
	case EquipCategory::Shield:
		equippedShieldId = -1;
		break;
	case EquipCategory::Armor:
		equippedArmorId = -1;
		break;
	case EquipCategory::Helmet:
		equippedHelmetId = -1;
		break;
	default:
		break;
	}
}

#include "PlayerData.h"

// コンストラクタ
PlayerData::PlayerData()
	: hp(30), maxHp(30), attack(10), defense(5),
	  experience(0), level(1),
	  equippedWeaponId(-1), equippedShieldId(-1),
	  equippedArmorId(-1), equippedHelmetId(-1)
{


}

PlayerData::~PlayerData()
{

}

// =========================
// ステータス設定／取得
// =========================

// 攻撃力を設定
void PlayerData::SetAttack(int value) 
{ 
	attack = value;
}

// HPを設定
void PlayerData::SetHp(int value)
{
	hp = value;
	int cap = GetMaxHp(); // 装備補正込み
	if (hp > cap)
		hp = cap;
	if (hp < 0)
		hp = 0;
}


// 防御力を設定
void PlayerData::SetDefense(int value)
{
	defense = value;
}

// 経験値追加
void PlayerData::AddExperience(int exp) 
{
	if (exp > 0) {
		experience += exp;
		while (experience >= GetExperienceRequiredForLevel(level)) 
		{
			experience -= GetExperienceRequiredForLevel(level);
			LevelUp();
		}
	}
}

// レベルアップ
void PlayerData::LevelUp() 
{
	++level;
	attack += 5;
	defense += 3;
	maxHp += 10;
	hp = maxHp; // 全回復
}

// ステータス取得
int PlayerData::GetHp() const { return hp; }
int PlayerData::GetAttack() const {return attack + GetEquipAttackBonus();}
int PlayerData::GetDefense() const {return defense + GetEquipDefenseBonus();}
int PlayerData::GetExperience() const { return experience; }
int PlayerData::GetLevel() const { return level; }
int PlayerData::GetMaxHp() const {return maxHp+GetEquipHpBonus();}


// ★装備補正の合算（攻撃）
int PlayerData::GetEquipAttackBonus() const 
{
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

// 必要経験値計算
int PlayerData::GetExperienceRequiredForLevel(int currentLevel) const 
{
	return currentLevel * 100;
}

// =========================
// アイテム管理
// =========================

// 所持品にアイテムを追加
void PlayerData::AddItem(const Item& item) 
{
	// 既存IDがあれば追加失敗して上書きしない運用（そのままでOK）
	// 上書きしたいなら: ownedItems[item.GetId()] = item;
	ownedItems.insert({ item.GetId(), item });
}

// IDで所持判定
bool PlayerData::IsCollected(int id) const 
{
	return ownedItems.find(id) != ownedItems.end();
}

// 「消費アイテム」の所持数を名前ごとに集計して返す
std::map<std::string, int> PlayerData::GetConsumableCounts() const 
{
	std::map<std::string, int> counts;
	for (auto& kv : ownedItems) {
		if (kv.second.GetType() == ItemType::Consumable)
		{
			counts[kv.second.GetName()]++;
		}
	}
	return counts;
}

// 全アイテムの所持数を名前ごとに集計して返す
std::map<std::string, int> PlayerData::GetAllItemCounts() const
{
	std::map<std::string, int> counts;
	for (auto& kv : ownedItems) {
		counts[kv.second.GetName()]++;
	}
	return counts;
}

// ID→アイテム本体の連想配列を参照返し（描画や装備選択に利用）
const std::map<int, Item>& PlayerData::GetOwnedItems() const
{
	return ownedItems;
}

// 消費アイテムを使用
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
		ownedItems.erase(it); // 一度きり使用可能にする場合は削除
		return true;
	}
	return false;
}



// =========================
// 装備管理
// =========================


// 指定カテゴリの装備を itemId に差し替える（所持していないIDは無視）
void PlayerData::EquipItem(EquipCategory category, int itemId) 
{
	if (!IsCollected(itemId))
		return;
	switch (category) 
	{
	case EquipCategory::Weapon://武器
		 equippedWeaponId = itemId;
		break;
	case EquipCategory::Shield://盾
		 equippedShieldId = itemId;
		break;
	case EquipCategory::Armor://防具
		 equippedArmorId = itemId;
		break;
	case EquipCategory::Helmet://頭
		 equippedHelmetId = itemId;
		break;
	default:
		break;
	}
}

// 指定カテゴリの「装備名」を返す（未装備なら「なし」）
std::string PlayerData::GetEquippedName(EquipCategory category) const
{
	int id = -1;
	switch (category) {
	case EquipCategory::Weapon://武器
		id = equippedWeaponId;
		break;
	case EquipCategory::Shield://盾
		id = equippedShieldId;
		break;
	case EquipCategory::Armor://防具
		id = equippedArmorId;
		break;
	case EquipCategory::Helmet://頭
		id = equippedHelmetId;
		break;
	default:
		return "なし";
	}
	auto it = ownedItems.find(id);
	return (it != ownedItems.end()) ? it->second.GetName() : "なし";
}

// =========================
// 所持品クリア
// =========================

// 所持リストを全消去し、すべて未装備状態にする
void PlayerData::ClearCollectedItems() 
{
	ownedItems.clear();
	equippedWeaponId = equippedShieldId = equippedArmorId = equippedHelmetId = -1;
}

// 指定カテゴリの装備IDを返す（未装備は -1）
int PlayerData::GetEquippedId(EquipCategory category) const
{
	switch (category) {
	case EquipCategory::Weapon://武器
		return equippedWeaponId;
	case EquipCategory::Shield://盾
		return equippedShieldId;
	case EquipCategory::Armor://防具
		return equippedArmorId;
	case EquipCategory::Helmet://頭
		return equippedHelmetId;
	default:
		return -1;
	}
}

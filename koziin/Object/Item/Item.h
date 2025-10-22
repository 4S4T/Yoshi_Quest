#pragma once
#include <string>
#include <vector>
#include <memory>
#include "../../Utility/Vector2D.h"

// ===============================
// アイテムの種類
// ===============================
enum class ItemType {
	Consumable, // 消費アイテム
	Equipment	// 装備アイテム
};

// ===============================
// 装備アイテムの種類
// ===============================
enum class EquipCategory {
	None,	// 装備ではない
	Weapon, // 武器
	Shield, // 盾
	Armor,	// 防具（胴体）
	Helmet	// ヘルメット（頭装備）
};

// ------------- 追加: 取得済みアイテムの永続ユーティリティ -------------
namespace ItemPersistence {
	// そのIDのアイテムが既に取得済みか？
	bool IsCollected(int id);

	// そのIDのアイテムを取得済みに記録
	void MarkCollected(int id);
}
// --------------------------------------------------------------

// ===============================
// Itemクラス
// ===============================
// ゲーム内で拾えるアイテムの情報を保持
class Item {
public:
	Item()
		: id(0), itemName(""), position(0, 0), collected(false),
		  type(ItemType::Consumable), category(EquipCategory::None),
		  attackAddValue(0), defenseAddValue(0), hpAddValue(0), healAmount(0) {}

	Item(int id_, const std::string& name, const Vector2D& pos,
		ItemType type_, EquipCategory category_,
		int atkAddValue = 0, int defAddValue = 0, int hpAddValue = 0, int healAmount = 0);

	// 基本情報取得
	int GetId() const;
	const std::string& GetName() const;
	const Vector2D& GetPosition() const;

	// 取得フラグ操作
	bool IsCollected() const;
	void Collect();

	// 属性取得
	ItemType GetType() const;
	EquipCategory GetCategory() const;

	// 能力値追加
	int GetAttackAddValue() const;
	int GetDefenseAddValue() const;
	int GetHpAddValue() const;

	// 消費アイテム用
	int GetHealAmount() const;

private:
	int id;
	std::string itemName;
	Vector2D position;
	bool collected = false;

	ItemType type;
	EquipCategory category;

	int attackAddValue;
	int defenseAddValue;
	int hpAddValue;
	int healAmount;
};

// 初期配置アイテム生成
std::vector<std::shared_ptr<Item>> GenerateMapItems();

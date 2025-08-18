#include "Item.h"

Item::Item(int id_, const std::string& name, const Vector2D& pos,
	ItemType type_, EquipCategory category_,
	int atkAddValue, int defAddValue, int hpAddValue, int healAmount)
	: id(id_), itemName(name), position(pos), collected(false),
	  type(type_), category(category_),
	  attackAddValue(atkAddValue), defenseAddValue(defAddValue),
	  hpAddValue(hpAddValue), healAmount(healAmount)
{

}

int Item::GetId() const
{ 
	return id;  
}

const std::string& Item::GetName() const
{ 
	return itemName;
}
const Vector2D& Item::GetPosition() const 
{
	return position;
}
bool Item::IsCollected() const 
{ 
	return collected; 
 }
void Item::Collect()
{ 
	collected = true;
}

ItemType Item::GetType() const
{ 
	return type;
}
EquipCategory Item::GetCategory() const 
{ 
	return category;
}
int Item::GetAttackAddValue() const 
{ 
	return attackAddValue;
}
int Item::GetDefenseAddValue() const 
{ 
	return defenseAddValue;
}
int Item::GetHpAddValue() const
{ 
	return hpAddValue;
}
int Item::GetHealAmount() const 
{ 
	return healAmount; 
}

// マップに配置する初期アイテム
std::vector<std::shared_ptr<Item>> GenerateMapItems() {
	std::vector<std::shared_ptr<Item>> items;

	// 消費アイテム
	items.push_back(std::make_shared<Item>(1, "回復薬", Vector2D(300, 600),ItemType::Consumable, EquipCategory::None,0, 0, 0, 20));

	// 装備アイテム
	items.push_back(std::make_shared<Item>(2, "日輪刀DX", Vector2D(500, 400),ItemType::Equipment, EquipCategory::Weapon,5, 0, 0, 0));

	items.push_back(std::make_shared<Item>(3, "盾", Vector2D(550, 420),ItemType::Equipment, EquipCategory::Shield,0, 3, 0, 0));

	items.push_back(std::make_shared<Item>(4, "鎧", Vector2D(580, 450),ItemType::Equipment, EquipCategory::Armor,0, 0, 10, 0));

	items.push_back(std::make_shared<Item>(5, "鉄のヘルメット", Vector2D(600, 480),ItemType::Equipment, EquipCategory::Helmet,0, 2, 5, 0));

	return items;
}

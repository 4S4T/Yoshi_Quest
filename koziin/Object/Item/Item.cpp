#include "Item.h"

// コンストラクタ（ヘッダの宣言に合わせて id を受け取る）
Item::Item(int id_, const std::string& name, const Vector2D& pos)
	: id(id_), itemName(name), position(pos), collected(false) {}

// IDゲッター
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
}

// ★ 初期アイテム生成関数
std::vector<std::shared_ptr<Item>> GenerateMapItems() {
	std::vector<std::shared_ptr<Item>> items;

	items.push_back(std::make_shared<Item>(1, "回復薬", Vector2D(300, 600)));
	items.push_back(std::make_shared<Item>(2, "回復薬", Vector2D(200, 400)));
	items.push_back(std::make_shared<Item>(3, "日輪刀", Vector2D(500, 400)));
	// 他にも追加可能

	return items;
}

// Item.cpp
#include "Item.h"

Item::Item(const std::string& name, const Vector2D& pos)
    : itemName(name), position(pos), collected(false) {}

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

// �� �����A�C�e�������֐�
std::vector<std::shared_ptr<Item>> GenerateMapItems() {
    std::vector<std::shared_ptr<Item>> items;

    items.push_back(std::make_shared<Item>("�񕜖�", Vector2D(300, 600)));
	items.push_back(std::make_shared<Item>("�񕜖�", Vector2D(200, 400)));
    items.push_back(std::make_shared<Item>("���@�̐�", Vector2D(500, 400)));
    // ���ɂ��ǉ��\

    return items;
}

#include "Item.h"

// �R���X�g���N�^�i�w�b�_�̐錾�ɍ��킹�� id ���󂯎��j
Item::Item(int id_, const std::string& name, const Vector2D& pos)
	: id(id_), itemName(name), position(pos), collected(false) {}

// ID�Q�b�^�[
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

// �� �����A�C�e�������֐�
std::vector<std::shared_ptr<Item>> GenerateMapItems() {
	std::vector<std::shared_ptr<Item>> items;

	items.push_back(std::make_shared<Item>(1, "�񕜖�", Vector2D(300, 600)));
	items.push_back(std::make_shared<Item>(2, "�񕜖�", Vector2D(200, 400)));
	items.push_back(std::make_shared<Item>(3, "���֓�", Vector2D(500, 400)));
	// ���ɂ��ǉ��\

	return items;
}

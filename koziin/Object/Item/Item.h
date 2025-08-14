#pragma once
#include <string>
#include <vector>
#include <memory>
#include "../../Utility/Vector2D.h"

class Item {
public:
	Item(int id, const std::string& name, const Vector2D& pos); // id ���󂯎��
	int GetId() const;
	const std::string& GetName() const;
	const Vector2D& GetPosition() const;
	bool IsCollected() const;
	void Collect();

private:
	int id;
	std::string itemName;
	Vector2D position;
	bool collected = false;
};

// �� Map�p�F�����A�C�e������
std::vector<std::shared_ptr<Item>> GenerateMapItems();

#pragma once
#include <string>
#include <vector>
#include <memory>
#include "../../Utility/Vector2D.h"

// ===============================
// �A�C�e���̎��
// ===============================
enum class ItemType {
	Consumable, // ����A�C�e��
	Equipment	// �����A�C�e��
};

// ===============================
// �����A�C�e���̎��
// ===============================
enum class EquipCategory {
	None,	// �����ł͂Ȃ�
	Weapon, // ����
	Shield, // ��
	Armor,	// �h��i���́j
	Helmet	// �w�����b�g�i�������j
};

// ------------- �ǉ�: �擾�ς݃A�C�e���̉i�����[�e�B���e�B -------------
namespace ItemPersistence {
	// ����ID�̃A�C�e�������Ɏ擾�ς݂��H
	bool IsCollected(int id);

	// ����ID�̃A�C�e�����擾�ς݂ɋL�^
	void MarkCollected(int id);
}
// --------------------------------------------------------------

// ===============================
// Item�N���X
// ===============================
// �Q�[�����ŏE����A�C�e���̏���ێ�
class Item {
public:
	Item()
		: id(0), itemName(""), position(0, 0), collected(false),
		  type(ItemType::Consumable), category(EquipCategory::None),
		  attackAddValue(0), defenseAddValue(0), hpAddValue(0), healAmount(0) {}

	Item(int id_, const std::string& name, const Vector2D& pos,
		ItemType type_, EquipCategory category_,
		int atkAddValue = 0, int defAddValue = 0, int hpAddValue = 0, int healAmount = 0);

	// ��{���擾
	int GetId() const;
	const std::string& GetName() const;
	const Vector2D& GetPosition() const;

	// �擾�t���O����
	bool IsCollected() const;
	void Collect();

	// �����擾
	ItemType GetType() const;
	EquipCategory GetCategory() const;

	// �\�͒l�ǉ�
	int GetAttackAddValue() const;
	int GetDefenseAddValue() const;
	int GetHpAddValue() const;

	// ����A�C�e���p
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

// �����z�u�A�C�e������
std::vector<std::shared_ptr<Item>> GenerateMapItems();

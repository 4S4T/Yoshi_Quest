#pragma once

#include "../SceneBase.h"
#include "../../Utility/Vector2D.h"
#include "../../Object/Charactor/Player/Player.h"
#include "../../Object/NPC/NPC.h"
#include <memory>
#include <string>
#include <vector>
#include <random>
#include "../../Object/Item/Item.h"

class Map : public SceneBase {
private:
	// �}�b�v�J�ڃ|�C���g
	std::vector<Vector2D> transitionPoints;

	// �Փ˔���}�b�v
	std::vector<std::vector<bool>> collisionMap;

	// NCP �Q
	std::vector<std::shared_ptr<NCP>> ncps;

	// �X�e�[�W
	std::vector<std::string> stageFiles = {
		"Resource/stage1.csv",
		"Resource/stage2.csv",
		"Resource/stage3.csv"
	};
	int currentStageIndex = 0;

	// �G���J�E���g
	int encounterStepCounter = 0;
	Vector2D lastPlayerPos;

	// ===== ���j���[�֘A =====
	bool isMenuVisible = false;
	int menuSelection = 0;
	static constexpr int menuItemCount = 4;
	const char* menuItems[menuItemCount] = { "�ǂ���", "������", "�悳", "���ǂ�" };

	// �t�H�[�J�XON=�E���X�g���쒆 / OFF=�����j���[���쒆
	bool isSubMenuVisible = false; // �t�H�[�J�X�Ǘ��Ɏg�p
	std::string subMenuText;	   // ���ʕ\���e�L�X�g

	std::vector<std::shared_ptr<Item>> items;

	// �ǂ������X�g�i�O���[�v�\���p�j
	struct MenuEntry {
		int representativeId;
		std::string name;
		EquipCategory category; // Consumable �̏ꍇ�� None
		ItemType type;			// Equipment / Consumable
		int count;				// ���i������1�j
		bool equipped;			// �������i�����i�̂݁j
	};
	std::vector<MenuEntry> groupedItems;
	int subMenuSelection = 0;
	int listScrollOffset = 0;
	static constexpr int VISIBLE_ROWS = 8;

	void RebuildItemList();
	void RebuildItemListGrouped();

	// UI�`��
	void DrawStatusPanel(int x, int y, int w, int h);
	void DrawHelpPanel(int x, int y, int w, int h);
	void DrawItemListPanel(int x, int y, int w, int h);
	std::string BuildItemPreviewText(const MenuEntry& e);

	// �t�F�[�h
	bool isFadingIn = false;
	float fadeAlpha = 255.0f;
	const float fadeSpeed = 150.0f;
	void StartFadeIn();
	void UpdateFadeIn(float delta_second);
	void DrawFadeIn();

	template <typename T>
	T clamp(T v, T lo, T hi) {
		if (v < lo)
			return lo;
		if (v > hi)
			return hi;
		return v;
	}

	bool isFirstSpawn = true;
	bool isEncounterEnabled = true;
	float encounterCooldown = 2.0f;
	float encounterCooldownTimer = 0.0f;
	bool isAfterBattle = false;

public:
	Map();
	~Map();

	virtual void Initialize() override;
	virtual eSceneType Update(float delta_second) override;
	virtual void Draw() override;
	virtual void Finalize() override;
	virtual eSceneType GetNowSceneType() const override;

	std::vector<std::vector<char>> LoadStageMapCSV(std::string map_name);
	void DrawStageMap();
	void LoadNextMap();
	bool IsCollision(float x, float y);

	int MapImage;
	Vector2D generate_location;
	class Player* player;
	std::vector<std::vector<char>> mapdata;

private:
	int savedHp{};
	int savedAttack{};
	int savedDefense{};
};

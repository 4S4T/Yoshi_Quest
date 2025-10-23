// --- �d�v�FNOMINMAX ���ŏ��ɁADxLib -> <algorithm> -> <map> �̏� --- //
#define NOMINMAX
#include "DxLib.h"
#include <algorithm>
#include <map>

#include "Map.h"
#include "../Battle/Battle.h"
#include "../Memu/MemuSene.h"
#include "../../Object/Item/Item.h"
#include "../../Utility/InputControl.h"
#include "../SceneManager.h"
#include "../../Object/GameObjectManager.h"
#include "../../Object/NPC/NPC.h"
#include "../../Utility/ResourceManager.h"
#include "../../Utility/PlayerData.h"
#include "../../Utility/Vector2D.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <string>

#define MAP_SQUARE_Y 16
#define MAP_SQUARE_X 22
#define D_OBJECT_SIZE 24.0

static Vector2D old_location;
static int old_stageIndex;
static bool wasInBattle = false;

// �� ���j���[�ʒu���A�i�K�v�ɉ����Ďg�p�j
static Vector2D menu_old_location;
static bool wasInMenu = false;

// ========== �� Rebuild (�݊�) ==========
void Map::RebuildItemList() {
	RebuildItemListGrouped();
	// ������ subMenuSelection / �X�N���[���ʒu��␳
	if (groupedItems.empty()) {
		subMenuSelection = 0;
		listScrollOffset = 0;
	}
	else {
		if (subMenuSelection >= (int)groupedItems.size())
			subMenuSelection = (int)groupedItems.size() - 1;
		if (subMenuSelection < listScrollOffset)
			listScrollOffset = subMenuSelection;
		if (subMenuSelection >= listScrollOffset + VISIBLE_ROWS)
			listScrollOffset = subMenuSelection - (VISIBLE_ROWS - 1);
	}
}

// ========== �V�F�O���[�s���O�č\�z ==========
void Map::RebuildItemListGrouped() {
	groupedItems.clear();

	PlayerData* pd = PlayerData::GetInstance();
	const auto& owned = pd->GetOwnedItems();

	// �܂����O�ł܂Ƃ߂�iConsumable�j
	std::map<std::string, int> nameCount; // ���O -> ��
	for (auto& kv : owned) {
		if (kv.second.GetType() == ItemType::Consumable) {
			nameCount[kv.second.GetName()]++;
		}
	}

	// �\���p�z����\�z
	// 1) �����F1������
	for (auto& kv : owned) {
		const Item& it = kv.second;
		if (it.GetType() == ItemType::Equipment) {
			Map::MenuEntry e{};
			e.representativeId = kv.first;
			e.name = it.GetName();
			e.type = ItemType::Equipment;
			e.category = it.GetCategory();
			e.count = 1;
			int eqId = pd->GetEquippedId(it.GetCategory());
			e.equipped = (eqId == kv.first);
			groupedItems.push_back(e);
		}
	}

	// 2) ����F���O���Ƃ�1�s
	for (auto& pair : nameCount) {
		// ��\ID�͍ŏ��Ɍ�������ID��OK�iUse����1������j
		int repId = -1;
		for (auto& kv : owned) {
			if (kv.second.GetType() == ItemType::Consumable && kv.second.GetName() == pair.first) {
				repId = kv.first;
				break;
			}
		}
		if (repId != -1) {
			const Item& rep = owned.at(repId);
			Map::MenuEntry e{};
			e.representativeId = repId;
			e.name = pair.first;
			e.type = ItemType::Consumable;
			e.category = EquipCategory::None;
			e.count = pair.second;
			e.equipped = false;
			groupedItems.push_back(e);
		}
	}

	// 3) �\�[�g�F����������A�����̓J�e�S���D�恨���O�A����͖��O
	auto rankCat = [](EquipCategory c) -> int {
		switch (c) {
		case EquipCategory::Weapon:
			return 0;
		case EquipCategory::Shield:
			return 1;
		case EquipCategory::Armor:
			return 2;
		case EquipCategory::Helmet:
			return 3;
		default:
			return 9;
		}
	};
	std::sort(groupedItems.begin(), groupedItems.end(),
		[&](const Map::MenuEntry& a, const Map::MenuEntry& b) {
			if (a.type != b.type)
				return a.type == ItemType::Equipment; // �������
			if (a.type == ItemType::Equipment) {
				if (rankCat(a.category) != rankCat(b.category))
					return rankCat(a.category) < rankCat(b.category);
			}
			return a.name < b.name;
		});
}

// �R���X�g���N�^ / �f�X�g���N�^
Map::Map() : encounterStepCounter(0), lastPlayerPos(Vector2D(0, 0)), isFadingIn(false), fadeAlpha(255.0f) {
	srand(static_cast<unsigned int>(time(nullptr)));
}
Map::~Map() {}

// ������
void Map::Initialize() {
	mapdata = LoadStageMapCSV(stageFiles[currentStageIndex]);

	GameManager* obj = Singleton<GameManager>::GetInstance();

	if (isFirstSpawn) {
		generate_location = Vector2D(200.0f, 600.0f);
		isFirstSpawn = false;
	}
	else {
		generate_location = old_location;
	}
	player = obj->CreateGameObject<Player>(generate_location);
	player->SetMapData(mapdata);
	player->SetMapReference(this);

	if (isFirstSpawn) {
		old_location = generate_location;
		currentStageIndex = old_stageIndex;
	}

	encounterStepCounter = 0;
	lastPlayerPos = player->GetLocation();

	items.clear();
	if (isEncounterEnabled) {
		items = GenerateMapItems();
		for (auto& item : items) {
			if (PlayerData::GetInstance()->IsCollected(item->GetId())) {
				item->Collect();
			}
		}
	}

	ncps.clear();
	if (!isEncounterEnabled) {
		std::vector<std::string> lines{
			"�����͈��S�n�т��B", "���̏����͂ł��Ă��邩���H", "Z�L�[�ŉ�b��i�߂����B"
		};
		ncps.push_back(std::make_shared<NCP>(Vector2D(320.0f, 600.0f), "���̒��V", lines));
	}

	StartFadeIn();
}

eSceneType Map::Update(float delta_second) {
	InputControl* input = Singleton<InputControl>::GetInstance();
	GameManager* obj = Singleton<GameManager>::GetInstance();

	if (isFadingIn)
		UpdateFadeIn(delta_second);

	// --- �����Ă���A�C�e���E���i�G���J�E���g�L�����̂݁j ---
	if (isEncounterEnabled) {
		PlayerData* pd = PlayerData::GetInstance();
		for (const auto& it : items) {
			if (!it->IsCollected() && player->GetLocation().DistanceTo(it->GetPosition()) < D_OBJECT_SIZE) {
				it->Collect();
				pd->AddItem(*it);
			}
		}
	}

	// ================== ���j���[���� ==================
	if (isMenuVisible) {
		// �u�ǂ����v���w���Ă��Ė��\�z�Ȃ�ꗗ������
		if (menuSelection == 0 && groupedItems.empty()) {
			RebuildItemList();
			subMenuSelection = 0;
			listScrollOffset = 0;
		}

		// �E���X�g�Ɂg�t�H�[�J�X�h����������i�����ڂ̕`��Ƃ͓Ɨ��j
		bool listActive = (menuSelection == 0 && isSubMenuVisible && !groupedItems.empty());

		// ---------- �����j���[����i�t�H�[�J�X�����j ----------
		if (!listActive) {
			int before = menuSelection;

			if (input->GetKeyDown(KEY_INPUT_DOWN))
				menuSelection = (menuSelection + 1) % menuItemCount;
			if (input->GetKeyDown(KEY_INPUT_UP))
				menuSelection = (menuSelection - 1 + menuItemCount) % menuItemCount;

			if (menuSelection != before) {
				subMenuText.clear();
				// �ǂ����ɗ�����t�H�[�J�XON�A���֓�������OFF
				if (menuSelection == 0) {
					isSubMenuVisible = true;
					if (groupedItems.empty()) {
						RebuildItemList();
						subMenuSelection = 0;
						listScrollOffset = 0;
					}
				}
				else {
					isSubMenuVisible = false;
				}
			}

			// Enter�F�ǂ����ȊO�̌���
			if (input->GetKeyDown(KEY_INPUT_RETURN)) {
				switch (menuSelection) {
				case 1: { // ������
					PlayerData* pd = PlayerData::GetInstance();
					subMenuText = "�����ꗗ";
					subMenuText += "\n ����: " + pd->GetEquippedName(EquipCategory::Weapon);
					subMenuText += "\n ��:   " + pd->GetEquippedName(EquipCategory::Shield);
					subMenuText += "\n �h��: " + pd->GetEquippedName(EquipCategory::Armor);
					subMenuText += "\n ��:   " + pd->GetEquippedName(EquipCategory::Helmet);
					isSubMenuVisible = false;
					groupedItems.clear();
				} break;
				case 2: { // �悳
					PlayerData* pd = PlayerData::GetInstance();
					subMenuText = "�v���C���[���";
					subMenuText += "\n ���x��: " + std::to_string(pd->GetLevel());
					subMenuText += "\n HP: " + std::to_string(pd->GetHp()) + " / " + std::to_string(pd->GetMaxHp());
					subMenuText += "\n �U����: " + std::to_string(pd->GetAttack());
					subMenuText += "\n �h���: " + std::to_string(pd->GetDefense());
					isSubMenuVisible = false;
					groupedItems.clear();
				} break;
				case 3: // ���ǂ�
					isMenuVisible = false;
					isSubMenuVisible = false;
					groupedItems.clear();
					subMenuText.clear();
					return eSceneType::eMap;
				}
			}

			return eSceneType::eMap; // ���j���[�\���p��
		}

		// ---------- �E�F�ǂ������X�g����i�t�H�[�J�X�L��j ----------
		if (input->GetKeyDown(KEY_INPUT_DOWN)) {
			subMenuSelection = (subMenuSelection + 1) % (int)groupedItems.size();
			if (subMenuSelection >= listScrollOffset + VISIBLE_ROWS)
				listScrollOffset = subMenuSelection - (VISIBLE_ROWS - 1);
		}
		if (input->GetKeyDown(KEY_INPUT_UP)) {
			subMenuSelection = (subMenuSelection - 1 + (int)groupedItems.size()) % (int)groupedItems.size();
			if (subMenuSelection < listScrollOffset)
				listScrollOffset = subMenuSelection;
		}
		// �y�[�W����
		if (input->GetKeyDown(KEY_INPUT_A) || input->GetKeyDown(KEY_INPUT_PGUP)) {
			subMenuSelection = std::max(0, subMenuSelection - VISIBLE_ROWS);
			listScrollOffset = std::max(0, listScrollOffset - VISIBLE_ROWS);
		}
		if (input->GetKeyDown(KEY_INPUT_D) || input->GetKeyDown(KEY_INPUT_PGDN)) {
			subMenuSelection = std::min((int)groupedItems.size() - 1, subMenuSelection + VISIBLE_ROWS);
			listScrollOffset = std::min(std::max(0, (int)groupedItems.size() - VISIBLE_ROWS), listScrollOffset + VISIBLE_ROWS);
		}

		// Enter�F���� / �g�p
		if (input->GetKeyDown(KEY_INPUT_RETURN)) {
			const MenuEntry& entry = groupedItems[subMenuSelection];
			const auto& owned = PlayerData::GetInstance()->GetOwnedItems();

			auto it = owned.find(entry.representativeId);
			if (it != owned.end()) {
				const Item& selected = it->second;
				if (entry.type == ItemType::Equipment && selected.GetCategory() != EquipCategory::None) {
					PlayerData::GetInstance()->EquipItem(selected.GetCategory(), entry.representativeId);
					subMenuText = "�������܂����F\n " + selected.GetName();
				}
				else if (entry.type == ItemType::Consumable) {
					const std::string name = selected.GetName();
					const int heal = selected.GetHealAmount();
					if (PlayerData::GetInstance()->UseItem(entry.representativeId)) {
						subMenuText = "�g�p���܂����F\n " + name + "�i+" + std::to_string(heal) + " HP�j";
					}
					else {
						subMenuText = "�g�p�ł��܂���F\n " + name;
					}
				}
				else {
					subMenuText = "���̃A�C�e���͎g�p�ł��܂���F\n " + selected.GetName();
				}
			}

			// ���f�i�������⁚�X�V�j
			RebuildItemList();

			// �����ł͍��֖߂�Ȃ��B��ɂȂ����ꍇ�̂ݖ߂��B
			if (groupedItems.empty()) {
				isSubMenuVisible = false; // �����j���[�փt�H�[�J�X
				subMenuSelection = 0;
				listScrollOffset = 0;
			}
			else {
				// �I���ʒu���\�Ȍ���ێ��i��ʓ��␳�j
				subMenuSelection = std::min(subMenuSelection, (int)groupedItems.size() - 1);
				if (subMenuSelection < listScrollOffset)
					listScrollOffset = subMenuSelection;
				if (subMenuSelection >= listScrollOffset + VISIBLE_ROWS)
					listScrollOffset = std::max(0, subMenuSelection - (VISIBLE_ROWS - 1));
			}
		}

		// Space / Esc�F�C�ӂ̃^�C�~���O�ō����j���[�֖߂�
		if (input->GetKeyDown(KEY_INPUT_SPACE) || input->GetKeyDown(KEY_INPUT_ESCAPE)) {
			isSubMenuVisible = false;
			// subMenuText �͎c��/�����͍D�݂�
			// subMenuText.clear();
		}

		return eSceneType::eMap; // ���j���[�\���p��
	}
	// ================== ���j���[�ȊO ==================

	// NPC ��b
	if (!ncps.empty()) {
		bool someTalking = false;
		for (auto& npc : ncps) {
			if (npc->IsTalking()) {
				someTalking = true;
				break;
			}
		}
		if (someTalking) {
			if (input->GetKeyDown(KEY_INPUT_Z) || input->GetKeyDown(KEY_INPUT_RETURN)) {
				for (auto& npc : ncps)
					if (npc->IsTalking())
						npc->AdvanceTalk();
			}
			return eSceneType::eMap;
		}
		for (auto& npc : ncps) {
			if (npc->IsPlayerInRange(player->GetLocation(), 40.0f)) {
				if (input->GetKeyDown(KEY_INPUT_Z) || input->GetKeyDown(KEY_INPUT_RETURN)) {
					npc->BeginTalk();
					return eSceneType::eMap;
				}
			}
		}
	}

	// �ʏ�X�V
	obj->Update(delta_second);

	// �퓬���A
	if (wasInBattle && GetNowSceneType() == eSceneType::eMap) {
		player->SetLocation(old_location);
		wasInBattle = false;
		encounterStepCounter = 0;
		encounterCooldownTimer = encounterCooldown;
		isAfterBattle = true;
		lastPlayerPos = player->GetLocation();
	}

	// ���j���[���A
	if (wasInMenu && GetNowSceneType() == eSceneType::eMap) {
		player->SetLocation(menu_old_location);
		wasInMenu = false;
	}

	// �G���J�E���g
	Vector2D currentPos = player->GetLocation();
	if (encounterCooldownTimer > 0.0f) {
		encounterCooldownTimer -= delta_second;
		lastPlayerPos = currentPos;
	}
	else if (isEncounterEnabled && ((int)currentPos.x != (int)lastPlayerPos.x || (int)currentPos.y != (int)lastPlayerPos.y)) {
		encounterStepCounter++;
		lastPlayerPos = currentPos;

		if (encounterStepCounter >= 60) {
			encounterStepCounter = 0;
			if (rand() % 100 < 20) {
				old_location = lastPlayerPos;
				old_stageIndex = currentStageIndex;
				wasInBattle = true;
				BattleScene* battleScene = new BattleScene();
				battleScene->SetPlayer(player);
				return eSceneType::eBattle;
			}
		}
	}

	// �}�b�v�J�ڃ|�C���g
	for (const Vector2D& transitionPoint : transitionPoints) {
		if (currentPos.DistanceTo(transitionPoint) < D_OBJECT_SIZE) {
			LoadNextMap();
			break;
		}
	}

	// ���j���[���J��
	if (input->GetKeyDown(KEY_INPUT_TAB)) {
		isMenuVisible = true;
		// ����͍����j���[�Ƀt�H�[�J�X�B�ǂ����Ɉړ�������t�H�[�J�XON�ɂ���B
		isSubMenuVisible = false;
		groupedItems.clear();
		subMenuText.clear();
		return eSceneType::eMap;
	}

	return GetNowSceneType();
}


// �`��
void Map::Draw() {
	DrawStageMap();
	__super::Draw();

	// �����Ă���A�C�e��
	if (isEncounterEnabled) {
		for (const auto& it : items) {
			if (!it->IsCollected()) {
				DrawCircle((int)it->GetPosition().x, (int)it->GetPosition().y, 10, GetColor(255, 215, 0), TRUE);
			}
		}
	}

	// NPC
	for (const auto& npc : ncps)
		npc->Draw();

	if (isFadingIn)
		DrawFadeIn();

	// ===== ���j���[�`�� =====
	if (isMenuVisible) {
		const int rootX = 40;
		const int rootY = 40;
		const int menuW = 520;
		const int menuH = 320;

		// �w�i
		DrawBox(rootX, rootY, rootX + menuW, rootY + menuH, GetColor(20, 20, 28), TRUE);
		DrawBox(rootX, rootY, rootX + menuW, rootY + menuH, GetColor(255, 255, 255), FALSE);

		// �X�e�[�^�X�p�l���i��j
		DrawStatusPanel(rootX + 10, rootY + 10, menuW - 20, 60);

		// ���F���j���[����
		const int leftX = rootX + 10;
		const int leftY = rootY + 80;
		const int leftW = 160;
		const int leftH = menuH - 90;
		DrawBox(leftX, leftY, leftX + leftW, leftY + leftH, GetColor(30, 30, 40), TRUE);
		DrawBox(leftX, leftY, leftX + leftW, leftY + leftH, GetColor(200, 200, 200), FALSE);
		for (int i = 0; i < 4; ++i) {
			int y = leftY + 16 + i * 26;
			if (i == menuSelection)
				DrawString(leftX + 12, y, ("�� " + std::string(menuItems[i])).c_str(), GetColor(255, 255, 0));
			else
				DrawString(leftX + 28, y, menuItems[i], GetColor(235, 235, 235));
		}

		// �E�F�ǂ������X�g or ���
		const int rightX = leftX + leftW + 10;
		const int rightY = leftY;
		const int rightW = menuW - (rightX - rootX) - 10;
		const int rightH = leftH;

		// �E�y�C���F�I�𒆂̃��j���[�ɉ����đ����\���iEnter�s�v�j
		{
			std::string info;

			if (isSubMenuVisible) {
				// �T�u���j���[��
				if (menuSelection == 0 && !groupedItems.empty()) {
					// �ǂ����F�J�[�\���s�̃v���r���[���펞�\��
					int idx = clamp(subMenuSelection, 0, (int)groupedItems.size() - 1);
					info = BuildItemPreviewText(groupedItems[idx]);
				}
				// �u�����сv�u�悳�v�� Enter��̃e�L�X�g(subMenuText)�����ŕ��L
			}
			else {
				// �T�u���j���[���J���Ă��Ȃ��F�����j���[�I�������ŉE�𑦍X�V
				switch (menuSelection) {
				case 0: {
						  // �ǂ����F�e�L�X�g�͏o���Ȃ��i�v���r���[���o���������j
						 // ���X�g�͂��̂��ƕʏ����ŏ펞�`�悳���̂ł����͋��OK
						info.clear();
					
				} break;
				case 1: { // �����сF�����ꗗ�𑦕\��
					PlayerData* pd = PlayerData::GetInstance();
					info = "�����ꗗ\n";
					info += " ����: " + pd->GetEquippedName(EquipCategory::Weapon) + "\n";
					info += " ��:   " + pd->GetEquippedName(EquipCategory::Shield) + "\n";
					info += " �h��: " + pd->GetEquippedName(EquipCategory::Armor) + "\n";
					info += " ��:   " + pd->GetEquippedName(EquipCategory::Helmet);
				} break;
				case 2: { // �悳�F�X�e�[�^�X�𑦕\��
					PlayerData* pd = PlayerData::GetInstance();
					info = "�v���C���[���\n";
					info += " ���x��: " + std::to_string(pd->GetLevel()) + "\n";
					info += " HP: " + std::to_string(pd->GetHp()) + " / " + std::to_string(pd->GetMaxHp()) + "\n";
					info += " �U����: " + std::to_string(pd->GetAttack()) + "\n";
					info += " �h���: " + std::to_string(pd->GetDefense());
				} break;
				case 3: // ���ǂ�
					info = "���j���[����܂��B";
					break;
				}
			}

			// ���O�̑��쌋�ʃ��b�Z�[�W������΁A�v���r���[�̉��ɕ��L
			if (!subMenuText.empty()) {
				if (!info.empty())
					info += "\n----------------\n";
				info += subMenuText;
			}
			if (!info.empty()) {
				std::istringstream iss(info);
				std::string line;
				int lineNum = 0;
				while (std::getline(iss, line)) {
					DrawString(rightX, rightY + lineNum * 20, line.c_str(), GetColor(200, 255, 200));
					lineNum++;
				}
			}
		}

		// �ǂ������X�g�F�J�[�\�����u�ǂ����v���w���Ă���Ώ펞�\���iEnter�s�v�Ō�����j
		if (menuSelection == 0) {
			if (groupedItems.empty()) {
				RebuildItemList(); // �K�v�������\�z
			}
			// �͈͕␳
			if (!groupedItems.empty()) {
				subMenuSelection = clamp(subMenuSelection, 0, (int)groupedItems.size() - 1);
			}
			else {
				subMenuSelection = 0;
			}
			DrawItemListPanel(rightX, rightY + 60, rightW, rightH - 60);
			// �܂�Enter�ŃT�u���j���[�ɓ����Ă��Ȃ����́A����q���g�𔖂��\���i�C�Ӂj
			if (!isSubMenuVisible) {
				DrawString(rightX, rightY + rightH - 18, "Enter�őI�� / �g�p", GetColor(180, 180, 180));
			}
		}


		// ����w���v�i���j
		DrawHelpPanel(rootX + 10, rootY + menuH - 28, menuW - 20, 20);
	}
}

// �w���v
void Map::DrawHelpPanel(int x, int y, int w, int h) {
	DrawBox(x, y, x + w, y + h, GetColor(30, 30, 30), TRUE);
	DrawBox(x, y, x + w, y + h, GetColor(120, 120, 120), FALSE);
	DrawString(x + 8, y + 3, "����:�I��  Enter:����  space: �߂�", GetColor(210, 210, 210));
}

// �X�e�[�^�X
void Map::DrawStatusPanel(int x, int y, int w, int h) {
	DrawBox(x, y, x + w, y + h, GetColor(30, 30, 40), TRUE);
	DrawBox(x, y, x + w, y + h, GetColor(200, 200, 200), FALSE);
	PlayerData* pd = PlayerData::GetInstance();
	std::string txt = "Lv " + std::to_string(pd->GetLevel()) + "   HP " + std::to_string(pd->GetHp()) + "/" + std::to_string(pd->GetMaxHp()) + "   �U " + std::to_string(pd->GetAttack()) + "   �h " + std::to_string(pd->GetDefense());
	DrawString(x + 12, y + 10, txt.c_str(), GetColor(240, 240, 240));
}

// �ǂ������X�g�`��
void Map::DrawItemListPanel(int x, int y, int w, int h) {
	DrawBox(x, y, x + w, y + h, GetColor(26, 26, 32), TRUE);
	DrawBox(x, y, x + w, y + h, GetColor(140, 140, 140), FALSE);

	// ���o��
	DrawString(x + 8, y - 18, "�����i", GetColor(255, 255, 180));

	const int lineH = 20;
	int maxRows = std::min(VISIBLE_ROWS, (int)groupedItems.size());
	for (int i = 0; i < maxRows; ++i) {
		int idx = listScrollOffset + i;
		if (idx < 0 || idx >= (int)groupedItems.size())
			break;
		const Map::MenuEntry& e = groupedItems[idx];

		// �J�e�S�����x��
		std::string label;
		switch (e.category) {
		case EquipCategory::Weapon:
			label = "[��] ";
			break;
		case EquipCategory::Shield:
			label = "[��] ";
			break;
		case EquipCategory::Armor:
			label = "[�Z] ";
			break;
		case EquipCategory::Helmet:
			label = "[��] ";
			break;
		default:
			label = "[��] ";
			break;
		}
		label += e.name;
		if (e.type == ItemType::Consumable && e.count > 1) {
			label += " (x" + std::to_string(e.count) + ")";
		}
		if (e.equipped)
			label = "�� " + label;

		// �J�[�\��
		if (idx == subMenuSelection) {
			DrawString(x + 8, y + 6 + i * lineH, "��", GetColor(255, 255, 0));
		}
		DrawString(x + 28, y + 6 + i * lineH, label.c_str(), GetColor(220, 255, 220));
	}

	// �X�N���[���o�[�ڈ�
	if ((int)groupedItems.size() > VISIBLE_ROWS) {
		float ratio = (float)VISIBLE_ROWS / (float)groupedItems.size();
		int barH = std::max(8, (int)(h * ratio));
		int track = h - barH;
		float posRatio = (float)listScrollOffset / std::max(1, (int)groupedItems.size() - VISIBLE_ROWS);
		int barY = y + (int)(posRatio * track);
		int barX = x + w - 8;
		DrawBox(barX, y, barX + 4, y + h, GetColor(60, 60, 60), TRUE);
		DrawBox(barX, barY, barX + 4, barY + barH, GetColor(200, 200, 200), TRUE);
	}
}

// �� �J�[�\���ʒu�̃A�C�e�����i�v���r���[�펞�\���j
std::string Map::BuildItemPreviewText(const Map::MenuEntry& e) {
	// ��\ID������̂������i���݂��Ȃ���ΊȈՏ��j
	const auto& owned = PlayerData::GetInstance()->GetOwnedItems();
	auto it = owned.find(e.representativeId);

	// ���ʃ��x��
	auto catLabel = [](EquipCategory c) -> const char* {
		switch (c) {
		case EquipCategory::Weapon:
			return "����";
		case EquipCategory::Shield:
			return "��";
		case EquipCategory::Armor:
			return "�Z";
		case EquipCategory::Helmet:
			return "��";
		default:
			return "����";
		}
	};

	std::string s;
	return s;
}

// �I��
void Map::Finalize() {
	GameManager* obj = Singleton<GameManager>::GetInstance();
	obj->Finalize();
}

// ���݂̃V�[���^�C�v
eSceneType Map::GetNowSceneType() const { return eSceneType::eMap; }

// CSV �ǂݍ��݁i���R���W����/�J�ڂ͊������W�b�N�j
std::vector<std::vector<char>> Map::LoadStageMapCSV(std::string map_name) {
	std::ifstream ifs(map_name);
	if (ifs.fail())
		throw std::runtime_error(map_name + " ���J���܂���ł����B");

	std::vector<std::vector<char>> data;
	collisionMap.clear();
	transitionPoints.clear();

	std::string line;
	int rowIdx = 0;
	isEncounterEnabled = (map_name != "Resource/stage2.csv");

	while (std::getline(ifs, line)) {
		std::vector<char> row;
		std::vector<bool> collisionRow;
		std::stringstream ss(line);
		std::string cell;
		int colIdx = 0;
		while (std::getline(ss, cell, ',')) {
			char c = cell[0];
			row.push_back(c);
			// '3' �� '6' ���Փ˂Ƃ݂Ȃ��i�����W�b�N�ɍ��킹��j
			collisionRow.push_back(c == '3');
			collisionRow.push_back(c == '6');
			if (c == '4') {
				float x = D_OBJECT_SIZE + ((D_OBJECT_SIZE * 2) * colIdx);
				float y = D_OBJECT_SIZE + ((D_OBJECT_SIZE * 2) * rowIdx);
				transitionPoints.push_back(Vector2D(x, y));
			}
			colIdx++;
		}
		data.push_back(row);
		collisionMap.push_back(collisionRow);
		rowIdx++;
	}
	return data;
}

// �}�b�v�`��
void Map::DrawStageMap() {
	ResourceManager* rm = ResourceManager::GetInstance();
	for (int i = 0; i < MAP_SQUARE_Y; i++) {
		for (int j = 0; j < MAP_SQUARE_X; j++) {
			char c = mapdata[i][j];
			std::string path = "Resource/Images/Block/" + std::to_string(c - '0') + ".png";
			MapImage = rm->GetImages(path, 1, 1, 1, 16, 16)[0];
			DrawRotaGraphF(
				D_OBJECT_SIZE + ((D_OBJECT_SIZE * 2) * j),
				D_OBJECT_SIZE + ((D_OBJECT_SIZE * 2) * i),
				1.9, 0.0, MapImage, TRUE);
		}
	}
}

// ���}�b�v
void Map::LoadNextMap() {
	currentStageIndex++;
	if (currentStageIndex >= stageFiles.size())
		currentStageIndex = 0;

	transitionPoints.clear();

	mapdata = LoadStageMapCSV(stageFiles[currentStageIndex]);
	player->SetMapData(mapdata);
	player->SetLocation(Vector2D(200.0f, 600.0f));

	// NCP
	ncps.clear();
	if (!isEncounterEnabled) {
		std::vector<std::string> lines{ "�����͈��S�n�т��B", "���̏����͂ł��Ă��邩���H" };
		ncps.push_back(std::make_shared<NCP>(Vector2D(320.0f, 600.0f), "���̒��V", lines));
		ncps.back()->SetImage("Resource/Images/peabird.png");
	}
	if (!isEncounterEnabled) {
		std::vector<std::string> lines{ "�l������[�I�I" };
		ncps.push_back(std::make_shared<NCP>(Vector2D(220.0f, 300.0f), "���lA", lines));
		ncps.back()->SetImage("Resource/Images/yossi_ikiri.png");
	}
}

// �t�F�[�h
void Map::StartFadeIn() {
	isFadingIn = true;
	fadeAlpha = 255.0f;
}
void Map::UpdateFadeIn(float delta_second) {
	fadeAlpha -= fadeSpeed * delta_second;
	fadeAlpha = clamp(fadeAlpha, 0.0f, 255.0f);
	if (fadeAlpha <= 0.0f)
		isFadingIn = false;
}
void Map::DrawFadeIn() {
	if (isFadingIn) {
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(fadeAlpha));
		DrawBox(0, 0, 960, 720, GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

// �Փ˔���
bool Map::IsCollision(float x, float y) {
	int col = static_cast<int>(x / (D_OBJECT_SIZE * 2));
	int row = static_cast<int>(y / (D_OBJECT_SIZE * 2));
	return row < 0 || row >= (int)collisionMap.size() || col < 0 || col >= (int)collisionMap[row].size() || collisionMap[row][col];
}

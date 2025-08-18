#include "Map.h"
#include "../Battle/Battle.h"
#include "../Memu/MemuSene.h"
#include "../../Object/Item/Item.h"
#include "../../Utility/InputControl.h"
#include "DxLib.h"
#include "../SceneManager.h"
#include "../../Object/GameObjectManager.h"
#include "../../Utility/ResourceManager.h"
#include "../../Utility/PlayerData.h"
#include "../../Utility/Vector2D.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <string>


#define MAP_SQUARE_Y 16	   // �}�b�v�̏c�̃}�X��
#define MAP_SQUARE_X 22	   // �}�b�v�̉��̃}�X��
#define D_OBJECT_SIZE 24.0 // �I�u�W�F�N�g�̊�{�T�C�Y

static Vector2D old_location; // �퓬�J�n���̃v���C���[�ʒu
static int old_stageIndex;	  // �퓬�O�̃X�e�[�W�C���f�b�N�X

static bool wasInBattle = false; // �퓬���A�t���O

// �� ���j���[�p�ǉ��ϐ�
static Vector2D menu_old_location; // ���j���[�J�n�O�̈ʒu�ۑ�
static bool wasInMenu = false;	   // ���j���[���A�t���O

// �R���X�g���N�^
Map::Map() : encounterStepCounter(0), lastPlayerPos(Vector2D(0, 0)), isFadingIn(false), fadeAlpha(255.0f) {
	srand(static_cast<unsigned int>(time(nullptr))); // �����V�[�h
}

// �f�X�g���N�^
Map::~Map() {}

// ����������
void Map::Initialize() {
	// �}�b�v�f�[�^�����[�h
	mapdata = LoadStageMapCSV(stageFiles[currentStageIndex]);

	GameManager* obj = Singleton<GameManager>::GetInstance();

	// **�v���C���[�̃X�|�[���ʒu����**
	if (isFirstSpawn) {
		generate_location = Vector2D(200.0f, 600.0f);
		isFirstSpawn = false;
	}
	else {
		generate_location = old_location; // �퓬�E���j���[��� old_location �ɕ��A
	}

	player = obj->CreateGameObject<Player>(generate_location);

	// �v���C���[�̃}�b�v�f�[�^�ݒ�
	player->SetMapData(mapdata);
	player->SetMapReference(this);

	// old_location �����l
	if (isFirstSpawn) {
		old_location = generate_location;
		currentStageIndex = old_stageIndex;
	}

	encounterStepCounter = 0;
	lastPlayerPos = player->GetLocation();

	items = GenerateMapItems();


	for (auto& item : items) {
		if (PlayerData::GetInstance()->IsCollected(item->GetId())) {
			item->Collect();
		}
	}



	StartFadeIn();
}


// �X�V����
eSceneType Map::Update(float delta_second) {
	InputControl* input = Singleton<InputControl>::GetInstance();
	GameManager* obj = Singleton<GameManager>::GetInstance();

	/*obj->Update(delta_second);*/

	// �t�F�[�h�C���X�V
	if (isFadingIn)
		UpdateFadeIn(delta_second);

	 // ===== �A�C�e���E�� =====
	PlayerData* pd = PlayerData::GetInstance();
	for (const auto& it : items) {
		if (!it->IsCollected() &&
			player->GetLocation().DistanceTo(it->GetPosition()) < D_OBJECT_SIZE) {
			it->Collect();
			pd->AddItem(*it); // �A�C�e���{�b�N�X��
		}
	}

	// ===== ���j���[ =====
	if (isMenuVisible) {
		// �g�b�v���j���[�̃J�[�\���̓T�u���j���[���쒆�͓������Ȃ�
		if (!(isSubMenuVisible && isItemListActive)) {
			if (input->GetKeyDown(KEY_INPUT_DOWN)) {
				menuSelection = (menuSelection + 1) % menuItemCount;
			}
			if (input->GetKeyDown(KEY_INPUT_UP)) {
				menuSelection = (menuSelection - 1 + menuItemCount) % menuItemCount;
			}
			// �u���ǂ�v���w���Ă���Ԃ̓T�u���j���[�𑦃N���A�i�����`�悵�Ȃ��j
			if (menuSelection == 3) { // ���ǂ�
				isSubMenuVisible = false;
				isItemListActive = false;
				subMenuText.clear();
				subMenuItemIds.clear();
			}
		}


		// �T�u���j���[�i�ǂ����ꗗ�j�̑���
		if (isSubMenuVisible && isItemListActive && !subMenuItemIds.empty()) {
			if (input->GetKeyDown(KEY_INPUT_DOWN)) {
				subMenuSelection = (subMenuSelection + 1) % static_cast<int>(subMenuItemIds.size());
			}
			if (input->GetKeyDown(KEY_INPUT_UP)) {
				subMenuSelection = (subMenuSelection - 1 + static_cast<int>(subMenuItemIds.size())) % static_cast<int>(subMenuItemIds.size());
			}
			if (input->GetKeyDown(KEY_INPUT_RETURN)) {
				int id = subMenuItemIds[subMenuSelection];
				const auto& owned = PlayerData::GetInstance()->GetOwnedItems();
				auto it = owned.find(id);
				if (it != owned.end()) {
					const Item& selected = it->second;
					if (selected.GetType() == ItemType::Equipment && selected.GetCategory() != EquipCategory::None) {
						PlayerData::GetInstance()->EquipItem(selected.GetCategory(), id);
						subMenuText = "�������܂����F\n " + selected.GetName();
					}
					else {
						subMenuText = "���̃A�C�e���͑����ł��܂���F\n " + selected.GetName();
					}
				}
			}
			// Esc / Backspace �ňꗗ�����
			if (input->GetKeyDown(KEY_INPUT_ESCAPE) || input->GetKeyDown(KEY_INPUT_SPACE)) {
				isItemListActive = false;
				isSubMenuVisible = false;
				subMenuItemIds.clear();
				subMenuText.clear();
			}
		}

		// �g�b�v���j���[��Enter�̓T�u���j���[���쒆�͖���
		if (!(isSubMenuVisible && isItemListActive)) {
			if (input->GetKeyDown(KEY_INPUT_RETURN)) {
				switch (menuSelection) {
				case 0: {						// �ǂ���
					subMenuText = "   ";
					subMenuItemIds.clear();
					const auto& owned = PlayerData::GetInstance()->GetOwnedItems();
					for (const auto& kv : owned)
						subMenuItemIds.push_back(kv.first);
					if (subMenuItemIds.empty()) {
						subMenuText += "\n �Ȃ�";
						isItemListActive = false;
					}
					else {
						subMenuSelection = 0;
						isItemListActive = true;
					}
					isSubMenuVisible = true;
				} break;

				case 1: { // ������
					PlayerData* pd = PlayerData::GetInstance();
					subMenuText = "�����ꗗ";
					subMenuText += "\n ����: " + pd->GetEquippedName(EquipCategory::Weapon);
					subMenuText += "\n ��:   " + pd->GetEquippedName(EquipCategory::Shield);
					subMenuText += "\n �h��: " + pd->GetEquippedName(EquipCategory::Armor);
					subMenuText += "\n ��: " + pd->GetEquippedName(EquipCategory::Helmet);
					isItemListActive = false;
					subMenuItemIds.clear();
					isSubMenuVisible = true;
				} break;

				case 2: { // �悳
					PlayerData* pd = PlayerData::GetInstance();
					subMenuText = "�v���C���[���";
					subMenuText += "\n ���x��: " + std::to_string(pd->GetLevel());
					subMenuText += "\n HP: " + std::to_string(pd->GetHp()) + " / " + std::to_string(pd->GetMaxHp());
					subMenuText += "\n �U����: " + std::to_string(pd->GetAttack());
					subMenuText += "\n �h���: " + std::to_string(pd->GetDefense());
					isItemListActive = false;
					subMenuItemIds.clear();
					isSubMenuVisible = true;
				} break;

				case 3: // ���ǂ�
					isMenuVisible = false;
					isSubMenuVisible = false;
					isItemListActive = false;
					subMenuItemIds.clear();
					subMenuText.clear();
					break;
				}
			}
		}


		// ���j���[���̓v���C���[�E�o�g�����������X�L�b�v
		return eSceneType::eMap;
	}

	// �ʏ�X�V
	obj->Update(delta_second);

	// **�퓬���A����**
	if (wasInBattle && GetNowSceneType() == eSceneType::eMap) {
		player->SetLocation(old_location);
		wasInBattle = false;
	}

	// **���j���[���A�����i�ǉ������j**
	if (wasInMenu && GetNowSceneType() == eSceneType::eMap) {
		player->SetLocation(menu_old_location);
		wasInMenu = false;
	}

	// **�G���J�E���g����**
	Vector2D currentPos = player->GetLocation();
	if (isEncounterEnabled && ((int)currentPos.x != (int)lastPlayerPos.x || (int)currentPos.y != (int)lastPlayerPos.y)) {
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

	// **�}�b�v�J�ڃ|�C���g�m�F**
	for (const Vector2D& transitionPoint : transitionPoints) {
		if (currentPos.DistanceTo(transitionPoint) < D_OBJECT_SIZE) {
			LoadNextMap();
			break;
		}
	}

	// ���j���[���J��
	if (input->GetKeyDown(KEY_INPUT_TAB)) {
		isMenuVisible = true;
		isSubMenuVisible = false;
		isItemListActive = false;
		subMenuItemIds.clear();
		subMenuText.clear();
		return eSceneType::eMap;
	}

	return GetNowSceneType();
}

void Map::Draw() {
	DrawStageMap();
	__super::Draw();

	 // ���擾�A�C�e����`��
	for (const auto& it : items) {
		if (!it->IsCollected()) {
			DrawCircle(
				static_cast<int>(it->GetPosition().x),
				static_cast<int>(it->GetPosition().y),
				10,
				GetColor(255, 215, 0),
				TRUE);
		}
	}

	if (isFadingIn)
		DrawFadeIn();

	// ���j���[�`��
	if (isMenuVisible) {
		const int menuX = 520;
		const int menuY = 100;
		const int menuWidth = 380;
		const int menuHeight = 260;

		// �w�i
		DrawBox(menuX, menuY, menuX + menuWidth, menuY + menuHeight, GetColor(30, 30, 30), TRUE);
		DrawBox(menuX, menuY, menuX + menuWidth, menuY + menuHeight, GetColor(255, 255, 255), FALSE);

		// ����F���j���[����
		for (int i = 0; i < 4; ++i) {
			int y = menuY + 30 + i * 30;
			if (i == menuSelection)
				DrawString(menuX + 20, y, ("�� " + std::string(menuItems[i])).c_str(), GetColor(255, 255, 0));
			else
				DrawString(menuX + 40, y, menuItems[i], GetColor(255, 255, 255));
		}

		// �T�u���j���[���̕`��i�u�ǂ����v�ꗗ���͍����^����ȊO�͉E��B���u���ǂ�v�I�𒆂͕`���Ȃ��j
		if (isSubMenuVisible && menuSelection != 3 && !subMenuText.empty()) {
			std::istringstream iss(subMenuText);
			std::string line;
			int lineNum = 0;

			// �ǂ����ꗗ�̂Ƃ��͉E�����X�g�Ɣ��̂ō����ցA����ȊO�͉E���
			const bool showBottomLeft = (menuSelection == 0 && isItemListActive);
			const int textX = showBottomLeft ? (menuX + 20) : (menuX + 200);
			const int textY = showBottomLeft ? (menuY + 150) : (menuY + 20);

			while (std::getline(iss, line)) {
				DrawString(textX, textY + lineNum * 20, line.c_str(), GetColor(200, 255, 200));
				lineNum++;
			}
		}


		// �E��F�u�ǂ����v�ꗗ�i�J�[�\���{�����}�[�N�j���u�ǂ����v�I�����̂�
		if (isSubMenuVisible && menuSelection == 0 && isItemListActive && !subMenuItemIds.empty()) {
			const int listX = menuX + 200; // �E��
			const int listY = menuY + 20;
			const int lineH = 20;

			PlayerData* pd = PlayerData::GetInstance();
			const auto& owned = pd->GetOwnedItems();

			for (int i = 0; i < static_cast<int>(subMenuItemIds.size()); ++i) {
				int id = subMenuItemIds[i];
				auto it = owned.find(id);
				if (it == owned.end())
					continue;

				const Item& obj = it->second;

				// �J�e�S���L��
				std::string label;
				switch (obj.GetCategory()) {
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
					label = "     ";
					break;
				}
				label += obj.GetName();

				// �������Ȃ灚
				int eqId = pd->GetEquippedId(obj.GetCategory());
				if (eqId == id && obj.GetCategory() != EquipCategory::None) {
					label = "�� " + label;
				}

				// �J�[�\��
				if (i == subMenuSelection) {
					DrawString(listX - 20, listY + i * lineH, "��", GetColor(255, 255, 0));
				}

				// ���x��
				DrawString(listX, listY + i * lineH, label.c_str(), GetColor(220, 255, 220));
			}
		}
	}
}


// �I������
void Map::Finalize() {
	GameManager* obj = Singleton<GameManager>::GetInstance();
	obj->Finalize();
}


// ���݂̃V�[���^�C�v
eSceneType Map::GetNowSceneType() const {
	return eSceneType::eMap;
}

// CSV�ǂݍ���
std::vector<std::vector<char>> Map::LoadStageMapCSV(std::string map_name) {
	std::ifstream ifs(map_name);
	if (ifs.fail()) {
		throw std::runtime_error(map_name + " ���J���܂���ł����B");
	}

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
			collisionRow.push_back(c == '3');
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

			DrawRotaGraphF(D_OBJECT_SIZE + ((D_OBJECT_SIZE * 2) * j),
				D_OBJECT_SIZE + ((D_OBJECT_SIZE * 2) * i),
				1.9, 0.0, MapImage, TRUE);
		}
	}
}

// ���̃}�b�v
void Map::LoadNextMap() {
	currentStageIndex++;
	if (currentStageIndex >= stageFiles.size()) {
		currentStageIndex = 0;
	}
	transitionPoints.clear();
	mapdata = LoadStageMapCSV(stageFiles[currentStageIndex]);
	player->SetMapData(mapdata);
	player->SetLocation(Vector2D(200.0f, 600.0f));
}

// �t�F�[�h�C��
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
	return row < 0 || row >= collisionMap.size() || col < 0 || col >= collisionMap[row].size() || collisionMap[row][col];
}
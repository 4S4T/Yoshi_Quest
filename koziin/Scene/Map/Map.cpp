#include "Map.h"
#include "../Battle/Battle.h"
#include "../Memu/MemuSene.h"
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

	StartFadeIn();
}

// �X�V����
eSceneType Map::Update(float delta_second) {
	InputControl* input = Singleton<InputControl>::GetInstance();
	GameManager* obj = Singleton<GameManager>::GetInstance();

	obj->Update(delta_second);

	// �t�F�[�h�C���X�V
	if (isFadingIn)
		UpdateFadeIn(delta_second);

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

	// **���j���[�Ăяo�������i�ǉ������j**
	if (input->GetKeyDown(KEY_INPUT_TAB)) {
		menu_old_location = player->GetLocation(); // ���݈ʒu��ۑ�
		wasInMenu = true;						   // ���A�t���OON
		return eSceneType::eMemu;				   // ���j���[�֑J��
	}

	// **�}�b�v�J�ڃ|�C���g�m�F**
	for (const Vector2D& transitionPoint : transitionPoints) {
		if (currentPos.DistanceTo(transitionPoint) < D_OBJECT_SIZE) {
			LoadNextMap();
			break;
		}
	}

	return GetNowSceneType();
}

// �`�揈��
void Map::Draw() {
	DrawStageMap();
	__super::Draw();

	if (isFadingIn)
		DrawFadeIn();

	PlayerData* pd = PlayerData::GetInstance();
	int PlayerHp = pd->GetHp();
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

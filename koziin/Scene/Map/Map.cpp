#include "Map.h"
#include"../Battle/Battle.h"
#include "../../Utility/InputControl.h"
#include "DxLib.h"
#include "../SceneManager.h"
#include "../../Object/GameObjectManager.h"
#include "../../Utility/ResourceManager.h"
#include"../../Utility/PlayerData.h"
#include"../../Utility/Vector2D.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>

#define MAP_SQUARE_Y 16	   // �}�b�v�̏c�̃}�X��
#define MAP_SQUARE_X 22	   // �}�b�v�̉��̃}�X��
#define D_OBJECT_SIZE 24.0 // �I�u�W�F�N�g�̊�{�T�C�Y

static Vector2D old_location; // �퓬�J�n���̃v���C���[�ʒu�̕ۑ��p
static int old_stageIndex;	  // �퓬�O�̃X�e�[�W�C���f�b�N�X���L�^




// �R���X�g���N�^
Map::Map() : encounterStepCounter(0), lastPlayerPos(Vector2D(0, 0)), isFadingIn(false), fadeAlpha(255.0f) {
	srand(static_cast<unsigned int>(time(nullptr))); // �����̃V�[�h��ݒ�
}

// �f�X�g���N�^
Map::~Map() {}

// ����������
void Map::Initialize() {
	// �}�b�v�f�[�^�����[�h
	mapdata = LoadStageMapCSV(stageFiles[currentStageIndex]);

	// �Q�[���}�l�[�W���[�̃C���X�^���X�擾
	GameManager* obj = Singleton<GameManager>::GetInstance();


	// **�v���C���[�̃X�|�[���ʒu������**
	if (isFirstSpawn)
	{
		generate_location = Vector2D(200.0f, 600.0f);
		isFirstSpawn = false; // ����X�|�[�����I������̂Ńt���O���I�t
	}
	else {
		generate_location = old_location; // �퓬��� old_location �ɕ��A
	}

	player = obj->CreateGameObject<Player>(generate_location);

	// **�v���C���[�̃}�b�v�f�[�^��ݒ�**
	player->SetMapData(mapdata);
	player->SetMapReference(this);

	// **old_location �̏����l��ݒ�**
	if (isFirstSpawn) {
		old_location = generate_location; // ����� (200,600)
		currentStageIndex = old_stageIndex; // **�퓬�O�̃}�b�v�ɖ߂�**
	}

	encounterStepCounter = 0;
	lastPlayerPos = player->GetLocation();

	StartFadeIn(); // �t�F�[�h�C���J�n
}


// �X�V����
// �퓬���A�����̃t���O�i�퓬�J�n���ɐݒ肷��j
static bool wasInBattle = false;

// �X�V����
eSceneType Map::Update(float delta_second) {
	InputControl* input = Singleton<InputControl>::GetInstance();
	GameManager* obj = Singleton<GameManager>::GetInstance();

	// �Q�[���I�u�W�F�N�g�̍X�V
	obj->Update(delta_second);

	// �t�F�[�h�C�������̍X�V
	if (isFadingIn)
		UpdateFadeIn(delta_second);

	// **�퓬���A����**
	if (wasInBattle && GetNowSceneType() == eSceneType::eMap) {
		player->SetLocation(old_location); // �퓬�O�̈ʒu�ɖ߂�
		wasInBattle = false;			   // �t���O�����Z�b�g
	}

	// **�G���J�E���g����**
	Vector2D currentPos = player->GetLocation();
	if (isEncounterEnabled && ((int)currentPos.x != (int)lastPlayerPos.x || (int)currentPos.y != (int)lastPlayerPos.y)) {
		encounterStepCounter++;
		lastPlayerPos = currentPos;

		if (encounterStepCounter >= 60) {
			encounterStepCounter = 0;
			if (rand() % 100 < 20) {
				old_location = lastPlayerPos;		// �퓬�O�̃v���C���[�ʒu��ۑ�
				old_stageIndex = currentStageIndex; // �퓬�O�̃}�b�v��ۑ�

				wasInBattle = true; // �퓬�t���O�𗧂Ă�

				// **�o�g���V�[�����쐬**
				BattleScene* battleScene = new BattleScene();

				// **�v���C���[���o�g���V�[���ɓn��**
				battleScene->SetPlayer(player);

				return eSceneType::eBattle;
			}
		}
	}

	// **�}�b�v�J�ڃ|�C���g�̊m�F**
	for (const Vector2D& transitionPoint : transitionPoints) {
		if (currentPos.DistanceTo(transitionPoint) < D_OBJECT_SIZE) { // �v���C���[�ƑJ�ڃ|�C���g�̋������߂��ꍇ
			LoadNextMap();											  // ���̃}�b�v�����[�h
			break;
		}
	}

	return GetNowSceneType();
}





// �`�揈��
void Map::Draw() {
	DrawStageMap();	 // �}�b�v�̕`��
	__super::Draw(); // �e�N���X�̕`�揈��
	if (isFadingIn)
		DrawFadeIn(); // �t�F�[�h�C���`��
	PlayerData* pd = PlayerData::GetInstance();
	int PlayerHp = pd->GetHp();
}

// �I������
void Map::Finalize() {
	GameManager* obj = Singleton<GameManager>::GetInstance();
	obj->Finalize();
}

// ���݂̃V�[���^�C�v���擾
eSceneType Map::GetNowSceneType() const {
	return eSceneType::eMap;
}

// �}�b�v�f�[�^��CSV���烍�[�h
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

	// **�G���J�E���g�ۂ̐ݒ�**
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
			collisionRow.push_back(c == '3'); // '3'�͏Փ˔���
			if (c == '4') {					  // '4'�̓}�b�v�J�ڃ|�C���g
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


// �}�b�v�̕`��
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

// ���̃}�b�v�����[�h
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

// �t�F�[�h�C���̊J�n
void Map::StartFadeIn() {
	isFadingIn = true;
	fadeAlpha = 255.0f;
}

// �t�F�[�h�C���̍X�V
void Map::UpdateFadeIn(float delta_second) {
	fadeAlpha -= fadeSpeed * delta_second;
	fadeAlpha = clamp(fadeAlpha, 0.0f, 255.0f);
	if (fadeAlpha <= 0.0f)
		isFadingIn = false;
}

// �t�F�[�h�C���̕`��
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

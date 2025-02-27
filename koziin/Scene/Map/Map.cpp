#include "Map.h"
#include "../../Utility/InputControl.h"
#include "DxLib.h"
#include "../SceneManager.h"
#include "../../Object/GameObjectManager.h"
#include "../../Utility/ResourceManager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>

#define MAP_SQUARE_Y 16
#define MAP_SQUARE_X 22
#define D_OBJECT_SIZE 24.0

Map::Map() : encounterStepCounter(0), lastPlayerPos(Vector2D(0, 0)), isFadingIn(false), fadeAlpha(255.0f) {
	srand(static_cast<unsigned int>(time(nullptr)));
}

Map::~Map() {}

void Map::Initialize() {
	mapdata = LoadStageMapCSV(stageFiles[currentStageIndex]);
	GameManager* obj = Singleton<GameManager>::GetInstance();
	generate_location = player ? player->GetLocation() : Vector2D(200.0f, 600.0f);
	player = obj->CreateGameObject<Player>(generate_location);
	player->SetMapData(mapdata);
	player->SetMapReference(this);
	encounterStepCounter = 0;
	lastPlayerPos = player->GetLocation();
	StartFadeIn();
}

eSceneType Map::Update(float delta_second) {
	InputControl* input = Singleton<InputControl>::GetInstance();
	GameManager* obj = Singleton<GameManager>::GetInstance();
	obj->Update(delta_second);

	if (isFadingIn)
		UpdateFadeIn(delta_second);

	Vector2D currentPos = player->GetLocation();
	if ((int)currentPos.x != (int)lastPlayerPos.x || (int)currentPos.y != (int)lastPlayerPos.y) {
		encounterStepCounter++;
		lastPlayerPos = currentPos;
		if (encounterStepCounter >= 5) {
			encounterStepCounter = 0;
			if (rand() % 100 < 1) {
				return eSceneType::eBattle;
			}
		}
	}

	for (const auto& point : transitionPoints) {
		float distance = sqrt(pow(currentPos.x - point.x, 2) + pow(currentPos.y - point.y, 2));
		if (distance <= 20.0f) {
			LoadNextMap();
			StartFadeIn();
			break;
		}
	}

	if (input->GetKeyDown(KEY_INPUT_P)) {
		return eSceneType::eTitle;
	}

	return GetNowSceneType();
}

void Map::Draw() {
	DrawStageMap();
	__super::Draw();
	if (isFadingIn)
		DrawFadeIn();
}

void Map::Finalize() {
	GameManager* obj = Singleton<GameManager>::GetInstance();
	obj->Finalize();
}

eSceneType Map::GetNowSceneType() const {
	return eSceneType::eMap;
}

std::vector<std::vector<char>> Map::LoadStageMapCSV(std::string map_name) {
	std::ifstream ifs(map_name);
	if (ifs.fail()) {
		throw std::runtime_error(map_name + " Ç™äJÇØÇ‹ÇπÇÒÇ≈ÇµÇΩÅB");
	}

	std::vector<std::vector<char>> data;
	collisionMap.clear();
	transitionPoints.clear();
	std::string line;
	int rowIdx = 0;

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
		DrawBox(0, 0, static_cast<int>(960), static_cast<int>(720), GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

bool Map::IsCollision(float x, float y) {
	if (collisionMap.empty())
		return true;
	int col = static_cast<int>(x / (D_OBJECT_SIZE * 2));
	int row = static_cast<int>(y / (D_OBJECT_SIZE * 2));
	if (row < 0 || row >= static_cast<int>(collisionMap.size()) ||
		col < 0 || col >= static_cast<int>(collisionMap[row].size())) {
		return true;
	}
	return collisionMap[row][col];
}

void Map::DrawCollisionMap() {
	for (int row = 0; row < collisionMap.size(); ++row) {
		for (int col = 0; col < collisionMap[row].size(); ++col) {
			if (collisionMap[row][col]) {
				DrawBox(col * (D_OBJECT_SIZE * 2), row * (D_OBJECT_SIZE * 2),
					(col + 1) * (D_OBJECT_SIZE * 2), (row + 1) * (D_OBJECT_SIZE * 2),
					GetColor(255, 0, 0), TRUE);
			}
		}
	}
}
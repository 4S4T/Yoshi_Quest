#pragma once

#include "../SceneBase.h"
#include "../../Utility/Vector2D.h"
#include "../../Object/Charactor/Player/Player.h"
#include <string>
#include <vector>
#include <random>

class Map : public SceneBase {
private:
	std::vector<Vector2D> transitionPoints;
	std::vector<std::vector<bool>> collisionMap;
	std::vector<std::string> stageFiles = {
		"Resource/stage1.csv",
		"Resource/stage2.csv",
		"Resource/stage3.csv"
	};
	int currentStageIndex = 0;

	Vector2D cameraOffset;
	const float SCREEN_WIDTH = 800.0f;
	const float SCREEN_HEIGHT = 600.0f;

	int encounterStepCounter = 0;
	Vector2D lastPlayerPos;
	const int encounterStepThreshold = 5;
	std::mt19937 randomEngine;
	std::uniform_int_distribution<int> distribution;
	bool hasInitialFadeOut = false;
	bool initialFadeOutCompleted = false;

	// フェードイン・フェードアウト用変数
	bool isFadingIn = false;
	bool isFadingOut = false;
	bool hasTransitioned = false;
	eSceneType nextScene = eSceneType::eMap;
	float fadeAlpha = 255.0f;
	const float fadeSpeed = 150.0f;

	bool isFirstSpawn = true; // 初回スポーン判定
	bool isEncounterEnabled = true;


	void UpdateCamera();
	void CheckEncounter();
	void StartFadeIn();
	void UpdateFadeIn(float delta_second);
	void DrawFadeIn();
	void StartFadeOut(eSceneType targetScene);
	void UpdateFadeOut(float delta_second);
	void DrawFadeOut();

	template <typename T>
	T clamp(T value, T min, T max) {
		if (value < min)
			return min;
		if (value > max)
			return max;
		return value;
	}

public:
	Map();
	~Map();

	virtual void Initialize() override;
	int MapImage;
	Vector2D generate_location;
	class Player* player;

	virtual eSceneType Update(float delta_second) override;
	virtual void Draw() override;
	virtual void Finalize() override;
	virtual eSceneType GetNowSceneType() const override;

	std::vector<std::vector<char>> LoadStageMapCSV(std::string map_name);
	void DrawStageMap();
	void LoadNextMap();
	bool IsCollision(float x, float y);
	void DrawCollisionMap();
	std::vector<std::vector<char>> mapdata;
};

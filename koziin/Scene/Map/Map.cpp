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

#define MAP_SQUARE_Y 16	   // マップの縦のマス数
#define MAP_SQUARE_X 22	   // マップの横のマス数
#define D_OBJECT_SIZE 24.0 // オブジェクトの基本サイズ

static Vector2D old_location; // 戦闘開始時のプレイヤー位置の保存用

// コンストラクタ
Map::Map() : encounterStepCounter(0), lastPlayerPos(Vector2D(0, 0)), isFadingIn(false), fadeAlpha(255.0f) {
	srand(static_cast<unsigned int>(time(nullptr))); // 乱数のシードを設定
}

// デストラクタ
Map::~Map() {}

// 初期化処理
void Map::Initialize() {
	// マップデータをロード
	mapdata = LoadStageMapCSV(stageFiles[currentStageIndex]);

	// ゲームマネージャーのインスタンス取得
	GameManager* obj = Singleton<GameManager>::GetInstance();

	// **プレイヤーのスポーン位置を決定**
	if (isFirstSpawn) {
		generate_location = Vector2D(200.0f, 600.0f);
		isFirstSpawn = false; // 初回スポーンが終わったのでフラグをオフ
	}
	else {
		generate_location = old_location; // 戦闘後は old_location に復帰
	}

	player = obj->CreateGameObject<Player>(generate_location);

	// **プレイヤーのマップデータを設定**
	player->SetMapData(mapdata);
	player->SetMapReference(this);

	// **old_location の初期値を設定**
	if (isFirstSpawn) {
		old_location = generate_location; // 初回は (200,600)
	}

	encounterStepCounter = 0;
	lastPlayerPos = player->GetLocation();

	StartFadeIn(); // フェードイン開始
}


// 更新処理
eSceneType Map::Update(float delta_second) {
	InputControl* input = Singleton<InputControl>::GetInstance();
	GameManager* obj = Singleton<GameManager>::GetInstance();

	// ゲームオブジェクトの更新
	obj->Update(delta_second);

	// フェードイン処理の更新
	if (isFadingIn)
		UpdateFadeIn(delta_second);

	// **戦闘復帰処理（1回だけ実行）**
	static bool wasInBattle = false; // 戦闘後の復帰フラグ
	if (wasInBattle && GetNowSceneType() == eSceneType::eMap) {
		player->SetLocation(old_location); // 戦闘前の位置に戻す
		wasInBattle = false;			   // フラグをリセット
	}

	// プレイヤーの現在位置を取得
	Vector2D currentPos = player->GetLocation();

	// **エンカウント処理**
	if ((int)currentPos.x != (int)lastPlayerPos.x || (int)currentPos.y != (int)lastPlayerPos.y) {
		encounterStepCounter++;
		lastPlayerPos = currentPos;

		// 5歩移動後、1% の確率でバトルへ遷移
		if (encounterStepCounter >= 5) {
			encounterStepCounter = 0;
			if (rand() % 100 < 1) {
				old_location = lastPlayerPos; // **戦闘前のプレイヤー位置を保存**
				wasInBattle = true;			  // **戦闘状態に入ったことを記録**
				return eSceneType::eBattle;
			}
		}
	}

	// **マップ遷移ポイントの処理**
	for (const auto& point : transitionPoints) {
		float distance = sqrt(pow(currentPos.x - point.x, 2) + pow(currentPos.y - point.y, 2));
		if (distance <= 20.0f) {
			LoadNextMap();
			StartFadeIn();
			break;
		}
	}

	// **タイトル画面へ戻る処理（Pキー）**
	if (input->GetKeyDown(KEY_INPUT_P)) {
		return eSceneType::eTitle;
	}

	return GetNowSceneType();
}



// 描画処理
void Map::Draw() {
	DrawStageMap();	 // マップの描画
	__super::Draw(); // 親クラスの描画処理
	if (isFadingIn)
		DrawFadeIn(); // フェードイン描画
}

// 終了処理
void Map::Finalize() {
	GameManager* obj = Singleton<GameManager>::GetInstance();
	obj->Finalize();
}

// 現在のシーンタイプを取得
eSceneType Map::GetNowSceneType() const {
	return eSceneType::eMap;
}

// マップデータをCSVからロード
std::vector<std::vector<char>> Map::LoadStageMapCSV(std::string map_name) {
	std::ifstream ifs(map_name);
	if (ifs.fail()) {
		throw std::runtime_error(map_name + " が開けませんでした。");
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
			collisionRow.push_back(c == '3'); // '3'は衝突判定
			if (c == '4') {					  // '4'はマップ遷移ポイント
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

// マップの描画
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

// 次のマップをロード
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

// フェードインの開始
void Map::StartFadeIn() {
	isFadingIn = true;
	fadeAlpha = 255.0f;
}

// フェードインの更新
void Map::UpdateFadeIn(float delta_second) {
	fadeAlpha -= fadeSpeed * delta_second;
	fadeAlpha = clamp(fadeAlpha, 0.0f, 255.0f);
	if (fadeAlpha <= 0.0f)
		isFadingIn = false;
}

// フェードインの描画
void Map::DrawFadeIn() {
	if (isFadingIn) {
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(fadeAlpha));
		DrawBox(0, 0, 960, 720, GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

// 衝突判定
bool Map::IsCollision(float x, float y) {
	int col = static_cast<int>(x / (D_OBJECT_SIZE * 2));
	int row = static_cast<int>(y / (D_OBJECT_SIZE * 2));
	return row < 0 || row >= collisionMap.size() || col < 0 || col >= collisionMap[row].size() || collisionMap[row][col];
}

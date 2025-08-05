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

#define MAP_SQUARE_Y 16	   // マップの縦のマス数
#define MAP_SQUARE_X 22	   // マップの横のマス数
#define D_OBJECT_SIZE 24.0 // オブジェクトの基本サイズ

static Vector2D old_location; // 戦闘開始時のプレイヤー位置
static int old_stageIndex;	  // 戦闘前のステージインデックス

static bool wasInBattle = false; // 戦闘復帰フラグ

// ▼ メニュー用追加変数
static Vector2D menu_old_location; // メニュー開始前の位置保存
static bool wasInMenu = false;	   // メニュー復帰フラグ

// コンストラクタ
Map::Map() : encounterStepCounter(0), lastPlayerPos(Vector2D(0, 0)), isFadingIn(false), fadeAlpha(255.0f) {
	srand(static_cast<unsigned int>(time(nullptr))); // 乱数シード
}

// デストラクタ
Map::~Map() {}

// 初期化処理
void Map::Initialize() {
	// マップデータをロード
	mapdata = LoadStageMapCSV(stageFiles[currentStageIndex]);

	GameManager* obj = Singleton<GameManager>::GetInstance();

	// **プレイヤーのスポーン位置決定**
	if (isFirstSpawn) {
		generate_location = Vector2D(200.0f, 600.0f);
		isFirstSpawn = false;
	}
	else {
		generate_location = old_location; // 戦闘・メニュー後は old_location に復帰
	}

	player = obj->CreateGameObject<Player>(generate_location);

	// プレイヤーのマップデータ設定
	player->SetMapData(mapdata);
	player->SetMapReference(this);

	// old_location 初期値
	if (isFirstSpawn) {
		old_location = generate_location;
		currentStageIndex = old_stageIndex;
	}

	encounterStepCounter = 0;
	lastPlayerPos = player->GetLocation();

	items = GenerateMapItems();


	StartFadeIn();
}


// 更新処理
eSceneType Map::Update(float delta_second) {
	InputControl* input = Singleton<InputControl>::GetInstance();
	GameManager* obj = Singleton<GameManager>::GetInstance();

	/*obj->Update(delta_second);*/

	// フェードイン更新
	if (isFadingIn)
		UpdateFadeIn(delta_second);

	// --- アイテム取得処理 ---
	PlayerData* pd = PlayerData::GetInstance();
	for (const auto& item : items) {
		if (!item->IsCollected() &&
			player->GetLocation().DistanceTo(item->GetPosition()) < D_OBJECT_SIZE) {
			item->Collect();					   // アイテムを取得済みに
			pd->AddCollectedItem(item->GetName()); // ★ PlayerData に記録
		}
	}

	// == = メニュー処理：最優先で処理 == =
	if (isMenuVisible) {
		// 入力処理のみ許可
		if (input->GetKeyDown(KEY_INPUT_DOWN)) {
			menuSelection = (menuSelection + 1) % menuItemCount;
		}
		if (input->GetKeyDown(KEY_INPUT_UP)) {
			menuSelection = (menuSelection - 1 + menuItemCount) % menuItemCount;
		}
		if (input->GetKeyDown(KEY_INPUT_RETURN)) {
			switch (menuSelection) {
			case 0:
				// 設定処理
				break;
			case 1:
				// クレジット処理
				break;
			case 2:
				// メニュー終了
				isMenuVisible = false;
				break;
			}
		}

		if (input->GetKeyDown(KEY_INPUT_RETURN)) {
			switch (menuSelection) {
			case 0: // 設定
				subMenuText = "音量: 100%\n画面サイズ: 960x720\n操作方法: キーボード";
				isSubMenuVisible = true;
				break;
			case 1: // アイテム
				subMenuText = "アイテムボックス：";
				{
					const auto& itemList = pd->GetCollectedItems();
					if (itemList.empty()) {
						subMenuText += "\n なし";
					}
					else {
						for (const auto& name : itemList) {
							subMenuText += "\n - " + name;
						}
					}
				}
				isSubMenuVisible = true;
				break;
			case 2: // メニューを閉じる
				isMenuVisible = false;
				isSubMenuVisible = false;
				subMenuText.clear();
				break;
			}
		}


		// メニュー中はプレイヤー・バトル処理等をスキップ
		return eSceneType::eMap;
	}
	obj->Update(delta_second);

	// **戦闘復帰処理**
	if (wasInBattle && GetNowSceneType() == eSceneType::eMap) {
		player->SetLocation(old_location);
		wasInBattle = false;
	}

	// **メニュー復帰処理（追加部分）**
	if (wasInMenu && GetNowSceneType() == eSceneType::eMap) {
		player->SetLocation(menu_old_location);
		wasInMenu = false;
	}

	// **エンカウント処理**
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

	//// **メニュー呼び出し処理（追加部分）**
	// if (input->GetKeyDown(KEY_INPUT_TAB)) {
	//	menu_old_location = player->GetLocation(); // 現在位置を保存
	//	wasInMenu = true;						   // 復帰フラグON
	//	return eSceneType::eMemu;				   // メニューへ遷移
	// }

	// **マップ遷移ポイント確認**
	for (const Vector2D& transitionPoint : transitionPoints) {
		if (currentPos.DistanceTo(transitionPoint) < D_OBJECT_SIZE) {
			LoadNextMap();
			break;
		}
	}


	// --- メニューを開く入力検出 ---
	if (input->GetKeyDown(KEY_INPUT_TAB)) {
		menu_old_location = player->GetLocation(); // メニュー復帰用に保存
		isMenuVisible = true;
		return eSceneType::eMap;
	}

	return GetNowSceneType();
}

// 描画処理
void Map::Draw() {
	DrawStageMap();
	__super::Draw();

	// --- アイテム描画ここに追加 ---
	for (const auto& item : items) {
		if (!item->IsCollected()) {
			DrawCircle(
				static_cast<int>(item->GetPosition().x),
				static_cast<int>(item->GetPosition().y),
				10,					   // 半径
				GetColor(255, 215, 0), // 金色
				TRUE				   // 塗りつぶし
			);
		}
	}

	if (isFadingIn)
		DrawFadeIn();

	PlayerData* pd = PlayerData::GetInstance();
	int PlayerHp = pd->GetHp();

	if (isMenuVisible) {
		const int menuX = 520;
		const int menuY = 100;
		const int menuWidth = 380;
		const int menuHeight = 240;

		// メニュー＋サブメニュー 背景ボックス
		DrawBox(menuX, menuY, menuX + menuWidth, menuY + menuHeight, GetColor(30, 30, 30), TRUE);
		DrawBox(menuX, menuY, menuX + menuWidth, menuY + menuHeight, GetColor(255, 255, 255), FALSE);

		// メニュー項目表示
		for (int i = 0; i < 3; ++i) {
			int y = menuY + 30 + i * 30;
			if (i == menuSelection) {
				DrawString(menuX + 20, y, ("＞ " + std::string(menuItems[i])).c_str(), GetColor(255, 255, 0));
			}
			else {
				DrawString(menuX + 40, y, menuItems[i], GetColor(255, 255, 255));
			}
		}

		// サブメニュー情報の描画（メニュー項目の下に表示）
		if (isSubMenuVisible && !subMenuText.empty()) {
			std::istringstream iss(subMenuText);
			std::string line;
			int lineNum = 0;
			const int subStartY = menuY + 20;
			const int subStartX = menuX;

			while (std::getline(iss, line)) {
				DrawString(menuX + 180, subStartY + lineNum * 20, line.c_str(), GetColor(200, 255, 200));
				lineNum++;
			}
		}
	}
}

// 終了処理
void Map::Finalize() {
	GameManager* obj = Singleton<GameManager>::GetInstance();
	obj->Finalize();
}

// 現在のシーンタイプ
eSceneType Map::GetNowSceneType() const {
	return eSceneType::eMap;
}

// CSV読み込み
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

// マップ描画
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

// 次のマップ
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

// フェードイン
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

// 衝突判定
bool Map::IsCollision(float x, float y) {
	int col = static_cast<int>(x / (D_OBJECT_SIZE * 2));
	int row = static_cast<int>(y / (D_OBJECT_SIZE * 2));
	return row < 0 || row >= collisionMap.size() || col < 0 || col >= collisionMap[row].size() || collisionMap[row][col];
}
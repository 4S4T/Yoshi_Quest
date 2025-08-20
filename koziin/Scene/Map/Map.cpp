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


	for (auto& item : items) {
		if (PlayerData::GetInstance()->IsCollected(item->GetId())) {
			item->Collect();
		}
	}



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

	 // ===== アイテム拾得 =====
	PlayerData* pd = PlayerData::GetInstance();
	for (const auto& it : items) {
		if (!it->IsCollected() &&
			player->GetLocation().DistanceTo(it->GetPosition()) < D_OBJECT_SIZE) {
			it->Collect();
			pd->AddItem(*it); // アイテムボックスへ
		}
	}

	// ===== メニュー =====
	if (isMenuVisible) {
		// トップメニューのカーソルはサブメニュー操作中は動かさない
		if (!(isSubMenuVisible && isItemListActive)) {
			if (input->GetKeyDown(KEY_INPUT_DOWN)) {
				menuSelection = (menuSelection + 1) % menuItemCount;
			}
			if (input->GetKeyDown(KEY_INPUT_UP)) {
				menuSelection = (menuSelection - 1 + menuItemCount) % menuItemCount;
			}
			// 「もどる」を指している間はサブメニューを即クリア（何も描画しない）
			if (menuSelection == 3) { // もどる
				isSubMenuVisible = false;
				isItemListActive = false;
				subMenuText.clear();
				subMenuItemIds.clear();
			}
		}


		// サブメニュー（どうぐ一覧）の操作
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
						subMenuText = "装備しました：\n " + selected.GetName();
					}
					else {
						subMenuText = "このアイテムは装備できません：\n " + selected.GetName();
					}
				}
			}
			// Esc / Backspace で一覧を閉じる
			if (input->GetKeyDown(KEY_INPUT_ESCAPE) || input->GetKeyDown(KEY_INPUT_SPACE)) {
				isItemListActive = false;
				isSubMenuVisible = false;
				subMenuItemIds.clear();
				subMenuText.clear();
			}
		}

		// トップメニューのEnterはサブメニュー操作中は無効
		if (!(isSubMenuVisible && isItemListActive)) {
			if (input->GetKeyDown(KEY_INPUT_RETURN)) {
				switch (menuSelection) {
				case 0: {						// どうぐ
					subMenuText = "   ";
					subMenuItemIds.clear();
					const auto& owned = PlayerData::GetInstance()->GetOwnedItems();
					for (const auto& kv : owned)
						subMenuItemIds.push_back(kv.first);
					if (subMenuItemIds.empty()) {
						subMenuText += "\n なし";
						isItemListActive = false;
					}
					else {
						subMenuSelection = 0;
						isItemListActive = true;
					}
					isSubMenuVisible = true;
				} break;

				case 1: { // そうび
					PlayerData* pd = PlayerData::GetInstance();
					subMenuText = "装備一覧";
					subMenuText += "\n 武器: " + pd->GetEquippedName(EquipCategory::Weapon);
					subMenuText += "\n 盾:   " + pd->GetEquippedName(EquipCategory::Shield);
					subMenuText += "\n 防具: " + pd->GetEquippedName(EquipCategory::Armor);
					subMenuText += "\n 頭: " + pd->GetEquippedName(EquipCategory::Helmet);
					isItemListActive = false;
					subMenuItemIds.clear();
					isSubMenuVisible = true;
				} break;

				case 2: { // つよさ
					PlayerData* pd = PlayerData::GetInstance();
					subMenuText = "プレイヤー情報";
					subMenuText += "\n レベル: " + std::to_string(pd->GetLevel());
					subMenuText += "\n HP: " + std::to_string(pd->GetHp()) + " / " + std::to_string(pd->GetMaxHp());
					subMenuText += "\n 攻撃力: " + std::to_string(pd->GetAttack());
					subMenuText += "\n 防御力: " + std::to_string(pd->GetDefense());
					isItemListActive = false;
					subMenuItemIds.clear();
					isSubMenuVisible = true;
				} break;

				case 3: // もどる
					isMenuVisible = false;
					isSubMenuVisible = false;
					isItemListActive = false;
					subMenuItemIds.clear();
					subMenuText.clear();
					break;
				}
			}
		}


		// メニュー中はプレイヤー・バトル処理等をスキップ
		return eSceneType::eMap;
	}

	// 通常更新
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

	// **マップ遷移ポイント確認**
	for (const Vector2D& transitionPoint : transitionPoints) {
		if (currentPos.DistanceTo(transitionPoint) < D_OBJECT_SIZE) {
			LoadNextMap();
			break;
		}
	}

	// メニューを開く
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

	 // 未取得アイテムを描画
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

	// メニュー描画
	if (isMenuVisible) {
		const int menuX = 520;
		const int menuY = 100;
		const int menuWidth = 380;
		const int menuHeight = 260;

		// 背景
		DrawBox(menuX, menuY, menuX + menuWidth, menuY + menuHeight, GetColor(30, 30, 30), TRUE);
		DrawBox(menuX, menuY, menuX + menuWidth, menuY + menuHeight, GetColor(255, 255, 255), FALSE);

		// 左列：メニュー項目
		for (int i = 0; i < 4; ++i) {
			int y = menuY + 30 + i * 30;
			if (i == menuSelection)
				DrawString(menuX + 20, y, ("＞ " + std::string(menuItems[i])).c_str(), GetColor(255, 255, 0));
			else
				DrawString(menuX + 40, y, menuItems[i], GetColor(255, 255, 255));
		}

		// サブメニュー情報の描画（「どうぐ」一覧中は左下／それ以外は右上。※「もどる」選択中は描かない）
		if (isSubMenuVisible && menuSelection != 3 && !subMenuText.empty()) {
			std::istringstream iss(subMenuText);
			std::string line;
			int lineNum = 0;

			// どうぐ一覧のときは右側リストと被るので左下へ、それ以外は右上へ
			const bool showBottomLeft = (menuSelection == 0 && isItemListActive);
			const int textX = showBottomLeft ? (menuX + 20) : (menuX + 200);
			const int textY = showBottomLeft ? (menuY + 150) : (menuY + 20);

			while (std::getline(iss, line)) {
				DrawString(textX, textY + lineNum * 20, line.c_str(), GetColor(200, 255, 200));
				lineNum++;
			}
		}


		// 右列：「どうぐ」一覧（カーソル＋装備マーク）※「どうぐ」選択時のみ
		if (isSubMenuVisible && menuSelection == 0 && isItemListActive && !subMenuItemIds.empty()) {
			const int listX = menuX + 200; // 右側
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

				// カテゴリ記号
				std::string label;
				switch (obj.GetCategory()) {
				case EquipCategory::Weapon:
					label = "[武] ";
					break;
				case EquipCategory::Shield:
					label = "[盾] ";
					break;
				case EquipCategory::Armor:
					label = "[鎧] ";
					break;
				case EquipCategory::Helmet:
					label = "[頭] ";
					break;
				default:
					label = "     ";
					break;
				}
				label += obj.GetName();

				// 装備中なら★
				int eqId = pd->GetEquippedId(obj.GetCategory());
				if (eqId == id && obj.GetCategory() != EquipCategory::None) {
					label = "★ " + label;
				}

				// カーソル
				if (i == subMenuSelection) {
					DrawString(listX - 20, listY + i * lineH, "＞", GetColor(255, 255, 0));
				}

				// ラベル
				DrawString(listX, listY + i * lineH, label.c_str(), GetColor(220, 255, 220));
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
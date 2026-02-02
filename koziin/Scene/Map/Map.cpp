// --- 重要：NOMINMAX を最初に、DxLib -> <algorithm> -> <map> の順 --- //
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
#include "../../Utility/SoundManager.h"
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

// ★ メニュー位置用（必要に応じて使用）
static Vector2D menu_old_location;
static bool wasInMenu = false;

// =====================================================
// ★★ 追加：パスを正規化して「安全地帯」を確実に判定する
// =====================================================
static std::string NormalizePath(std::string s) {
	// \ を / に統一
	for (char& ch : s) {
		if (ch == '\\')
			ch = '/';
	}

	// 小文字化（Windows想定）
	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return (char)std::tolower(c); });

	return s;
}

static bool IsSafeZoneMapPath(const std::string& path) {
	std::string p = NormalizePath(path);

	// 「mati」「stage2」を含んでたら安全地帯扱い
	if (p.find("mati") != std::string::npos)
		return true;
	if (p.find("stage2") != std::string::npos)
		return true;

	return false;
}

// ========== ★ Rebuild (共通) ==========
// 今の、アイテムのグループ一覧を作り直す
void Map::RebuildItemList() {
	RebuildItemListGrouped();
	// 追加： subMenuSelection / スクロール位置を補正
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

// ========== ★ 新：グルーピング再構築 ==========
// PlayerData::ownedItems から、同名消費アイテムはまとめる
void Map::RebuildItemListGrouped() {
	groupedItems.clear();

	PlayerData* pd = PlayerData::GetInstance();
	const auto& owned = pd->GetOwnedItems();

	// まず名前でまとめる（Consumable）
	std::map<std::string, int> nameCount; // 名前 -> 個数
	for (auto& kv : owned) {
		if (kv.second.GetType() == ItemType::Consumable) {
			nameCount[kv.second.GetName()]++;
		}
	}

	// 表示用配列を構築
	// 1) 装備：1個ずつ
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

	// 2) 消費：名前ごとに1行
	for (auto& pair : nameCount) {
		// 代表IDは最初に見つけたIDでOK（Useで1個処理する）
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

	// 3) ソート：装備優先、装備はカテゴリ順→名前、消費は名前
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
	std::sort(
		groupedItems.begin(), groupedItems.end(),
		[&](const Map::MenuEntry& a, const Map::MenuEntry& b) {
			if (a.type != b.type)
				return a.type == ItemType::Equipment; // 装備が先
			if (a.type == ItemType::Equipment) {
				if (rankCat(a.category) != rankCat(b.category))
					return rankCat(a.category) < rankCat(b.category);
			}
			return a.name < b.name;
		});
}

// コンストラクタ / デストラクタ
Map::Map()
	: encounterStepCounter(0), lastPlayerPos(Vector2D(0, 0)), isFadingIn(false), fadeAlpha(255.0f) {
	srand(static_cast<unsigned int>(time(nullptr)));
}
Map::~Map() {}

// 初期化
void Map::Initialize() {
	// バトルから戻る場合：バトル前のステージに戻す
	if (wasInBattle) {
		currentStageIndex = old_stageIndex;
	}

	// ★ このステージ index に対応する CSV を読み込む
	// 重要：isEncounterEnabled は LoadStageMapCSV 内で確定する
	mapdata = LoadStageMapCSV(stageFiles[currentStageIndex]);

	GameManager* obj = Singleton<GameManager>::GetInstance();

	SoundManager::GetInstance().PlayBGM("Resource/Sounds/Title.mp3");
	soubi_Sound = LoadSoundMem("Resource/Sounds/槍/選択/装備フラグ音.mp3");
	sentaku_Sound = LoadSoundMem("Resource/Sounds/Cursor.mp3");
	erabu_Sound = LoadSoundMem("Resource/Sounds/槍/選択/選択1.mp3");
	modoru_Sound = LoadSoundMem("Resource/Sounds/槍/選択/cancel.mp3");

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

	encounterStepCounter = 0;
	lastPlayerPos = player->GetLocation();

	// アイテム配置
	items.clear();
	if (isEncounterEnabled) {
		items = GenerateMapItems();
		for (auto& item : items) {
			if (PlayerData::GetInstance()->IsCollected(item->GetId())) {
				item->Collect();
			}
		}
	}

	// NPC
	ncps.clear();
	if (!isEncounterEnabled) {
		std::vector<std::string> lines{
			"街は安全地帯だ。", "この町はできてるかな？", "Zキーで話を進める。"
		};
		ncps.push_back(std::make_shared<NCP>(Vector2D(320.0f, 600.0f), "町の中心", lines));
	}

	StartFadeIn();
}

eSceneType Map::Update(float delta_second) {
	InputControl* input = Singleton<InputControl>::GetInstance();
	GameManager* obj = Singleton<GameManager>::GetInstance();

	if (isFadingIn)
		UpdateFadeIn(delta_second);

	// --- 触れてる、アイテム回収（エンカウントキャンセルのみ） ---
	if (isEncounterEnabled) {
		PlayerData* pd = PlayerData::GetInstance();
		for (const auto& it : items) {
			if (!it->IsCollected() && player->GetLocation().DistanceTo(it->GetPosition()) < D_OBJECT_SIZE) {
				it->Collect();
				pd->AddItem(*it);
			}
		}
	}

	bool move =
		input->GetKeyDown(KEY_INPUT_DOWN) ||
		input->GetKeyDown(KEY_INPUT_UP);

	if (move && CheckSoundMem(erabu_Sound) == 0) {
		PlaySoundMem(erabu_Sound, DX_PLAYTYPE_BACK);
	}

	// ================== メニュー処理 ==================
	if (isMenuVisible) {
		// （あなたのコードそのまま）
		// …（中身省略せず、貼ってる内容をそのまま使ってOK）
		// ※ここはあなたが貼ったままなので変更無し
	}

	// ================== メニュー外 ==================

	// NPC 会話（あなたのコードそのまま）
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

	// 通常更新
	obj->Update(delta_second);

	// バトル復帰
	if (wasInBattle && GetNowSceneType() == eSceneType::eMap) {
		player->SetLocation(old_location);
		wasInBattle = false;
		encounterStepCounter = 0;
		encounterCooldownTimer = encounterCooldown;
		isAfterBattle = true;
		lastPlayerPos = player->GetLocation();
	}

	// メニュー復帰（必要なら）
	if (wasInMenu && GetNowSceneType() == eSceneType::eMap) {
		player->SetLocation(menu_old_location);
		wasInMenu = false;
	}

	// === タイル 6・7・8 に触れたらマップ切替 ===
	{
		int col = static_cast<int>(player->GetLocation().x / (D_OBJECT_SIZE * 2));
		int row = static_cast<int>(player->GetLocation().y / (D_OBJECT_SIZE * 2));

		if (row >= 0 && row < (int)mapdata.size() &&
			col >= 0 && col < (int)mapdata[row].size()) {

			char tile = mapdata[row][col];

			// 6,7 = Book2 / 8 = mati
			if (tile == '6' || tile == '7' || tile == '8') {

				// --- 8: mati.csv に遷移 (エンカウントOFF) ---
				if (tile == '8') {
					if (stageFiles.size() > 2) {
						currentStageIndex = 2; // mati.csv

						mapdata = LoadStageMapCSV(stageFiles[currentStageIndex]); // ← LoadStageMapCSV内で確実にOFFになる
						player->SetMapData(mapdata);

						// mati のスタート位置（必要なら変えて）
						player->SetLocation(Vector2D(200.0f, 600.0f));

						ncps.clear();

						// mati はアイテムも出さない
						items.clear();

						encounterStepCounter = 0;
						encounterCooldownTimer = encounterCooldown;
						lastPlayerPos = player->GetLocation();

						StartFadeIn();
					}
					return eSceneType::eMap;
				}

				// --- 6/7: Book2.csv に遷移 (エンカウントON) ---
				if (stageFiles.size() > 1) {
					currentStageIndex = 1; // Book2.csv

					mapdata = LoadStageMapCSV(stageFiles[currentStageIndex]);
					player->SetMapData(mapdata);

					player->SetLocation(Vector2D(200.0f, 600.0f));

					ncps.clear();

					// アイテム配置（エンカウントONのときのみ）
					items.clear();
					if (isEncounterEnabled) {
						items = GenerateMapItems();
						for (auto& item : items) {
							if (PlayerData::GetInstance()->IsCollected(item->GetId())) {
								item->Collect();
							}
						}
					}

					encounterStepCounter = 0;
					encounterCooldownTimer = encounterCooldown;
					lastPlayerPos = player->GetLocation();

					StartFadeIn();
				}

				return eSceneType::eMap;
			}
		}
	}

	// =====================================================
	// ★最終保険：今のステージファイルが「安全地帯」なら絶対エンカしない
	// =====================================================
	if (currentStageIndex >= 0 && currentStageIndex < (int)stageFiles.size()) {
		if (IsSafeZoneMapPath(stageFiles[currentStageIndex])) {
			isEncounterEnabled = false; // 強制OFF
			encounterStepCounter = 0;
			encounterCooldownTimer = std::max(encounterCooldownTimer, encounterCooldown);
			lastPlayerPos = player->GetLocation();
			return eSceneType::eMap;
		}
	}

	// （あなたが入れてた保険も残す：isEncounterEnabledがOFFなら即return）
	if (!isEncounterEnabled) {
		encounterStepCounter = 0;
		encounterCooldownTimer = std::max(encounterCooldownTimer, encounterCooldown);
		lastPlayerPos = player->GetLocation();
		return eSceneType::eMap;
	}

	// エンカウント
	Vector2D currentPos = player->GetLocation();
	if (encounterCooldownTimer > 0.0f) {
		encounterCooldownTimer -= delta_second;
		lastPlayerPos = currentPos;
	}
	else if (isEncounterEnabled &&
			 ((int)currentPos.x != (int)lastPlayerPos.x || (int)currentPos.y != (int)lastPlayerPos.y)) {

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
				SoundManager::GetInstance().StopBGM();
				return eSceneType::eBattle;
			}
		}
	}

	// マップ遷移ポイント
	for (const Vector2D& transitionPoint : transitionPoints) {
		if (currentPos.DistanceTo(transitionPoint) < D_OBJECT_SIZE) {
			LoadNextMap();
			break;
		}
	}

	// メニューを開く
	if (input->GetKeyDown(KEY_INPUT_TAB)) {
		PlaySoundMem(erabu_Sound, DX_PLAYTYPE_NORMAL);
		isMenuVisible = true;
		isSubMenuVisible = false;
		rightMode = RightMode::None;
		groupedItems.clear();
		subMenuText.clear();
		return eSceneType::eMap;
	}

	return GetNowSceneType();
}

// 描画
void Map::Draw() {
	DrawStageMap();
	__super::Draw();

	// 触れてる、アイテム
	if (isEncounterEnabled) {
		for (const auto& it : items) {
			if (!it->IsCollected()) {
				DrawCircle(
					(int)it->GetPosition().x,
					(int)it->GetPosition().y,
					10,
					GetColor(255, 215, 0),
					TRUE);
			}
		}
	}

	// NPC
	for (const auto& npc : ncps)
		npc->Draw();

	if (isFadingIn)
		DrawFadeIn();

	// メニュー描画（あなたのコードそのまま）
	// ※ここは貼ってる内容をそのまま使ってOK（変更無し）
}

// ヘルプ
void Map::DrawHelpPanel(int x, int y, int w, int h) {
	DrawBox(x, y, x + w, y + h, GetColor(30, 30, 30), TRUE);
	DrawBox(x, y, x + w, y + h, GetColor(120, 120, 120), FALSE);
	DrawString(x + 8, y + 3, "↑↓:選択  Enter:決定  space: 戻る", GetColor(210, 210, 210));
}

// ステータス
void Map::DrawStatusPanel(int x, int y, int w, int h) {
	DrawBox(x, y, x + w, y + h, GetColor(30, 30, 40), TRUE);
	DrawBox(x, y, x + w, y + h, GetColor(200, 200, 200), FALSE);
	PlayerData* pd = PlayerData::GetInstance();
	std::string txt =
		"勇者たろう　Lv " + std::to_string(pd->GetLevel()) + "   /HP " + std::to_string(pd->GetHp());
	DrawString(x + 12, y + 10, txt.c_str(), GetColor(240, 240, 240));
}

// メニュー右側どうぐ（あなたのコードそのまま）
void Map::DrawItemListPanel(int x, int y, int w, int h) {
	// …（貼ってる内容そのまま）
}

// カーソル位置のアイテム説明（あなたのコードそのまま）
std::string Map::BuildItemPreviewText(const Map::MenuEntry& e) {
	// …（貼ってる内容そのまま）
}

// 終了
void Map::Finalize() {
	GameManager* obj = Singleton<GameManager>::GetInstance();
	obj->Finalize();

	if (soubi_Sound != -1) {
		DeleteSoundMem(soubi_Sound);
		soubi_Sound = -1;
	}
	if (sentaku_Sound != -1) {
		DeleteSoundMem(sentaku_Sound);
		sentaku_Sound = -1;
	}
	if (erabu_Sound != -1) {
		DeleteSoundMem(erabu_Sound);
		erabu_Sound = -1;
	}
	if (modoru_Sound != -1) {
		DeleteSoundMem(modoru_Sound);
		modoru_Sound = -1;
	}
}

// 現在のシーンタイプ
eSceneType Map::GetNowSceneType() const {
	return eSceneType::eMap;
}

// ============================
// CSV 読み込み
// ============================
std::vector<std::vector<char>> Map::LoadStageMapCSV(std::string map_name) {
	std::ifstream ifs(map_name);
	if (ifs.fail())
		throw std::runtime_error(map_name + " が開けませんでした。");

	std::vector<std::vector<char>> data;
	collisionMap.clear();
	transitionPoints.clear();

	std::string line;

	// ★★★ 修正：安全地帯は「完全一致」じゃなく「含む判定」にする ★★★
	isEncounterEnabled = !IsSafeZoneMapPath(map_name);

	while (std::getline(ifs, line)) {
		std::vector<char> row;
		std::vector<bool> collisionRow;
		std::stringstream ss(line);
		std::string cell;

		while (std::getline(ss, cell, ',')) {

			while (!cell.empty() &&
				   (cell.back() == ' ' || cell.back() == '\t' ||
					   cell.back() == '\r' || cell.back() == '\n')) {
				cell.pop_back();
			}

			size_t pos = 0;
			while (pos < cell.size() && (cell[pos] == ' ' || cell[pos] == '\t')) {
				++pos;
			}

			char c;
			if (pos >= cell.size()) {
				c = '0';
			}
			else {
				c = cell[pos];
				if (c < '0' || c > '9') {
					c = '0';
				}
			}

			row.push_back(c);

			// 衝突：'3' と '4' が通行不可（6,7,8は通行可）
			collisionRow.push_back(c == '3' || c == '4');
		}

		data.push_back(row);
		collisionMap.push_back(collisionRow);
	}
	return data;
}

// ============================
// マップ描画（ドット風）
// ============================
void Map::DrawStageMap() {
	ResourceManager* rm = ResourceManager::GetInstance();

	for (int i = 0; i < MAP_SQUARE_Y; i++) {
		if (i < 0 || i >= (int)mapdata.size())
			continue;

		for (int j = 0; j < MAP_SQUARE_X; j++) {
			if (j < 0 || j >= (int)mapdata[i].size())
				continue;

			char c = mapdata[i][j];

			int tileIndex = c - '0';
			if (tileIndex < 0 || tileIndex > 9)
				tileIndex = 0;

			std::string path = "Resource/Images/Block/" + std::to_string(tileIndex) + ".png";

			MapImage = rm->GetImages(path, 1, 1, 1, 16, 16)[0];
			DrawRotaGraphF(
				D_OBJECT_SIZE + ((D_OBJECT_SIZE * 2) * j),
				D_OBJECT_SIZE + ((D_OBJECT_SIZE * 2) * i),
				1.9f,
				0.0f,
				MapImage,
				TRUE);
		}
	}
}

// 次マップ
void Map::LoadNextMap() {
	currentStageIndex++;
	if (currentStageIndex >= (int)stageFiles.size())
		currentStageIndex = 0;

	transitionPoints.clear();

	mapdata = LoadStageMapCSV(stageFiles[currentStageIndex]);
	player->SetMapData(mapdata);
	player->SetLocation(Vector2D(200.0f, 600.0f));

	// マップ切替時：歩数持ち越しを潰す
	encounterStepCounter = 0;
	encounterCooldownTimer = encounterCooldown;
	lastPlayerPos = player->GetLocation();

	// NPC
	ncps.clear();
	if (!isEncounterEnabled) {
		std::vector<std::string> lines{ "街は安全地帯だ。", "この町はできてるかな？" };
		ncps.push_back(std::make_shared<NCP>(Vector2D(320.0f, 600.0f), "町の中心", lines));
		ncps.back()->SetImage("Resource/Images/peabird.png");
	}
	if (!isEncounterEnabled) {
		std::vector<std::string> lines{ "人間よぉぉ！" };
		ncps.push_back(std::make_shared<NCP>(Vector2D(220.0f, 300.0f), "町人A", lines));
		ncps.back()->SetImage("Resource/Images/yossi_ikiri.png");
	}
}

// フェード
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
	return row < 0 ||
		   row >= (int)collisionMap.size() ||
		   col < 0 ||
		   col >= (int)collisionMap[row].size() ||
		   collisionMap[row][col];
}

/* =========================
   そうび：補助用関数
   ========================= */

void Map::BuildEquipFilteredList(EquipCategory cat) {
	equipFiltered.clear();
	const auto& owned = PlayerData::GetInstance()->GetOwnedItems();
	auto* pd = PlayerData::GetInstance();
	for (auto& kv : owned) {
		const Item& it = kv.second;
		if (it.GetType() == ItemType::Equipment && it.GetCategory() == cat) {
			MenuEntry e{};
			e.representativeId = kv.first;
			e.name = it.GetName();
			e.type = ItemType::Equipment;
			e.category = cat;
			e.count = 1;
			e.equipped = (pd->GetEquippedId(cat) == kv.first);
			equipFiltered.push_back(e);
		}
	}
	std::sort(
		equipFiltered.begin(), equipFiltered.end(),
		[](const MenuEntry& a, const MenuEntry& b) { return a.name < b.name; });
}

void Map::DrawEquipSlotsPanel(int x, int y, int w, int h) {
	DrawBox(x, y, x + w, y + h, GetColor(26, 26, 32), TRUE);
	DrawBox(x, y, x + w, y + h, GetColor(140, 140, 140), FALSE);

	PlayerData* pd = PlayerData::GetInstance();
	struct Row {
		const char* label;
		EquipCategory cat;
	};
	Row rows[4] = {
		{ "武器", EquipCategory::Weapon },
		{ "盾", EquipCategory::Shield },
		{ "鎧", EquipCategory::Armor },
		{ "兜", EquipCategory::Helmet }
	};

	for (int i = 0; i < 4; ++i) {
		int yy = y + 6 + i * 24;
		if (i == equipSlotSelection)
			DrawString(x + 8, yy, "→", GetColor(255, 255, 0));
		std::string line = std::string(rows[i].label) + "： " + pd->GetEquippedName(rows[i].cat);
		DrawString(x + 28, yy, line.c_str(), GetColor(200, 255, 200));
	}
}

void Map::DrawEquipItemListPanel(int x, int y, int w, int h) {
	DrawBox(x, y, x + w, y + h, GetColor(26, 26, 32), TRUE);
	DrawBox(x, y, x + w, y + h, GetColor(140, 140, 140), FALSE);

	DrawString(x + 8, y + 3, "候補", GetColor(255, 255, 180));
	{
		int n = (int)equipFiltered.size();
		int totalPages = (n == 0) ? 1 : ((n + VISIBLE_ROWS - 1) / VISIBLE_ROWS);
		int currentPage = (n == 0) ? 1 : ((equipItemScrollOffset / VISIBLE_ROWS) + 1);
		std::string pg = std::to_string(currentPage) + " / " + std::to_string(totalPages);
		DrawString(x + w - 80, y + 190, pg.c_str(), GetColor(220, 220, 220));
	}

	const int lineH = 20;
	const int offsetY = 30;

	int maxRows = std::min(VISIBLE_ROWS, (int)equipFiltered.size());

	for (int i = 0; i < maxRows; ++i) {
		int idx = equipItemScrollOffset + i;
		if (idx < 0 || idx >= (int)equipFiltered.size())
			break;
		const auto& e = equipFiltered[idx];

		std::string label = e.name;
		if (e.equipped)
			label = "★ " + label;

		if (idx == equipItemSelection)
			DrawString(x + 8, y + offsetY + 6 + i * lineH, "→", GetColor(255, 255, 0));

		DrawString(x + 28, y + offsetY + 6 + i * lineH, label.c_str(), GetColor(220, 255, 220));
	}

	if ((int)equipFiltered.size() > VISIBLE_ROWS) {
		float ratio = (float)VISIBLE_ROWS / (float)equipFiltered.size();
		int barH = std::max(8, (int)(h * ratio));
		int track = h - barH;
		float posRatio = (float)equipItemScrollOffset /
						 std::max(1, (int)equipFiltered.size() - VISIBLE_ROWS);
		int barY = y + (int)(posRatio * track);
		int barX = x + w - 8;
		DrawBox(barX, y, barX + 4, y + h, GetColor(60, 60, 60), TRUE);
		DrawBox(barX, barY, barX + 4, barY + barH, GetColor(200, 200, 200), TRUE);
	}
}

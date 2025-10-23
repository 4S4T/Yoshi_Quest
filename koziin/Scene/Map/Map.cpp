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

// ▼ メニュー位置復帰（必要に応じて使用）
static Vector2D menu_old_location;
static bool wasInMenu = false;

// ========== 旧 Rebuild (互換) ==========
void Map::RebuildItemList() {
	RebuildItemListGrouped();
	// 既存の subMenuSelection / スクロール位置を補正
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

// ========== 新：グルーピング再構築 ==========
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
	// 1) 装備：1件ずつ
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
		// 代表IDは最初に見つかったIDでOK（Use時に1個消える）
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

	// 3) ソート：装備→消費、装備はカテゴリ優先→名前、消費は名前
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
				return a.type == ItemType::Equipment; // 装備を先
			if (a.type == ItemType::Equipment) {
				if (rankCat(a.category) != rankCat(b.category))
					return rankCat(a.category) < rankCat(b.category);
			}
			return a.name < b.name;
		});
}

// コンストラクタ / デストラクタ
Map::Map() : encounterStepCounter(0), lastPlayerPos(Vector2D(0, 0)), isFadingIn(false), fadeAlpha(255.0f) {
	srand(static_cast<unsigned int>(time(nullptr)));
}
Map::~Map() {}

// 初期化
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
			"ここは安全地帯だ。", "旅の準備はできているかい？", "Zキーで会話を進められるよ。"
		};
		ncps.push_back(std::make_shared<NCP>(Vector2D(320.0f, 600.0f), "村の長老", lines));
	}

	StartFadeIn();
}

eSceneType Map::Update(float delta_second) {
	InputControl* input = Singleton<InputControl>::GetInstance();
	GameManager* obj = Singleton<GameManager>::GetInstance();

	if (isFadingIn)
		UpdateFadeIn(delta_second);

	// --- 落ちているアイテム拾得（エンカウント有効時のみ） ---
	if (isEncounterEnabled) {
		PlayerData* pd = PlayerData::GetInstance();
		for (const auto& it : items) {
			if (!it->IsCollected() && player->GetLocation().DistanceTo(it->GetPosition()) < D_OBJECT_SIZE) {
				it->Collect();
				pd->AddItem(*it);
			}
		}
	}

	// ================== メニュー処理 ==================
	if (isMenuVisible) {
		// 「どうぐ」を指していて未構築なら一覧を準備
		if (menuSelection == 0 && groupedItems.empty()) {
			RebuildItemList();
			subMenuSelection = 0;
			listScrollOffset = 0;
		}

		// 右リストに“フォーカス”がある条件（見た目の描画とは独立）
		bool listActive = (menuSelection == 0 && isSubMenuVisible && !groupedItems.empty());

		// ---------- 左メニュー操作（フォーカス無し） ----------
		if (!listActive) {
			int before = menuSelection;

			if (input->GetKeyDown(KEY_INPUT_DOWN))
				menuSelection = (menuSelection + 1) % menuItemCount;
			if (input->GetKeyDown(KEY_INPUT_UP))
				menuSelection = (menuSelection - 1 + menuItemCount) % menuItemCount;

			if (menuSelection != before) {
				subMenuText.clear();
				// どうぐに来たらフォーカスON、他へ動いたらOFF
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

			// Enter：どうぐ以外の決定
			if (input->GetKeyDown(KEY_INPUT_RETURN)) {
				switch (menuSelection) {
				case 1: { // そうび
					PlayerData* pd = PlayerData::GetInstance();
					subMenuText = "装備一覧";
					subMenuText += "\n 武器: " + pd->GetEquippedName(EquipCategory::Weapon);
					subMenuText += "\n 盾:   " + pd->GetEquippedName(EquipCategory::Shield);
					subMenuText += "\n 防具: " + pd->GetEquippedName(EquipCategory::Armor);
					subMenuText += "\n 頭:   " + pd->GetEquippedName(EquipCategory::Helmet);
					isSubMenuVisible = false;
					groupedItems.clear();
				} break;
				case 2: { // つよさ
					PlayerData* pd = PlayerData::GetInstance();
					subMenuText = "プレイヤー情報";
					subMenuText += "\n レベル: " + std::to_string(pd->GetLevel());
					subMenuText += "\n HP: " + std::to_string(pd->GetHp()) + " / " + std::to_string(pd->GetMaxHp());
					subMenuText += "\n 攻撃力: " + std::to_string(pd->GetAttack());
					subMenuText += "\n 防御力: " + std::to_string(pd->GetDefense());
					isSubMenuVisible = false;
					groupedItems.clear();
				} break;
				case 3: // もどる
					isMenuVisible = false;
					isSubMenuVisible = false;
					groupedItems.clear();
					subMenuText.clear();
					return eSceneType::eMap;
				}
			}

			return eSceneType::eMap; // メニュー表示継続
		}

		// ---------- 右：どうぐリスト操作（フォーカス有り） ----------
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
		// ページ送り
		if (input->GetKeyDown(KEY_INPUT_A) || input->GetKeyDown(KEY_INPUT_PGUP)) {
			subMenuSelection = std::max(0, subMenuSelection - VISIBLE_ROWS);
			listScrollOffset = std::max(0, listScrollOffset - VISIBLE_ROWS);
		}
		if (input->GetKeyDown(KEY_INPUT_D) || input->GetKeyDown(KEY_INPUT_PGDN)) {
			subMenuSelection = std::min((int)groupedItems.size() - 1, subMenuSelection + VISIBLE_ROWS);
			listScrollOffset = std::min(std::max(0, (int)groupedItems.size() - VISIBLE_ROWS), listScrollOffset + VISIBLE_ROWS);
		}

		// Enter：装備 / 使用
		if (input->GetKeyDown(KEY_INPUT_RETURN)) {
			const MenuEntry& entry = groupedItems[subMenuSelection];
			const auto& owned = PlayerData::GetInstance()->GetOwnedItems();

			auto it = owned.find(entry.representativeId);
			if (it != owned.end()) {
				const Item& selected = it->second;
				if (entry.type == ItemType::Equipment && selected.GetCategory() != EquipCategory::None) {
					PlayerData::GetInstance()->EquipItem(selected.GetCategory(), entry.representativeId);
					subMenuText = "装備しました：\n " + selected.GetName();
				}
				else if (entry.type == ItemType::Consumable) {
					const std::string name = selected.GetName();
					const int heal = selected.GetHealAmount();
					if (PlayerData::GetInstance()->UseItem(entry.representativeId)) {
						subMenuText = "使用しました：\n " + name + "（+" + std::to_string(heal) + " HP）";
					}
					else {
						subMenuText = "使用できません：\n " + name;
					}
				}
				else {
					subMenuText = "このアイテムは使用できません：\n " + selected.GetName();
				}
			}

			// 反映（個数減少や★更新）
			RebuildItemList();

			// 自動では左へ戻らない。空になった場合のみ戻す。
			if (groupedItems.empty()) {
				isSubMenuVisible = false; // 左メニューへフォーカス
				subMenuSelection = 0;
				listScrollOffset = 0;
			}
			else {
				// 選択位置を可能な限り維持（画面内補正）
				subMenuSelection = std::min(subMenuSelection, (int)groupedItems.size() - 1);
				if (subMenuSelection < listScrollOffset)
					listScrollOffset = subMenuSelection;
				if (subMenuSelection >= listScrollOffset + VISIBLE_ROWS)
					listScrollOffset = std::max(0, subMenuSelection - (VISIBLE_ROWS - 1));
			}
		}

		// Space / Esc：任意のタイミングで左メニューへ戻る
		if (input->GetKeyDown(KEY_INPUT_SPACE) || input->GetKeyDown(KEY_INPUT_ESCAPE)) {
			isSubMenuVisible = false;
			// subMenuText は残す/消すは好みで
			// subMenuText.clear();
		}

		return eSceneType::eMap; // メニュー表示継続
	}
	// ================== メニュー以外 ==================

	// NPC 会話
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

	// 戦闘復帰
	if (wasInBattle && GetNowSceneType() == eSceneType::eMap) {
		player->SetLocation(old_location);
		wasInBattle = false;
		encounterStepCounter = 0;
		encounterCooldownTimer = encounterCooldown;
		isAfterBattle = true;
		lastPlayerPos = player->GetLocation();
	}

	// メニュー復帰
	if (wasInMenu && GetNowSceneType() == eSceneType::eMap) {
		player->SetLocation(menu_old_location);
		wasInMenu = false;
	}

	// エンカウント
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

	// マップ遷移ポイント
	for (const Vector2D& transitionPoint : transitionPoints) {
		if (currentPos.DistanceTo(transitionPoint) < D_OBJECT_SIZE) {
			LoadNextMap();
			break;
		}
	}

	// メニューを開く
	if (input->GetKeyDown(KEY_INPUT_TAB)) {
		isMenuVisible = true;
		// 初回は左メニューにフォーカス。どうぐに移動したらフォーカスONにする。
		isSubMenuVisible = false;
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

	// 落ちているアイテム
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

	// ===== メニュー描画 =====
	if (isMenuVisible) {
		const int rootX = 40;
		const int rootY = 40;
		const int menuW = 520;
		const int menuH = 320;

		// 背景
		DrawBox(rootX, rootY, rootX + menuW, rootY + menuH, GetColor(20, 20, 28), TRUE);
		DrawBox(rootX, rootY, rootX + menuW, rootY + menuH, GetColor(255, 255, 255), FALSE);

		// ステータスパネル（上）
		DrawStatusPanel(rootX + 10, rootY + 10, menuW - 20, 60);

		// 左：メニュー項目
		const int leftX = rootX + 10;
		const int leftY = rootY + 80;
		const int leftW = 160;
		const int leftH = menuH - 90;
		DrawBox(leftX, leftY, leftX + leftW, leftY + leftH, GetColor(30, 30, 40), TRUE);
		DrawBox(leftX, leftY, leftX + leftW, leftY + leftH, GetColor(200, 200, 200), FALSE);
		for (int i = 0; i < 4; ++i) {
			int y = leftY + 16 + i * 26;
			if (i == menuSelection)
				DrawString(leftX + 12, y, ("＞ " + std::string(menuItems[i])).c_str(), GetColor(255, 255, 0));
			else
				DrawString(leftX + 28, y, menuItems[i], GetColor(235, 235, 235));
		}

		// 右：どうぐリスト or 情報
		const int rightX = leftX + leftW + 10;
		const int rightY = leftY;
		const int rightW = menuW - (rightX - rootX) - 10;
		const int rightH = leftH;

		// 右ペイン：選択中のメニューに応じて即時表示（Enter不要）
		{
			std::string info;

			if (isSubMenuVisible) {
				// サブメニュー中
				if (menuSelection == 0 && !groupedItems.empty()) {
					// どうぐ：カーソル行のプレビューを常時表示
					int idx = clamp(subMenuSelection, 0, (int)groupedItems.size() - 1);
					info = BuildItemPreviewText(groupedItems[idx]);
				}
				// 「そうび」「つよさ」は Enter後のテキスト(subMenuText)を下で併記
			}
			else {
				// サブメニューを開いていない：左メニュー選択だけで右を即更新
				switch (menuSelection) {
				case 0: {
						  // どうぐ：テキストは出さない（プレビュー見出しを消す）
						 // リストはこのあと別処理で常時描画されるのでここは空でOK
						info.clear();
					
				} break;
				case 1: { // そうび：装備一覧を即表示
					PlayerData* pd = PlayerData::GetInstance();
					info = "装備一覧\n";
					info += " 武器: " + pd->GetEquippedName(EquipCategory::Weapon) + "\n";
					info += " 盾:   " + pd->GetEquippedName(EquipCategory::Shield) + "\n";
					info += " 防具: " + pd->GetEquippedName(EquipCategory::Armor) + "\n";
					info += " 頭:   " + pd->GetEquippedName(EquipCategory::Helmet);
				} break;
				case 2: { // つよさ：ステータスを即表示
					PlayerData* pd = PlayerData::GetInstance();
					info = "プレイヤー情報\n";
					info += " レベル: " + std::to_string(pd->GetLevel()) + "\n";
					info += " HP: " + std::to_string(pd->GetHp()) + " / " + std::to_string(pd->GetMaxHp()) + "\n";
					info += " 攻撃力: " + std::to_string(pd->GetAttack()) + "\n";
					info += " 防御力: " + std::to_string(pd->GetDefense());
				} break;
				case 3: // もどる
					info = "メニューを閉じます。";
					break;
				}
			}

			// 直前の操作結果メッセージがあれば、プレビューの下に併記
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

		// どうぐリスト：カーソルが「どうぐ」を指していれば常時表示（Enter不要で見える）
		if (menuSelection == 0) {
			if (groupedItems.empty()) {
				RebuildItemList(); // 必要時だけ構築
			}
			// 範囲補正
			if (!groupedItems.empty()) {
				subMenuSelection = clamp(subMenuSelection, 0, (int)groupedItems.size() - 1);
			}
			else {
				subMenuSelection = 0;
			}
			DrawItemListPanel(rightX, rightY + 60, rightW, rightH - 60);
			// まだEnterでサブメニューに入っていない時は、操作ヒントを薄く表示（任意）
			if (!isSubMenuVisible) {
				DrawString(rightX, rightY + rightH - 18, "Enterで選択 / 使用", GetColor(180, 180, 180));
			}
		}


		// 操作ヘルプ（下）
		DrawHelpPanel(rootX + 10, rootY + menuH - 28, menuW - 20, 20);
	}
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
	std::string txt = "Lv " + std::to_string(pd->GetLevel()) + "   HP " + std::to_string(pd->GetHp()) + "/" + std::to_string(pd->GetMaxHp()) + "   攻 " + std::to_string(pd->GetAttack()) + "   防 " + std::to_string(pd->GetDefense());
	DrawString(x + 12, y + 10, txt.c_str(), GetColor(240, 240, 240));
}

// どうぐリスト描画
void Map::DrawItemListPanel(int x, int y, int w, int h) {
	DrawBox(x, y, x + w, y + h, GetColor(26, 26, 32), TRUE);
	DrawBox(x, y, x + w, y + h, GetColor(140, 140, 140), FALSE);

	// 見出し
	DrawString(x + 8, y - 18, "所持品", GetColor(255, 255, 180));

	const int lineH = 20;
	int maxRows = std::min(VISIBLE_ROWS, (int)groupedItems.size());
	for (int i = 0; i < maxRows; ++i) {
		int idx = listScrollOffset + i;
		if (idx < 0 || idx >= (int)groupedItems.size())
			break;
		const Map::MenuEntry& e = groupedItems[idx];

		// カテゴリラベル
		std::string label;
		switch (e.category) {
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
			label = "[薬] ";
			break;
		}
		label += e.name;
		if (e.type == ItemType::Consumable && e.count > 1) {
			label += " (x" + std::to_string(e.count) + ")";
		}
		if (e.equipped)
			label = "★ " + label;

		// カーソル
		if (idx == subMenuSelection) {
			DrawString(x + 8, y + 6 + i * lineH, "＞", GetColor(255, 255, 0));
		}
		DrawString(x + 28, y + 6 + i * lineH, label.c_str(), GetColor(220, 255, 220));
	}

	// スクロールバー目安
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

// ★ カーソル位置のアイテム情報（プレビュー常時表示）
std::string Map::BuildItemPreviewText(const Map::MenuEntry& e) {
	// 代表IDから実体を引く（存在しなければ簡易情報）
	const auto& owned = PlayerData::GetInstance()->GetOwnedItems();
	auto it = owned.find(e.representativeId);

	// 共通ラベル
	auto catLabel = [](EquipCategory c) -> const char* {
		switch (c) {
		case EquipCategory::Weapon:
			return "武器";
		case EquipCategory::Shield:
			return "盾";
		case EquipCategory::Armor:
			return "鎧";
		case EquipCategory::Helmet:
			return "頭";
		default:
			return "消費";
		}
	};

	std::string s;
	return s;
}

// 終了
void Map::Finalize() {
	GameManager* obj = Singleton<GameManager>::GetInstance();
	obj->Finalize();
}

// 現在のシーンタイプ
eSceneType Map::GetNowSceneType() const { return eSceneType::eMap; }

// CSV 読み込み（※コリジョン/遷移は既存ロジック）
std::vector<std::vector<char>> Map::LoadStageMapCSV(std::string map_name) {
	std::ifstream ifs(map_name);
	if (ifs.fail())
		throw std::runtime_error(map_name + " が開けませんでした。");

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
			// '3' や '6' を衝突とみなす（元ロジックに合わせる）
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

// マップ描画
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

// 次マップ
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
		std::vector<std::string> lines{ "ここは安全地帯だ。", "旅の準備はできているかい？" };
		ncps.push_back(std::make_shared<NCP>(Vector2D(320.0f, 600.0f), "村の長老", lines));
		ncps.back()->SetImage("Resource/Images/peabird.png");
	}
	if (!isEncounterEnabled) {
		std::vector<std::string> lines{ "僕よっしー！！" };
		ncps.push_back(std::make_shared<NCP>(Vector2D(220.0f, 300.0f), "村人A", lines));
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
	return row < 0 || row >= (int)collisionMap.size() || col < 0 || col >= (int)collisionMap[row].size() || collisionMap[row][col];
}

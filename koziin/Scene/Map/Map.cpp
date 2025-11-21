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
// 所持アイテムのグループ一覧を作り直す
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
// PlayerData::ownedItems から、
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
	std::sort(
		groupedItems.begin(), groupedItems.end(),
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
Map::Map()
	: encounterStepCounter(0), lastPlayerPos(Vector2D(0, 0)), isFadingIn(false), fadeAlpha(255.0f) {
	srand(static_cast<unsigned int>(time(nullptr)));
}
Map::~Map() {}

// 初期化
void Map::Initialize() {
	// ★ 今のステージ index に対応する CSV を読み込む
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

	// NCP
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

		// 右側にフォーカスがあるか（どうぐ or そうびUI）
		bool listActive = isSubMenuVisible;

		// ---------- 左メニュー操作（フォーカス無し） ----------
		if (!listActive) {
			int before = menuSelection;

			if (input->GetKeyDown(KEY_INPUT_DOWN))
				menuSelection = (menuSelection + 1) % menuItemCount;
			if (input->GetKeyDown(KEY_INPUT_UP))
				menuSelection = (menuSelection - 1 + menuItemCount) % menuItemCount;

			if (menuSelection != before) {
				subMenuText.clear();
				if (menuSelection == 0) {
					if (groupedItems.empty()) {
						RebuildItemList();
						subMenuSelection = 0;
						listScrollOffset = 0;
					}
					isSubMenuVisible = false;
					rightMode = RightMode::None;
				}
				else {
					isSubMenuVisible = false;
					rightMode = RightMode::None;
				}
			}

			// Enter：どうぐ/そうび/つよさ/もどる
			if (input->GetKeyDown(KEY_INPUT_RETURN)) {
				switch (menuSelection) {
				case 0: { // どうぐ：右リストに入る
					if (groupedItems.empty()) {
						RebuildItemList();
						subMenuSelection = 0;
						listScrollOffset = 0;
					}
					rightMode = RightMode::Items;
					isSubMenuVisible = true;
				} break;

				case 1: { // そうび：右に部位一覧
					rightMode = RightMode::EquipSlots;
					equipSlotSelection = 0;
					isSubMenuVisible = true;
				} break;

				case 2: { // つよさ：即時表示のみ（右に入らない）
					rightMode = RightMode::None;
				} break;

				case 3: { // もどる：メニューを閉じる
					isMenuVisible = false;
					isSubMenuVisible = false;
					rightMode = RightMode::None;
					groupedItems.clear();
					subMenuText.clear();
					return eSceneType::eMap;
				} break;
				}
			}

			return eSceneType::eMap; // メニュー表示継続
		}

		// ---------- 右：どうぐ（Items） ----------
		if (rightMode == RightMode::Items && menuSelection == 0 && !groupedItems.empty()) {

			// ↓：ページ最下段で次ページ先頭へ。末尾では先頭ページ先頭にラップ。
			if (input->GetKeyDown(KEY_INPUT_DOWN)) {
				int rel = subMenuSelection - listScrollOffset; // 0..VISIBLE_ROWS-1
				if (subMenuSelection + 1 < (int)groupedItems.size()) {
					if (rel == VISIBLE_ROWS - 1) {
						int nextOffset = listScrollOffset + VISIBLE_ROWS;
						int lastOffset = std::max(0, (int)groupedItems.size() - VISIBLE_ROWS);
						listScrollOffset = std::min(nextOffset, lastOffset);
						subMenuSelection = listScrollOffset; // 次ページ先頭
					}
					else {
						subMenuSelection++;
					}
				}
				else {
					// 末尾 → 先頭ページ先頭へラップ
					listScrollOffset = 0;
					subMenuSelection = 0;
				}
			}

			// ↑：ページ先頭で前ページ末尾へ。先頭では最後ページ末尾にラップ。
			if (input->GetKeyDown(KEY_INPUT_UP)) {
				int rel = subMenuSelection - listScrollOffset;
				if (subMenuSelection > 0) {
					if (rel == 0) {
						int prevOffset = std::max(0, listScrollOffset - VISIBLE_ROWS);
						listScrollOffset = prevOffset;
						subMenuSelection = listScrollOffset + VISIBLE_ROWS - 1;
						if (subMenuSelection >= (int)groupedItems.size())
							subMenuSelection = (int)groupedItems.size() - 1;
					}
					else {
						subMenuSelection--;
					}
				}
				else {
					// 先頭 → 最後ページ末尾へラップ
					int lastOffset = ((int)groupedItems.size() - 1) / VISIBLE_ROWS * VISIBLE_ROWS;
					listScrollOffset = std::max(0, lastOffset);
					subMenuSelection = (int)groupedItems.size() - 1;
				}
			}

			// Enter：装備 / 使用（トグル）
			if (input->GetKeyDown(KEY_INPUT_RETURN)) {
				const MenuEntry& entry = groupedItems[subMenuSelection];
				const auto& owned = PlayerData::GetInstance()->GetOwnedItems();
				auto it = owned.find(entry.representativeId);

				if (it != owned.end()) {
					const Item& selected = it->second;
					auto* pd = PlayerData::GetInstance();

					if (entry.type == ItemType::Equipment && selected.GetCategory() != EquipCategory::None) {
						EquipCategory cat = selected.GetCategory();
						int cur = pd->GetEquippedId(cat);

						if (cur == entry.representativeId) {
							pd->Unequip(cat);
							subMenuText = "外しました： " + selected.GetName();
						}
						else {
							pd->EquipItem(cat, entry.representativeId);
							subMenuText = "装備しました： " + selected.GetName();
						}
					}
					else if (entry.type == ItemType::Consumable) {
						const std::string name = selected.GetName();
						const int heal = selected.GetHealAmount();
						if (pd->UseItem(entry.representativeId)) {
							subMenuText = "使用しました： " + name + "（+" + std::to_string(heal) + " HP）";
						}
						else {
							subMenuText = "使用できません： " + name;
						}
					}
					else {
						subMenuText = "このアイテムは使用できません： " + selected.GetName();
					}
				}

				// 反映（個数減少や★更新）
				RebuildItemList();

				// リストが空なら左へ戻す
				if (groupedItems.empty()) {
					isSubMenuVisible = false;
					rightMode = RightMode::None;
					subMenuSelection = 0;
					listScrollOffset = 0;
				}
				else {
					// 選択位置/スクロール補正
					subMenuSelection = std::min(subMenuSelection, (int)groupedItems.size() - 1);
					if (subMenuSelection < listScrollOffset)
						listScrollOffset = subMenuSelection;
					if (subMenuSelection >= listScrollOffset + VISIBLE_ROWS)
						listScrollOffset = std::max(0, subMenuSelection - (VISIBLE_ROWS - 1));
				}
			}

			// Space / Esc：左メニューへ戻る
			if (input->GetKeyDown(KEY_INPUT_SPACE) || input->GetKeyDown(KEY_INPUT_ESCAPE)) {
				isSubMenuVisible = false;
				rightMode = RightMode::None;
			}

			return eSceneType::eMap;
		}

		// ---------- 右：どうぐ（Items：空のときの戻る処理） ----------
		else if (rightMode == RightMode::Items && menuSelection == 0 && groupedItems.empty()) {
			if (input->GetKeyDown(KEY_INPUT_SPACE) ||
				input->GetKeyDown(KEY_INPUT_ESCAPE) ||
				input->GetKeyDown(KEY_INPUT_TAB)) {
				isSubMenuVisible = false;
				rightMode = RightMode::None;
				subMenuText.clear();
			}
			return eSceneType::eMap;
		}

		// ---------- 右：そうび（EquipSlots：部位一覧） ----------
		if (rightMode == RightMode::EquipSlots && menuSelection == 1) {
			if (input->GetKeyDown(KEY_INPUT_DOWN))
				equipSlotSelection = (equipSlotSelection + 1) % 4;
			if (input->GetKeyDown(KEY_INPUT_UP))
				equipSlotSelection = (equipSlotSelection + 3) % 4;

			if (input->GetKeyDown(KEY_INPUT_RETURN)) {
				EquipCategory cat =
					(equipSlotSelection == 0)	? EquipCategory::Weapon
					: (equipSlotSelection == 1) ? EquipCategory::Shield
					: (equipSlotSelection == 2) ? EquipCategory::Armor
												: EquipCategory::Helmet;

				BuildEquipFilteredList(cat);
				equipItemSelection = 0;
				equipItemScrollOffset = 0;
				rightMode = RightMode::EquipItemList; // 候補へ
			}
			// Space/Esc：左へ戻る
			if (input->GetKeyDown(KEY_INPUT_SPACE) || input->GetKeyDown(KEY_INPUT_ESCAPE)) {
				isSubMenuVisible = false;
				rightMode = RightMode::None;
			}
			return eSceneType::eMap;
		}

		// ---------- 右：そうび（EquipItemList：候補一覧） ----------
		if (rightMode == RightMode::EquipItemList && menuSelection == 1) {

			// ★ 空のとき：Space/Esc/Tab/Enter のどれでも部位一覧へ戻す
			if (equipFiltered.empty()) {
				if (input->GetKeyDown(KEY_INPUT_SPACE) ||
					input->GetKeyDown(KEY_INPUT_ESCAPE) ||
					input->GetKeyDown(KEY_INPUT_TAB)) {
					rightMode = RightMode::EquipSlots;
				}
				return eSceneType::eMap;
			}

			// ↓：ページ最下段で次ページ先頭へ。末尾で先頭ページへラップ。
			if (input->GetKeyDown(KEY_INPUT_DOWN)) {
				int rel = equipItemSelection - equipItemScrollOffset;
				if (equipItemSelection + 1 < (int)equipFiltered.size()) {
					if (rel == VISIBLE_ROWS - 1) {
						int nextOffset = equipItemScrollOffset + VISIBLE_ROWS;
						int lastOffset = std::max(0, (int)equipFiltered.size() - VISIBLE_ROWS);
						equipItemScrollOffset = std::min(nextOffset, lastOffset);
						equipItemSelection = equipItemScrollOffset; // 次ページ先頭
					}
					else {
						equipItemSelection++;
					}
				}
				else {
					equipItemScrollOffset = 0;
					equipItemSelection = 0;
				}
			}

			// ↑：ページ先頭で前ページ末尾へ。先頭で最後ページ末尾にラップ。
			if (input->GetKeyDown(KEY_INPUT_UP)) {
				int rel = equipItemSelection - equipItemScrollOffset;
				if (equipItemSelection > 0) {
					if (rel == 0) {
						int prevOffset = std::max(0, equipItemScrollOffset - VISIBLE_ROWS);
						equipItemScrollOffset = prevOffset;
						equipItemSelection = equipItemScrollOffset + VISIBLE_ROWS - 1;
						if (equipItemSelection >= (int)equipFiltered.size())
							equipItemSelection = (int)equipFiltered.size() - 1;
					}
					else {
						equipItemSelection--;
					}
				}
				else {
					int lastOffset = ((int)equipFiltered.size() - 1) / VISIBLE_ROWS * VISIBLE_ROWS;
					equipItemScrollOffset = std::max(0, lastOffset);
					equipItemSelection = (int)equipFiltered.size() - 1;
				}
			}

			// Enter：装備 / 外す
			if (input->GetKeyDown(KEY_INPUT_RETURN)) {
				const MenuEntry& entry = equipFiltered[equipItemSelection];
				auto* pd = PlayerData::GetInstance();
				EquipCategory cat = entry.category;

				int cur = pd->GetEquippedId(cat);
				if (cur == entry.representativeId) {
					pd->Unequip(cat);
					subMenuText = "外しました： " + entry.name;
				}
				else {
					pd->EquipItem(cat, entry.representativeId);
					subMenuText = "装備しました： " + entry.name;
				}

				// 直近操作IDを保持 → 再構築後に同位置へ戻す
				const int keepId = entry.representativeId;
				RebuildItemList();
				BuildEquipFilteredList(cat);

				if (equipFiltered.empty()) {
					rightMode = RightMode::EquipSlots;
				}
				else {
					int newIndex = std::min(equipItemSelection, (int)equipFiltered.size() - 1);
					for (int i = 0; i < (int)equipFiltered.size(); ++i) {
						if (equipFiltered[i].representativeId == keepId) {
							newIndex = i;
							break;
						}
					}
					equipItemSelection = newIndex;
					if (equipItemSelection < equipItemScrollOffset)
						equipItemScrollOffset = equipItemSelection;
					if (equipItemSelection >= equipItemScrollOffset + VISIBLE_ROWS)
						equipItemScrollOffset = std::max(0, equipItemSelection - (VISIBLE_ROWS - 1));
				}
			}

			// ★ Space/Esc/Tab：部位一覧に戻る
			if (input->GetKeyDown(KEY_INPUT_SPACE) ||
				input->GetKeyDown(KEY_INPUT_ESCAPE) ||
				input->GetKeyDown(KEY_INPUT_TAB)) {
				rightMode = RightMode::EquipSlots;
			}

			return eSceneType::eMap;
		}
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

	// メニュー復帰（今はフラグだけ・必要なら使う）
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

	// 落ちているアイテム
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

		// ★ 右にフォーカスがある間は左の矢印を消す（どうぐ/そうび共通）
		const bool rightFocused = isSubMenuVisible;

		for (int i = 0; i < 4; ++i) {
			int y = leftY + 16 + i * 26;
			const bool showArrow = (i == menuSelection) && !rightFocused;
			if (showArrow) {
				DrawString(
					leftX + 12,
					y,
					("＞ " + std::string(menuItems[i])).c_str(),
					GetColor(255, 255, 0));
			}
			else {
				int col = (i == menuSelection) ? 255 : 235;
				DrawString(leftX + 28, y, menuItems[i], GetColor(col, col, col));
			}
		}

		// 右：どうぐリスト or 情報 or そうびUI
		const int rightX = leftX + leftW + 10;
		const int rightY = leftY;
		const int rightW = menuW - (rightX - rootX) - 10;
		const int rightH = leftH;

		if (menuSelection == 1) {
			DrawBox(rightX, rightY, rightX + rightW, rightY + rightH, GetColor(26, 26, 32), TRUE);	   // 背景
			DrawBox(rightX, rightY, rightX + rightW, rightY + rightH, GetColor(200, 200, 200), FALSE); // 枠線を明るく
		}

		// 右ペイン：選択中のメニューに応じて即時表示（Enter不要）
		{
			std::string info;

			if (isSubMenuVisible) {
				// サブメニュー中
				if (menuSelection == 0 && !groupedItems.empty() && rightMode == RightMode::Items) {
					// どうぐ：カーソル行のプレビューを常時表示
					int idx = clamp(subMenuSelection, 0, (int)groupedItems.size() - 1);
					info = BuildItemPreviewText(groupedItems[idx]);
				}
				// 「そうび」のUIは別描画（下で処理）
			}
			else {
				// サブメニューを開いていない：左メニュー選択だけで右を即更新
				switch (menuSelection) {
				case 0: { // どうぐ：テキストは出さない（プレビュー見出しを消す）
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
					info += " MP: " + std::to_string(pd->GetMp()) + " / " + std::to_string(pd->GetMaxMp()) + "\n";
					info += " 攻撃力: " + std::to_string(pd->GetAttack()) + "\n";
					info += " 防御力: " + std::to_string(pd->GetDefense());
				} break;
				case 3: // もどる
					info = "メニューを閉じます。";
					break;
				}
			}

			// 直前の操作結果メッセージがあれば、プレビューの下に併記
			// ★ どうぐ画面のときだけ併記するよう制限
			if (menuSelection == 0 && !subMenuText.empty()) {
				if (!info.empty())
					info += "\n----------------\n";
				info += subMenuText;
			}

			if (!info.empty()) {
				std::istringstream iss(info);
				std::string line;
				int lineNum = 0;

				// 右ペインの内側に少し余白(左:12px, 上:10px)を取って描画
				const int marginLeft = 12;
				const int marginTop = 10;
				const int lineH = 20;

				while (std::getline(iss, line)) {
					DrawString(
						rightX + marginLeft,
						rightY + marginTop + lineNum * lineH,
						line.c_str(),
						GetColor(200, 255, 200));
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
			// どうぐリスト描画：左メニュー枠と高さを揃える
			DrawItemListPanel(rightX, leftY, rightW, leftH);

			// まだEnterでサブメニューに入っていない時は、操作ヒントを薄く表示（任意）
			if (!isSubMenuVisible) {
				DrawString(
					rightX,
					rightY + rightH - 18,
					"Enterで選択 / 使用",
					GetColor(180, 180, 180));
			}
		}

		// そうび：右フォーカスのときは専用描画
		if (menuSelection == 1 && isSubMenuVisible) {
			if (rightMode == RightMode::EquipSlots) {
				DrawEquipSlotsPanel(rightX, rightY, rightW, rightH);
			}
			else if (rightMode == RightMode::EquipItemList) {
				DrawEquipItemListPanel(rightX, rightY, rightW, rightH);
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
	std::string txt =
		"勇者よっしー　Lv " + std::to_string(pd->GetLevel()) + "   /HP " + std::to_string(pd->GetHp());
	DrawString(x + 12, y + 10, txt.c_str(), GetColor(240, 240, 240));
}

// メニュー画面どうぐ
void Map::DrawItemListPanel(int x, int y, int w, int h) {
	// 外枠
	DrawBox(x, y, x + w, y + h, GetColor(26, 26, 32), TRUE);
	DrawBox(x, y, x + w, y + h, GetColor(140, 140, 140), FALSE);

	// 枠内レイアウト
	const int headerH = 24;			 // 見出しエリア高さ（枠内）
	const int lineH = 20;			 // 1 行の高さ
	const int padX = 8;				 // 左余白
	const int listTop = y + headerH; // リスト描画開始 Y（枠内）
	const int listH = h - headerH;	 // リスト領域の高さ（枠内）
	const int rowsCap = std::max(1, listH / lineH);
	const int maxRows = std::min(std::min(VISIBLE_ROWS, rowsCap), (int)groupedItems.size());

	// 見出し（枠内に収める）
	DrawString(x + padX, y + 4, "所持品", GetColor(255, 255, 180));

	// 行描画
	for (int i = 0; i < maxRows; ++i) {
		int idx = listScrollOffset + i;
		if (idx < 0 || idx >= (int)groupedItems.size())
			break;
		const Map::MenuEntry& e = groupedItems[idx];

		// 表示文字列
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

		const bool rightFocused = isSubMenuVisible && (rightMode == RightMode::Items);
		int yLine = listTop + 6 + i * lineH;
		if (idx == subMenuSelection && rightFocused) {
			DrawString(x + padX, yLine, "＞", GetColor(255, 255, 0));
		}
		DrawString(x + padX + 20, yLine, label.c_str(), GetColor(220, 255, 220));
	}

	// スクロールバー（枠内で完結）
	if ((int)groupedItems.size() > maxRows) {
		int trackX = x + w - 8;
		int trackY = listTop;
		int trackH = listH;
		DrawBox(trackX, trackY, trackX + 4, trackY + trackH, GetColor(60, 60, 60), TRUE);

		int scrollable = std::max(1, (int)groupedItems.size() - maxRows);
		float posRatio = (float)listScrollOffset / (float)scrollable;
		int barH = std::max(8, (int)(trackH * ((float)maxRows / (float)groupedItems.size())));
		int barY = trackY + (int)((trackH - barH) * posRatio);
		DrawBox(trackX, barY, trackX + 4, barY + barH, GetColor(200, 200, 200), TRUE);
	}

	// ページ数表示（右下・枠内）
	const int rowsPerPage = VISIBLE_ROWS; // 1ページの行数（= 描画行数）
	const int n = (int)groupedItems.size();
	const int totalPages = (n == 0) ? 1 : ((n + rowsPerPage - 1) / rowsPerPage);
	const int currentPage = (n == 0) ? 1 : ((listScrollOffset / rowsPerPage) + 1);

	std::string pg = std::to_string(currentPage) + " / " + std::to_string(totalPages);

	// 見出しと同じY座標の右端寄せ。数値が被る場合は -70 等で微調整してください。
	DrawString(x + w - 70, y + 190, pg.c_str(), GetColor(220, 220, 220));
}

// ★ カーソル位置のアイテム情報（プレビュー常時表示）
std::string Map::BuildItemPreviewText(const Map::MenuEntry& e) {
	const auto& owned = PlayerData::GetInstance()->GetOwnedItems();
	auto it = owned.find(e.representativeId);

	if (it == owned.end()) {
		return "アイテム情報が見つかりません。";
	}

	const Item& item = it->second;

	// カテゴリ名
	auto catLabel = [](EquipCategory c) -> const char* {
		switch (c) {
		case EquipCategory::Weapon:
			return "武器";
		case EquipCategory::Shield:
			return "盾";
		case EquipCategory::Armor:
			return "防具";
		case EquipCategory::Helmet:
			return "頭";
		default:
			return "消費";
		}
	};

	std::string s;
	s += "名前：" + item.GetName() + "\n";
	s += "種類：" + std::string(catLabel(e.category)) + "\n";

	if (e.type == ItemType::Consumable) {
		s += "回復量：" + std::to_string(item.GetHealAmount()) + " HP\n";
		s += "所持数：" + std::to_string(e.count) + "\n";
	}
	else {
		// 装備の詳細ステータスを出したければ、Item に応じてここで追記
		if (e.equipped) {
			s += "状態：装備中\n";
		}
		else {
			s += "状態：未装備\n";
		}
	}

	return s;
}

// 終了
void Map::Finalize() {
	GameManager* obj = Singleton<GameManager>::GetInstance();
	obj->Finalize();
}

// 現在のシーンタイプ
eSceneType Map::GetNowSceneType() const {
	return eSceneType::eMap;
}

// ============================
// CSV 読み込み（Book1.csv 対応）
// ============================
std::vector<std::vector<char>> Map::LoadStageMapCSV(std::string map_name) {
	std::ifstream ifs(map_name);
	if (ifs.fail())
		throw std::runtime_error(map_name + " が開けませんでした。");

	std::vector<std::vector<char>> data;
	collisionMap.clear();
	transitionPoints.clear();

	std::string line;
	int rowIdx = 0;

	// ※ 現状仕様維持：stage2 だけエンカウント無効
	isEncounterEnabled = (map_name != "Resource/stage2.csv");

	while (std::getline(ifs, line)) {
		std::vector<char> row;
		std::vector<bool> collisionRow;
		std::stringstream ss(line);
		std::string cell;
		int colIdx = 0;

		while (std::getline(ss, cell, ',')) {

			// ---- 空欄・空白対策 ----
			// 末尾の改行・空白削除
			while (!cell.empty() &&
				   (cell.back() == ' ' || cell.back() == '\t' ||
					   cell.back() == '\r' || cell.back() == '\n')) {
				cell.pop_back();
			}

			// 先頭の空白スキップ
			size_t pos = 0;
			while (pos < cell.size() && (cell[pos] == ' ' || cell[pos] == '\t')) {
				++pos;
			}

			char c;
			if (pos >= cell.size()) {
				// 完全に空欄 → '0' とみなす
				c = '0';
			}
			else {
				c = cell[pos];
				// 数字以外が来たら '0' とみなす
				if (c < '0' || c > '9') {
					c = '0';
				}
			}

			row.push_back(c);

			// 衝突フラグ：'3' と '6' を壁扱い
			collisionRow.push_back(c == '3' || c == '6');

			// マップ遷移ポイント（'4'）
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

// ============================
// マップ描画（防御付き）
// ============================
void Map::DrawStageMap() {
	ResourceManager* rm = ResourceManager::GetInstance();

	for (int i = 0; i < MAP_SQUARE_Y; i++) {
		// 行が足りない場合は描画スキップ
		if (i < 0 || i >= (int)mapdata.size())
			continue;

		for (int j = 0; j < MAP_SQUARE_X; j++) {
			// 列が足りない場合もスキップ
			if (j < 0 || j >= (int)mapdata[i].size())
				continue;

			char c = mapdata[i][j];

			// '0'～'9' → 0～9 のタイル番号に変換
			int tileIndex = c - '0';
			if (tileIndex < 0 || tileIndex > 9) {
				tileIndex = 0; // 異常値は 0 とみなす
			}

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
	return row < 0 ||
		   row >= (int)collisionMap.size() ||
		   col < 0 ||
		   col >= (int)collisionMap[row].size() ||
		   collisionMap[row][col];
}

/* =========================
   そうび：ヘルパ実装
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
	// ★装備中を先頭にしない。名前順のみ。
	std::sort(
		equipFiltered.begin(), equipFiltered.end(),
		[](const MenuEntry& a, const MenuEntry& b) {
			return a.name < b.name;
		});
}

// メニュー画面そうび
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
		{ "防具", EquipCategory::Armor },
		{ "頭", EquipCategory::Helmet }
	};

	DrawString(x + 8, y - 18, " ", GetColor(255, 255, 180));
	for (int i = 0; i < 4; ++i) {
		int yy = y + 6 + i * 24;
		if (i == equipSlotSelection)
			DrawString(x + 8, yy, "＞", GetColor(255, 255, 0));
		std::string line =
			std::string(rows[i].label) + "： " + pd->GetEquippedName(rows[i].cat);
		DrawString(x + 28, yy, line.c_str(), GetColor(200, 255, 200));
	}
	DrawString(x + 8, y + h - 18, " ", GetColor(180, 180, 180));
}

// 装備一覧ページ
void Map::DrawEquipItemListPanel(int x, int y, int w, int h) {
	DrawBox(x, y, x + w, y + h, GetColor(26, 26, 32), TRUE);
	DrawBox(x, y, x + w, y + h, GetColor(140, 140, 140), FALSE);

	DrawString(x + 8, y + 3, "候補", GetColor(255, 255, 180));
	{
		int n = (int)equipFiltered.size();
		int totalPages = (n == 0) ? 1 : ((n + VISIBLE_ROWS - 1) / VISIBLE_ROWS);
		int currentPage = (n == 0) ? 1 : ((equipItemScrollOffset / VISIBLE_ROWS) + 1);
		// ページ数
		std::string pg = std::to_string(currentPage) + " / " + std::to_string(totalPages);
		DrawString(x + w - 80, y + 190, pg.c_str(), GetColor(220, 220, 220));
	}

	const int lineH = 20;
	const int topPad = 6;
	const int botPad = 8;
	const int fontH = 16;

	int capacity = ((h - topPad - botPad - fontH) / lineH) + 1;
	capacity = std::max(1, capacity);
	int maxRows = std::min({ VISIBLE_ROWS, capacity, (int)equipFiltered.size() });

	const int offsetY = 30;

	for (int i = 0; i < maxRows; ++i) {
		int idx = equipItemScrollOffset + i;
		if (idx < 0 || idx >= (int)equipFiltered.size())
			break;
		const auto& e = equipFiltered[idx];

		std::string label = e.name;
		if (e.equipped)
			label = "★ " + label;

		if (idx == equipItemSelection)
			DrawString(
				x + 8,
				y + offsetY + 6 + i * lineH,
				"＞",
				GetColor(255, 255, 0));

		DrawString(
			x + 28,
			y + offsetY + 6 + i * lineH,
			label.c_str(),
			GetColor(220, 255, 220));
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

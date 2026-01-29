#pragma once

#include "../SceneBase.h"
#include "../../Utility/Vector2D.h"
#include "../../Object/Charactor/Player/Player.h"
#include "../../Object/NPC/NPC.h"
#include <memory>
#include <string>
#include <vector>
#include <random>
#include "../../Object/Item/Item.h"

class Map : public SceneBase {
private:
	// マップ遷移ポイント
	std::vector<Vector2D> transitionPoints;

	// 衝突判定マップ
	std::vector<std::vector<bool>> collisionMap;

	// NCP 群
	std::vector<std::shared_ptr<NCP>> ncps;

	// ステージ
	// ★ 必要に応じてファイルを増やす
	std::vector<std::string> stageFiles = {
		"Resource/Book1.csv",
		"Resource/Book2.csv",
	};
	int currentStageIndex = 0;

	// エンカウント
	int encounterStepCounter = 0;
	Vector2D lastPlayerPos;

	// ===== メニュー関連 =====
	bool isMenuVisible = false;
	int menuSelection = 0;
	static constexpr int menuItemCount = 4;
	const char* menuItems[menuItemCount] = { "どうぐ", "そうび", "つよさ", "もどる" };

	// フォーカスON=右リスト操作中 / OFF=左メニュー操作中
	bool isSubMenuVisible = false; // フォーカス管理に使用
	std::string subMenuText;	   // 結果表示テキスト

	std::vector<std::shared_ptr<Item>> items;

	// どうぐリスト（グループ表示用）
	struct MenuEntry {
		int representativeId;
		std::string name;
		EquipCategory category; // Consumable の場合は None
		ItemType type;			// Equipment / Consumable
		int count;				// 個数（装備は1）
		bool equipped;			// 装備中（装備品のみ）
	};
	std::vector<MenuEntry> groupedItems;
	int subMenuSelection = 0;
	int listScrollOffset = 0;
	static constexpr int VISIBLE_ROWS = 8;

	void RebuildItemList();
	void RebuildItemListGrouped();

	// UI描画
	void DrawStatusPanel(int x, int y, int w, int h);
	void DrawHelpPanel(int x, int y, int w, int h);
	void DrawItemListPanel(int x, int y, int w, int h);
	std::string BuildItemPreviewText(const MenuEntry& e);

	// サウンド


	int BGM = -1;
	int soubi_Sound = -1;
	int sentaku_Sound = -1;
	int erabu_Sound = -1;
	int modoru_Sound = -1;


	// フェード
	bool isFadingIn = false;
	float fadeAlpha = 255.0f;
	const float fadeSpeed = 150.0f;
	void StartFadeIn();
	void UpdateFadeIn(float delta_second);
	void DrawFadeIn();

	template <typename T>
	T clamp(T v, T lo, T hi) {
		if (v < lo)
			return lo;
		if (v > hi)
			return hi;
		return v;
	}

	bool isFirstSpawn = true;
	bool isEncounterEnabled = true;
	float encounterCooldown = 2.0f;
	float encounterCooldownTimer = 0.0f;
	bool isAfterBattle = false;

	// 既存のメニュー系メンバの近くに追記
	enum class RightMode {
		None,
		Items,
		EquipSlots,
		EquipItemList
	};

	RightMode rightMode = RightMode::None;

	// そうび用
	int equipSlotSelection = 0;			  // 0:武器 1:盾 2:防具 3:頭
	std::vector<MenuEntry> equipFiltered; // 選んだ部位の候補
	int equipItemSelection = 0;			  // 候補リスト用カーソル
	int equipItemScrollOffset = 0;		  // 候補リスト用スクロール

	// ヘルパ
	void BuildEquipFilteredList(EquipCategory cat);
	void DrawEquipSlotsPanel(int x, int y, int w, int h);
	void DrawEquipItemListPanel(int x, int y, int w, int h);

public:
	Map();
	~Map();

	virtual void Initialize() override;
	virtual eSceneType Update(float delta_second) override;
	virtual void Draw() override;
	virtual void Finalize() override;
	virtual eSceneType GetNowSceneType() const override;

	std::vector<std::vector<char>> LoadStageMapCSV(std::string map_name);
	void DrawStageMap();
	void LoadNextMap();
	bool IsCollision(float x, float y);

	int MapImage;
	Vector2D generate_location;
	class Player* player;
	std::vector<std::vector<char>> mapdata;

private:
	int savedHp{};
	int savedAttack{};
	int savedDefense{};
};

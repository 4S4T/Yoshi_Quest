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
	// マップ遷移ポイントのリスト
	std::vector<Vector2D> transitionPoints;

	// 衝突判定マップ（true: 衝突あり, false: 衝突なし）
	std::vector<std::vector<bool>> collisionMap;

	// ステージ2専用 NCP 群
	std::vector<std::shared_ptr<NCP>> ncps;


	// ステージデータファイルのリスト
	std::vector<std::string> stageFiles = {
		"Resource/stage1.csv",
		"Resource/stage2.csv",
		"Resource/stage3.csv"
	};

	// 現在のステージインデックス
	int currentStageIndex = 0;

	// カメラのオフセット（プレイヤーの位置に応じてスクロール）
	Vector2D cameraOffset;
	const float SCREEN_WIDTH = 800.0f;
	const float SCREEN_HEIGHT = 600.0f;

private:
	int encounterStepCounter = 0;		   // プレイヤーの移動ステップ数
	Vector2D lastPlayerPos;				   // プレイヤーの最後の位置
	const int encounterStepThreshold = 10; // 10歩ごとにエンカウント判定

	bool isMenuVisible = false; // メニュー表示中フラグ
	int menuSelection = 0;		// 現在選択中のメニュー項目
	const char* menuItems[4] = { "どうぐ", "そうび", "つよさ", "もどる" };
	const int menuItemCount = 4;

	bool isSubMenuVisible = false; // サブメニュー表示中フラグ
	std::string subMenuText = "";  // 表示する内容

	std::vector<std::shared_ptr<Item>> items;	 // マップ上に配置されるアイテム
	//std::vector<std::string> collectedItemNames; // プレイヤーが取得したアイテム名一覧

	// 「どうぐ」一覧の選択制御（追加）
	int subMenuSelection = 0;		 // 一覧内カーソル
	std::vector<int> subMenuItemIds; // 一覧に並べる ItemID
	bool isItemListActive = false;	 // 一覧操作中フラグ
									
	void RebuildItemList(); // どうぐ一覧の再構築（消費した直後などに呼ぶ）



	std::mt19937 randomEngine;								  // 乱数生成器
	std::uniform_int_distribution<int> distribution{ 0, 99 }; // 0～99 の乱数


	// 初回マップ遷移時のフェードアウト制御
	bool hasInitialFadeOut = false;
	bool initialFadeOutCompleted = false;

	// フェードイン・フェードアウト用変数
	bool isFadingIn = false;
	bool isFadingOut = false;
	bool hasTransitioned = false;			 // 遷移完了フラグ
	eSceneType nextScene = eSceneType::eMap; // 次のシーンタイプ
	float fadeAlpha = 255.0f;				 // フェードの透明度
	const float fadeSpeed = 150.0f;			 // フェード速度

	float encounterCooldown = 2.0f; // クールタイム（秒）
	float encounterCooldownTimer = 0.0f;
	bool isAfterBattle = false;


	// 初回スポーン判定（最初のマップロード時に使用）
	bool isFirstSpawn = true;

	// ランダムエンカウントの有効・無効
	bool isEncounterEnabled = true;

	// フェードイン開始処理
	void StartFadeIn();
	// フェードインの更新処理
	void UpdateFadeIn(float delta_second);
	// フェードイン描画処理
	void DrawFadeIn();

	// 値を指定範囲内に収めるヘルパー関数
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

	// 初期化処理
	virtual void Initialize() override;

	int MapImage;				// マップ画像
	Vector2D generate_location; // プレイヤーの初期位置
	class Player* player;		// プレイヤーオブジェクト

	// 更新処理（フェード、エンカウント、遷移判定などを含む）
	virtual eSceneType Update(float delta_second) override;

	// 描画処理（マップ、フェード効果など）
	virtual void Draw() override;

	// 終了処理（メモリ解放など）
	virtual void Finalize() override;

	// 現在のシーンタイプを取得
	virtual eSceneType GetNowSceneType() const override;

	// マップデータのロード処理（CSVファイルから）
	std::vector<std::vector<char>> LoadStageMapCSV(std::string map_name);

	// マップ描画処理
	void DrawStageMap();

	// 次のマップをロード
	void LoadNextMap();

	// 衝突判定（指定座標に移動可能か判定）
	bool IsCollision(float x, float y);

	// 現在のマップデータ
	std::vector<std::vector<char>> mapdata;

private:
	int savedHp;
	int savedAttack;
	int savedDefense;
};

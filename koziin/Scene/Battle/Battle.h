#pragma once

#include "../SceneBase.h"
#include "../../Utility/Vector2D.h"
#include "../../Utility/PlayerData.h"
#include <vector>
#include <queue>
#include <string>
#include <functional>

#include "../../Object/Enemy/EnemyType/Slime.h"
#include "../../Object/Enemy/EnemyType/peabird.h"
#include "../../Object/Enemy/EnemyType/Taurus.h"
#include "../Map/Map.h"

// 戦闘ステート
enum class BattleState {
	PlayerCommand,
	AttackSelect,
	MagicMenu, // じゅもんメニュー（拡張用）
	ItemMenu,  // どうぐメニュー（拡張用）
	EnemyTurn, // 敵はキューで1体ずつ行動
	Message,   // 手動送りメッセージ
	Result,	   // 勝利リザルト（タイプライタ）
	Defeat	   // ★敗北リザルト（タイプライタ）
};

// 敵ハンドル
struct EnemyHandle {
	std::string name;
	std::string displayName;
	int expValue = 0;
	bool defeated = false;

	std::function<int()> getHp;
	std::function<int()> getAtk;
	std::function<int()> getDef;
	std::function<void(int)> applyDamage;
	std::function<void(float)> setBlink;
	std::function<void(bool)> setVisible;

	float x = 0.0f;
	float y = 0.0f;
	int maxHp = 1;

	int dispHp = 1; // 視覚用HP
};

// 遭遇テーブル
struct EncounterEntry {
	int minCount = 1;
	int maxCount = 3;
	std::vector<int> enemyTypeWeights; // 0:peabird, 1:Taurus ...
};

// ダメージポップ
struct DamagePopup {
	float timer;
	int amount;
	float x, y;
};

// 被弾スプラッシュ（赤い円）
struct HitSplash {
	float timer = 0.0f;
	float duration = 0.3f;
	float x = 0.0f, y = 0.0f;
	float r0 = 10.0f;
	float r1 = 55.0f;
};

struct EnemyAction {
	int enemyIndex;
	int damage;
};

class BattleScene : public SceneBase {
private:
	static const int SCREEN_W = 960;
	static const int SCREEN_H = 720;

	Vector2D playerPosition;
	Player* player = nullptr;

	// カーソル
	int targetCursor = 0;
	int commandCursor = 0;
	int selectImg = -1;

	BattleState battleState = BattleState::PlayerCommand;

	// メッセージ（完全手動）
	std::queue<std::string> messageQueue;
	std::string currentMessage;
	BattleState messageNextState = BattleState::PlayerCommand;

	// 敵
	std::vector<EnemyHandle> enemies;
	int totalEarnedExp = 0;

	// 遭遇
	EncounterEntry encounter;

	// 逃走：連続失敗救済
	int escapePity = 0;

	// 被ダメ演出
	float hitFlashTimer = 0.0f;
	float hitFlashDuration = 0.18f;
	float uiShakeTimer = 0.0f;
	float uiShakeDuration = 0.20f;
	int lastDamage = 0;
	std::vector<DamagePopup> popups;
	std::vector<HitSplash> splashes;

	// 敵ターン逐次処理
	std::vector<EnemyAction> enemyTurnQueue;
	int enemyTurnCursor = 0;

	// 導入フェード＋ズーム
	int fadeAlpha = 0;		 // 0..255
	int fadeState = 0;		 // 0:なし / 1:フェードイン
	float introZoomT = 0.0f; // 0→1 : 1.15→1.0
	float introZoomDur = 0.6f;

	// 攻撃演出
	float worldShakeTimer = 0.0f;
	float worldShakeDuration = 0.12f;
	float attackFlashTimer = 0.0f;
	float attackFlashDuration = 0.10f;
	float burstTimer = 0.0f;
	float burstDuration = 0.18f;
	int burstLines = 14;
	Vector2D effectPos;

	// 攻撃ズーム（徐々に戻る）
	float attackZoomT = 0.0f;
	float attackZoomDecay = 3.0f;
	float attackZoomAmp = 0.06f;

	// ターゲット赤枠点滅
	float blinkClock = 0.0f;

	// 勝利演出
	float victoryTimer = 0.0f;
	float finishZoomT = 0.0f;
	float finishZoomDur = 0.8f;

	// オフスクリーン
	int sceneScreen = -1;

	// プレイヤー防御
	bool isPlayerDefending = false;

	// 会心率（%）
	int criticalRatePercent = 6;

	// リザルト（勝利）
	bool resultInitialized = false;
	std::vector<std::string> resultLines;
	int resultLineIndex = 0;
	int resultCharIndex = 0;
	float resultTypeTimer = 0.0f;
	float resultTypeSpeed = 60.0f;
	bool resultGrantDone = false;

	// ★リザルト（敗北）
	bool defeatInitialized = false;
	std::vector<std::string> defeatLines;
	int defeatLineIndex = 0;
	int defeatCharIndex = 0;
	float defeatTypeTimer = 0.0f;
	float defeatTypeSpeed = 54.0f;
	float defeatDarkT = 0.0f;	  // 0→1 で赤暗転
	float defeatDarkSpeed = 0.7f; // 暗転速度

	// メッセージ系
	void enqueueMessage(const std::string& text);
	void beginMessages(BattleState nextState);
	void pumpMessageManual();

	// 敵ユーティリティ
	int livingEnemyCount() const;
	int firstLivingIndex() const;
	int nextLivingIndex(int start, int dir) const;
	void onEnemyDefeated(EnemyHandle& e);
	void giveAllExpAndExit();

	// 遭遇
	int chooseEnemyType() const;
	void spawnEnemiesByEncounter();

	// 逃走
	void attemptEscape();

	// 被ダメ演出
	void triggerPlayerHit(int damage);
	void updateHitEffects(float dt);
	void drawHitEffects();
	void drawHitSplashes();

	// オフスクリーン
	void ensureOffscreen();
	void releaseOffscreen();

	// ズーム値
	float getCurrentZoom() const;

	// 攻撃演出
	void startAttackEffect();
	void drawProceduralAttackEffects();

	// ウィンドウ
	void drawWindow(int x, int y, int w, int h, int fillR = 0, int fillG = 0, int fillB = 64);

	// 勝利リザルト
	void initResultScreen();
	void updateResult(float dt);
	void drawResult();
	bool grantExpAndMaybeRebuildResultLines();

	// ★敗北リザルト
	void initDefeatScreen();
	void updateDefeat(float dt);
	void drawDefeat();

	// 敵HP 視覚追従
	void updateEnemyHpDisplays(float dt);

public:
	BattleScene();
	~BattleScene();

	void SetPlayer(Player* p);

	void Initialize() override;
	eSceneType Update(float delta_second) override;
	void Draw() override;
	void Finalize() override;
	eSceneType GetNowSceneType() const override;

	Vector2D generate_location;
	Vector2D player_location;

	void SetPlayerPosition(const Vector2D& position);
};

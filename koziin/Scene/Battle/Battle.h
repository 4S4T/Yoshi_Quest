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

// �퓬�X�e�[�g
enum class BattleState {
	PlayerCommand,
	AttackSelect,
	MagicMenu, // ������񃁃j���[�i�g���p�j
	ItemMenu,  // �ǂ������j���[�i�g���p�j
	EnemyTurn, // �G�̓L���[��1�̂��s��
	Message,   // �蓮���胁�b�Z�[�W
	Result,	   // �������U���g�i�^�C�v���C�^�j
	Defeat	   // ���s�k���U���g�i�^�C�v���C�^�j
};

// �G�n���h��
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

	int dispHp = 1; // ���o�pHP
};

// �����e�[�u��
struct EncounterEntry {
	int minCount = 1;
	int maxCount = 3;
	std::vector<int> enemyTypeWeights; // 0:peabird, 1:Taurus ...
};

// �_���[�W�|�b�v
struct DamagePopup {
	float timer;
	int amount;
	float x, y;
};

// ��e�X�v���b�V���i�Ԃ��~�j
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

	// �J�[�\��
	int targetCursor = 0;
	int commandCursor = 0;
	int selectImg = -1;

	BattleState battleState = BattleState::PlayerCommand;

	// ���b�Z�[�W�i���S�蓮�j
	std::queue<std::string> messageQueue;
	std::string currentMessage;
	BattleState messageNextState = BattleState::PlayerCommand;

	// �G
	std::vector<EnemyHandle> enemies;
	int totalEarnedExp = 0;

	// ����
	EncounterEntry encounter;

	// �����F�A�����s�~��
	int escapePity = 0;

	// ��_�����o
	float hitFlashTimer = 0.0f;
	float hitFlashDuration = 0.18f;
	float uiShakeTimer = 0.0f;
	float uiShakeDuration = 0.20f;
	int lastDamage = 0;
	std::vector<DamagePopup> popups;
	std::vector<HitSplash> splashes;

	// �G�^�[����������
	std::vector<EnemyAction> enemyTurnQueue;
	int enemyTurnCursor = 0;

	// �����t�F�[�h�{�Y�[��
	int fadeAlpha = 0;		 // 0..255
	int fadeState = 0;		 // 0:�Ȃ� / 1:�t�F�[�h�C��
	float introZoomT = 0.0f; // 0��1 : 1.15��1.0
	float introZoomDur = 0.6f;

	// �U�����o
	float worldShakeTimer = 0.0f;
	float worldShakeDuration = 0.12f;
	float attackFlashTimer = 0.0f;
	float attackFlashDuration = 0.10f;
	float burstTimer = 0.0f;
	float burstDuration = 0.18f;
	int burstLines = 14;
	Vector2D effectPos;

	// �U���Y�[���i���X�ɖ߂�j
	float attackZoomT = 0.0f;
	float attackZoomDecay = 3.0f;
	float attackZoomAmp = 0.06f;

	// �^�[�Q�b�g�Ԙg�_��
	float blinkClock = 0.0f;

	// �������o
	float victoryTimer = 0.0f;
	float finishZoomT = 0.0f;
	float finishZoomDur = 0.8f;

	// �I�t�X�N���[��
	int sceneScreen = -1;

	// �v���C���[�h��
	bool isPlayerDefending = false;

	// ��S���i%�j
	int criticalRatePercent = 6;

	// ���U���g�i�����j
	bool resultInitialized = false;
	std::vector<std::string> resultLines;
	int resultLineIndex = 0;
	int resultCharIndex = 0;
	float resultTypeTimer = 0.0f;
	float resultTypeSpeed = 60.0f;
	bool resultGrantDone = false;

	// �����U���g�i�s�k�j
	bool defeatInitialized = false;
	std::vector<std::string> defeatLines;
	int defeatLineIndex = 0;
	int defeatCharIndex = 0;
	float defeatTypeTimer = 0.0f;
	float defeatTypeSpeed = 54.0f;
	float defeatDarkT = 0.0f;	  // 0��1 �ŐԈÓ]
	float defeatDarkSpeed = 0.7f; // �Ó]���x

	// ���b�Z�[�W�n
	void enqueueMessage(const std::string& text);
	void beginMessages(BattleState nextState);
	void pumpMessageManual();

	// �G���[�e�B���e�B
	int livingEnemyCount() const;
	int firstLivingIndex() const;
	int nextLivingIndex(int start, int dir) const;
	void onEnemyDefeated(EnemyHandle& e);
	void giveAllExpAndExit();

	// ����
	int chooseEnemyType() const;
	void spawnEnemiesByEncounter();

	// ����
	void attemptEscape();

	// ��_�����o
	void triggerPlayerHit(int damage);
	void updateHitEffects(float dt);
	void drawHitEffects();
	void drawHitSplashes();

	// �I�t�X�N���[��
	void ensureOffscreen();
	void releaseOffscreen();

	// �Y�[���l
	float getCurrentZoom() const;

	// �U�����o
	void startAttackEffect();
	void drawProceduralAttackEffects();

	// �E�B���h�E
	void drawWindow(int x, int y, int w, int h, int fillR = 0, int fillG = 0, int fillB = 64);

	// �������U���g
	void initResultScreen();
	void updateResult(float dt);
	void drawResult();
	bool grantExpAndMaybeRebuildResultLines();

	// ���s�k���U���g
	void initDefeatScreen();
	void updateDefeat(float dt);
	void drawDefeat();

	// �GHP ���o�Ǐ]
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

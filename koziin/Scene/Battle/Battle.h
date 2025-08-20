#pragma once

#include "../SceneBase.h"
#include "../../Utility/Vector2D.h"
#include"../../Object/Enemy/EnemyType/Slime.h"
#include"../../Object/Enemy/EnemyType/peabird.h"
#include"../../Object/Enemy/EnemyType/Taurus.h"
#include"../Map/Map.h"


class BattleScene : public SceneBase {
private:
	Vector2D playerPosition; // プレイヤー位置保持
	Player* player;			  // Mapから受け取るプレイヤー情報
	bool battleEnded = false; // 戦闘終了フラグを追加
	int cursor;
	int select;
	bool isPlayerTurn = true; // trueならプレイヤーのターン、falseなら敵のターン
	std::string battleMessage; // 戦闘メッセージ
	float messageTimer = 0.0f; // メッセージ表示用タイマー
							   // BattleSceneのメンバ変数に追加
	float battleEndTimer = -1.0f; // -1: 未設定, 0以上: カウントダウン中
	bool isSlimeDefeated = false;
	bool isTaurusDefeated = false;



public:
	BattleScene();
	~BattleScene();
	void SetPlayer(Player* p); // プレイヤーデータをセットする関数

	void Initialize() override;
	eSceneType Update(float delta_second) override;
	void OnEnemyDefeated(const std::string& name, int exp);

	void Draw() override;
	void Finalize() override;
	eSceneType GetNowSceneType() const override;

	Vector2D generate_location;
	Vector2D player_location;
	Slime* slime;
	peabird* Peabird;
	Taurus* taurus;
	void SetPlayerPosition(const Vector2D& position); // 位置設定メソッド
};
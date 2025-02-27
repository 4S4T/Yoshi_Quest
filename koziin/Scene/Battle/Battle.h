#pragma once

#include "../SceneBase.h"
#include "../../Utility/Vector2D.h"
#include"../../Object/Enemy/EnemyType/Slime.h"

class BattleScene : public SceneBase {
private:
	Vector2D playerPosition; // プレイヤー位置保持

public:
	BattleScene();
	~BattleScene();

	void Initialize() override;
	eSceneType Update(float delta_second) override;
	void Draw() override;
	void Finalize() override;
	eSceneType GetNowSceneType() const override;

	Vector2D generate_location;
	Slime* slime;

	void SetPlayerPosition(const Vector2D& position); // 位置設定メソッド
};
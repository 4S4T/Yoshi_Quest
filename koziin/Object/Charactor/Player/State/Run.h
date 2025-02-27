#pragma once
#include "PlayerStateBase.h"
#include "../../../../Utility/Vector2D.h"
#include<vector>

/// <summary>
/// ジャンプ状態
/// </summary>
class RunState : public PlayerStateBase {
private:
	std::vector<int> player_animation; // 移動のアニメーション画像
	int animation[2];				   // アニメーション画像
	// 前フレームの座標取得
	Vector2D old_location;
	float animation_time; // アニメーション時間
	int animation_count;  // アニメーション添字

	// 移動アニメーションの順番
	const int animation_num[1] = 
	{
		0
	};
	public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="p">プレイヤー情報</param>
	RunState(class Player* p);

	// デストラクタ
	virtual ~RunState();

public:
	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void Update(float delta_second) override;
	virtual void Draw() const override;
	void AnimationControl(float delta_second);

	// 現在のステート情報を取得する
	virtual ePlayerState GetState() const override;
};
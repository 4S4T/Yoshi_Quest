#pragma once
#include "PlayerStateBase.h"

/// <summary>
/// 停止状態
/// </summary>
class IdleState : public PlayerStateBase {
private:
	// 入力情報
	class InputControl* input;

public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="p">プレイヤー情報</param>
	IdleState(class Player* p);

	// デストラクタ
	virtual ~IdleState();

public:
	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void Update(float delta_second) override;
	virtual void Draw() const override;

	// 現在のステート情報を取得する
	virtual ePlayerState GetState() const override;
};
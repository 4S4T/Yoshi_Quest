#pragma once
#include "Enum/PlayerState.h"

/// <summary>
/// プレイヤー状態管理用のベースクラス
/// </summary>
class PlayerStateBase {
protected:
	// プレイヤー情報
	class Player* player;
	int image;

public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="p">プレイヤー情報</param>
	PlayerStateBase(class Player* p) : player(p), image(0) {
	}

	// デストラクタ
	virtual ~PlayerStateBase() = default;

public:
	virtual void Initialize() = 0;
	virtual void Finalize() = 0;
	virtual void Update(float delta_second) = 0;
	virtual void Draw() const = 0;


	// 状態の取得
	virtual ePlayerState GetState() const = 0;
};
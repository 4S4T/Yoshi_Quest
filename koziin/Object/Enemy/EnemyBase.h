#pragma once
#include"../../Application.h"
#include"../GameObject.h"
#include"DxLib.h"

class EnemyBase : public GameObject
{
protected:
	std::vector<int> attck;
	
	EnemyBase();
	~EnemyBase();

	public:
	// 初期化処理
	virtual void Initialize() override;
	// 描画処理
	virtual void Draw(const Vector2D& screen_offset) const;
	// 終了時処理
	virtual void Finalize();


	public:
	std::vector<int> GetAttckPatten();
};

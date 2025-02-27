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
	// ‰Šú‰»ˆ—
	virtual void Initialize() override;
	// •`‰æˆ—
	virtual void Draw(const Vector2D& screen_offset) const;
	// I—¹ˆ—
	virtual void Finalize();


	public:
	std::vector<int> GetAttckPatten();
};

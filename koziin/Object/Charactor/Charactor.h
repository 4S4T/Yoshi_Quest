#pragma once
#include "../GameObject.h"

class Character : public GameObject
{
public:
	Character();
	~Character();

public:
	Vector2D velocity;

protected:
	int hit_point;
	float animation_time;
	int animation_count;
	std::vector<std::vector<char>> mapdata;

	std::vector<int> move_animation;

public:
	// 初期化処理
	virtual void Initialize() override;
	// 描画処理
	virtual void Draw(const Vector2D& screen_offset) const;
	// 終了時処理
	virtual void Finalize();

	void SetMapData(std::vector<std::vector<char>>& data);

	bool MapCollision(int x, int y);
};
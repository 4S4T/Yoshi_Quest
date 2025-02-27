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
	// ‰Šú‰»ˆ—
	virtual void Initialize() override;
	// •`‰æˆ—
	virtual void Draw(const Vector2D& screen_offset) const;
	// I—¹ˆ—
	virtual void Finalize();

	void SetMapData(std::vector<std::vector<char>>& data);

	bool MapCollision(int x, int y);
};
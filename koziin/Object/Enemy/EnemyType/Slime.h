#pragma once
#include"../EnemyBase.h"
class Slime : public EnemyBase
{
public:
	Slime();
	~Slime();

	void Initialize() override;
	void Finalize();
	void Update(float delta_second);
	void Draw(const Vector2D& screen_offset) const;


};

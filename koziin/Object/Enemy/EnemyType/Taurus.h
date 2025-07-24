#pragma once
#include "../EnemyBase.h"
class Taurus : public EnemyBase 
{
public:
	Taurus();
	~Taurus();

	void Initialize() override;
	void Finalize();
	void Update(float delta_second);
	void Draw(const Vector2D& screen_offset) const;
};

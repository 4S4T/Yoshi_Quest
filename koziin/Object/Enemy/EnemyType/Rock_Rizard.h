#pragma once
#include "../EnemyBase.h"
class Rock_Rizard : public EnemyBase 
{
public:
	Rock_Rizard();
	~Rock_Rizard();

	void Initialize() override;
	void Finalize();
	void Update(float delta_second);
	void Draw(const Vector2D& screen_offset) const;
};

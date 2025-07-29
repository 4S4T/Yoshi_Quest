#pragma once
#include "../EnemyBase.h"

class peabird : public EnemyBase 
{
public:

	peabird();
	~peabird();

	void Initialize() override;
	void Finalize();
	void Update(float delta_second);
	void Draw(const Vector2D& screen_offset) const;
};

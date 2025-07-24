#pragma once
#include "../EnemyBase.h"

class Rizard : public EnemyBase {
public:
	Rizard();
	~Rizard();

	void Initialize() override;
	void Finalize();
	void Update(float delta_second);
	void Draw(const Vector2D& screen_offset) const;
};

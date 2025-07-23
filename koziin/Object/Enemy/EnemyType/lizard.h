#pragma once
#include "../EnemyBase.h"

class lizard : public EnemyBase {
public:
	lizard();
	~lizard();

	void Initialize() override;
	void Finalize();
	void Update(float delta_second);
	void Draw(const Vector2D& screen_offset) const;
};

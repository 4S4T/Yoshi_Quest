#pragma once
#include "../EnemyBase.h"
class Rock_Lizard : public EnemyBase {
public:
	Rock_Lizard();
	~Rock_Lizard();

	void Initialize() override;
	void Finalize();
	void Update(float delta_second);
	void Draw(const Vector2D& screen_offset) const;
};

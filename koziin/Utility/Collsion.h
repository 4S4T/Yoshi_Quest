#pragma once

#include "Vector2D.h"
#include <vector>

// オブジェクトタイプ
enum class eObjectType : unsigned char {
	none,
	player
};

// 当たり判定基底クラス
class Collision {
public:
	bool is_blocking;						  // すり抜けフラグ
	eObjectType object_type;				  // 自身のオブジェクトタイプ
	std::vector<eObjectType> hit_object_type; // 適用するオブジェクトタイプ
	float radius;							  // 円の半径
	Vector2D point[2];						  // 始点と終点（相対座標）

public:
	Collision() : is_blocking(false),
				  object_type(eObjectType::none),
				  hit_object_type(),
				  radius(0.0f) {
		point[0] = 0.0f;
		point[1] = 0.0f;
	}
	~Collision() {
		hit_object_type.clear();
	}

	// 当たり判定有効確認処理
	bool IsCheckHitTarget(eObjectType hit_object) const;
};


bool IsCheckCollision(const Collision& c1, const Collision& c2);

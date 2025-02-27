#include "Charactor.h"
#include"../../Utility/ResourceManager.h"
#include"../GameObject.h"
#include<cmath>
#include"../../Application.h"

Character::Character():velocity(0.0f), hit_point(0), animation_time(0.0f), animation_count(0)
{
} 

Character::~Character()
{
}

void Character::Initialize()
{
}

void Character::Draw(const Vector2D& screen_offset) const 
{
	__super::Draw(screen_offset);
}

void Character::Finalize()
{
	// 動的配列の解放
	move_animation.clear();
}

void Character::SetMapData(std::vector<std::vector<char>>& data)
{
	mapdata = data;
}

// マップとの当たり判定
bool Character::MapCollision(int x, int y) {
	// 範囲外だったらあたってない
	if (this->location.x < 0 || this->location.x >= D_WIN_MAX_X || this->location.y < 24.0f * 4 ||  this->location.y >= D_WIN_MAX_Y) {
		return false; // マップ範囲外
	}

	//// ワールド座標を保存
	Vector2D object_rect = Vector2D(this->location.x , this->location.y);
	// サイズを保存
	Vector2D object_box = this->GetBoxSize();
	// 四つの頂点を保存
	Vector2D vertices[4] = 
	{
		// 左上の座標
		Vector2D(object_rect - object_box),
		// 左下の座標
		Vector2D(object_rect.x - object_box.x, object_rect.y + object_box.y),
		// 右上の座標
		Vector2D(object_rect.x + object_box.x, object_rect.y - object_box.y),
		// 右下の座標
		Vector2D(object_rect + object_box),
	};

	for (int i = 0; i < 4; i++) {
		// プレイヤーの現在のマスの位置
		int x_id = std::floor(vertices[i].x) / (24.0f * 2);
		int y_id = std::floor(vertices[i].y) / (24.0f * 2);
		// 当たり判定を付ける背景
		if (mapdata[y_id + y][x_id + x] == '2')
		{
			// どのポイントが当たっているか
			int id = i;
			// 当たっている
			return true;
		}
	}
	// 当たっていない
	return false;
}

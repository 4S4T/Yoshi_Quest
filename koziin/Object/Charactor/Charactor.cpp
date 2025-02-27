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
	// ���I�z��̉��
	move_animation.clear();
}

void Character::SetMapData(std::vector<std::vector<char>>& data)
{
	mapdata = data;
}

// �}�b�v�Ƃ̓����蔻��
bool Character::MapCollision(int x, int y) {
	// �͈͊O�������炠�����ĂȂ�
	if (this->location.x < 0 || this->location.x >= D_WIN_MAX_X || this->location.y < 24.0f * 4 ||  this->location.y >= D_WIN_MAX_Y) {
		return false; // �}�b�v�͈͊O
	}

	//// ���[���h���W��ۑ�
	Vector2D object_rect = Vector2D(this->location.x , this->location.y);
	// �T�C�Y��ۑ�
	Vector2D object_box = this->GetBoxSize();
	// �l�̒��_��ۑ�
	Vector2D vertices[4] = 
	{
		// ����̍��W
		Vector2D(object_rect - object_box),
		// �����̍��W
		Vector2D(object_rect.x - object_box.x, object_rect.y + object_box.y),
		// �E��̍��W
		Vector2D(object_rect.x + object_box.x, object_rect.y - object_box.y),
		// �E���̍��W
		Vector2D(object_rect + object_box),
	};

	for (int i = 0; i < 4; i++) {
		// �v���C���[�̌��݂̃}�X�̈ʒu
		int x_id = std::floor(vertices[i].x) / (24.0f * 2);
		int y_id = std::floor(vertices[i].y) / (24.0f * 2);
		// �����蔻���t����w�i
		if (mapdata[y_id + y][x_id + x] == '2')
		{
			// �ǂ̃|�C���g���������Ă��邩
			int id = i;
			// �������Ă���
			return true;
		}
	}
	// �������Ă��Ȃ�
	return false;
}

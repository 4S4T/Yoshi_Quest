#pragma once
#include"Vector2D.h"
#include "DxLib.h"

class AttackEffect {
public:
	Vector2D position; // �G�t�F�N�g�̈ʒu
	float duration;	   // �G�t�F�N�g�̎�������
	int currentFrame;  // ���݂̃t���[��
	int totalFrames;   // �G�t�F�N�g�̃t���[����
	int effectImage;   // �G�t�F�N�g�̉摜
	bool isActive;	   // �G�t�F�N�g���A�N�e�B�u���ǂ���

	AttackEffect(Vector2D pos) {
		position = pos;
		duration = 0.5f; // �G�t�F�N�g���������ԁi�Ⴆ��0.5�b�j
		currentFrame = 0;
		totalFrames = 5;												// �Ⴆ��5�t���[���̃X�v���C�g
		effectImage = LoadGraph("Resource/Images/attack_effect_%.png"); // �X�v���C�g�V�[�g�̃p�X���w��
		isActive = true;
	}

	void Update(float delta_second) {
		if (isActive) {
			duration -= delta_second;
			if (duration <= 0.0f) {
				isActive = false; // �G�t�F�N�g�I��
			}
			else {
				// �A�j���[�V������i�߂�
				currentFrame++;
				if (currentFrame >= totalFrames) {
					currentFrame = totalFrames - 1; // �Ō�̃t���[���Ŏ~�܂�
				}
			}
		}
	}

	void Draw() {
		if (isActive) {
			// �X�v���C�g�V�[�g���猻�݂̃t���[����\��
			DrawGraph(position.x, position.y, effectImage + currentFrame, TRUE);
		}
	}
};

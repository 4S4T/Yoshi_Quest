#pragma once

enum class ePlayerState : unsigned char
{
	IDLE = 0, // ��~���
	RUN,	  // �ړ����
	NONE // �����Ȃ�
};

enum class PlayerCommandState {
	MainCommand, // �키�E�����铙��I��
	EnemySelect	 // �G��I��
};

PlayerCommandState commandState;

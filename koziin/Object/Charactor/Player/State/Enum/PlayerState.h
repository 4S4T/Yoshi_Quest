#pragma once

enum class ePlayerState : unsigned char
{
	IDLE = 0, // ��~���
	RUN,	  // �ړ����
	NONE // �����Ȃ�
};

enum class PlayerCommandState {
	MainCommand, // �u���������v�u�ɂ���v�Ȃ�
	AttackTarget // �G��I��ōU��
};


extern PlayerCommandState commandState;

#pragma once

#include <string>
#include "Scene/SceneBase.h"

class Application : public Singleton<class Type> {
private:
	SceneBase* curent_scene;
	bool end;

public:
	Application();
	~Application();

	/// <summary>
	/// ����������
	/// </summary>
	void WakeUp();

	/// <summary>
	///  ���s����
	/// </summary>
	void Run();

	/// <summary>
	/// �I��������
	/// </summary>
	void Shutdown();
};

/**********************************************************
 * �}�N����`
 ***********************************************************/
#define D_SUCCESS (0)  // ����
#define D_FAILURE (-1) // ���s

#define D_WIN_MAX_X (672) // �X�N���[���T�C�Y�i���j
#define D_WIN_MAX_Y (864) // �X�N���[���T�C�Y�i�����j
#define D_COLOR_BIT (32)  // �J���[�r�b�g

/**********************************************************
 * �v���g�^�C�v�錾
 ***********************************************************/
/// <summary>
/// �t���[������
/// </summary>
void FreamControl();

/// <summary>
/// 1�t���[��������̎��Ԃ��擾����
/// </summary>
/// <returns>1�t���[��������̎���</returns>
const float& GetDeltaSecond();

/// <summary>
/// �G���[���e���o�͂���
/// </summary>
/// <returns>�I����Ԃ̒l</returns>
int ErrorThrow(std::string error_log);

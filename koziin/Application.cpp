#include "Scene/SceneManager.h"
#include "Application.h"
#include "DxLib.h"
#include "Utility/InputControl.h"
#include"Scene/Title/Title.h"


// �O���[�o���ϐ���`
LONGLONG old_time;	// �O��v���l
LONGLONG now_time;	// ���݌v���l
float delta_second; // �P�t���[��������̎���



Application::Application():end(false)  {
}

Application::~Application() {
	Shutdown();
}

void Application::WakeUp() {
	// �E�B���h�E���[�h�ŋN��
	ChangeWindowMode(TRUE);

	// ��ʃT�C�Y�̐ݒ�
	SetGraphMode(960, 720, 32);

	// DX���C�u�����̏�����
	if (DxLib_Init() == -1) {
		throw("DX���C�u�������������ł��܂���ł���\n");
	}

	// ����ʂ���`����n�߂�
	SetDrawScreen(DX_SCREEN_BACK);

	// �^�C�g����ʂ���V�[�����J�n����
	SceneManager* scene = Singleton<SceneManager>::GetInstance();
	scene->ChangeScene(eSceneType::eTitle);

	SetAlwaysRunFlag(TRUE);
}

void Application::Run() {
	InputControl* input = Singleton<InputControl>::GetInstance();
	SceneManager* sceneManager = Singleton<SceneManager>::GetInstance();

	while (ProcessMessage() != -1 && input->GetKeyUp(KEY_INPUT_ESCAPE) != true) {
		input->Update();					// ���͋@�\�̍X�V
		FreamControl();						// �t���[������
		ClearDrawScreen();					// ��ʂ��N���A
		sceneManager->Draw();				// �V�[���̕`��
		end = sceneManager->Update(delta_second); // �V�[���̍X�V
		if (end == true)
		{
			break;  
		}
		ScreenFlip();						// ����ʂ̓��e�𔽉f
	}
}
void Application::Shutdown() {
}


void FreamControl() {
	// ���ݎ��Ԃ̎擾�i�ʕb�j
	now_time = GetNowHiPerformanceCount();

	// �J�n���Ԃ��猻�ݎ��Ԃ܂łɌo�߂������Ԃ��v�Z����i�ʕb�j
	// ����\���ʕb���b�ɕϊ�����
	delta_second = (float)(now_time - old_time) * 1.0e-6f;

	// �v���J�n���Ԃ��X�V����
	old_time = now_time;

	// �f�B�X�v���C�̃��t���b�V�����[�g���擾����
	float refresh_rate = (float)GetRefreshRate();

	// �P�t���[��������̎��Ԃ�1/���t���b�V�����[�g�b�𒴂�����A������
	if (delta_second > (1.0f / refresh_rate)) {
		delta_second = (1.0f / refresh_rate);
	}
}

// 1�t���[��������̎��Ԃ��擾����
const float& GetDeltaSecond() {
	return delta_second;
}

// �G���[���e���o�͂���
int ErrorThrow(std::string error_log) {
	// Log.txt�ɃG���[���e��ǉ�����
	ErrorLogFmtAdd(error_log.c_str());
	return D_FAILURE;
}
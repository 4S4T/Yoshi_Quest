#include "MemuSene.h"
#include "../../Utility/InputControl.h"
#include "DxLib.h"
#include "../SceneManager.h"
#include "../Battle/Battle.h"
#include "../../Utility/ResourceManager.h"

Memu::Memu() {
}

Memu::~Memu() {
}


void Memu::Initialize() {
	ResourceManager* rm = ResourceManager::GetInstance();
	OptionImage = rm->GetImages("Resource/Images/option.png", 1, 1, 1, 32, 32)[0];
	currentSelection = 0; // �ŏ��̍��ڂ�I����Ԃ�
	subMenuState = SubMenuState::None;
}

eSceneType Memu::Update(float delta_second) {

    InputControl* input = Singleton<InputControl>::GetInstance();

    if (subMenuState == SubMenuState::None) {
        // ���j���[���ڑI��
        if (input->GetKeyDown(KEY_INPUT_UP)) {
            currentSelection = (currentSelection - 1 + menuItemCount) % menuItemCount;
        }
        if (input->GetKeyDown(KEY_INPUT_DOWN)) {
            currentSelection = (currentSelection + 1) % menuItemCount;
        }
		if (input->GetKeyDown(KEY_INPUT_RETURN)||(input->GetKeyDown(KEY_INPUT_SPACE))) {
            switch (currentSelection) {
                case 0: // �ݒ�
                    subMenuState = SubMenuState::Option;
                    break;
                case 1: // �N���W�b�g
                    subMenuState = SubMenuState::Credit;
                    break;
                case 2: // �}�b�v�ɖ߂�
                    return eSceneType::eMap;
            }
        }
    }
    else {
        // �T�u���j���[�\����
		if (input->GetKeyDown(KEY_INPUT_TAB) || input->GetKeyDown(KEY_INPUT_ESCAPE) || (input->GetKeyDown(KEY_INPUT_SPACE))) {
            // �߂�L�[�ŃT�u���j���[�I��
            subMenuState = SubMenuState::None;
        }
    }

    return GetNowSceneType();
}



void Memu::Draw() {

	if (subMenuState == SubMenuState::None) {
		// ���j���[�I�����\��
		for (int i = 0; i < menuItemCount; ++i) {
			int color = (i == currentSelection) ? GetColor(255, 255, 0) : GetColor(255, 255, 255);
			DrawFormatString(120, 200 + i * 40, color, menuItems[i]);
		}
	}
	else if (subMenuState == SubMenuState::Option) {
		// �I�v�V��������`��
		DrawFormatString(120, 200, GetColor(255, 255, 255), "=== �I�v�V������� ===");
		DrawFormatString(120, 240, GetColor(255, 255, 255), "���ʒ����⑀��ݒ�������ɕ\��");
		DrawFormatString(120, 280, GetColor(255, 255, 255), "�߂�ɂ� TAB �܂��� ESC �������Ă�������");
	}
	else if (subMenuState == SubMenuState::Credit) {
		// �N���W�b�g����`��
		DrawFormatString(120, 200, GetColor(255, 255, 255), "=== �N���W�b�g ===");
		DrawFormatString(120, 240, GetColor(255, 255, 255), "����F���Ȃ��̖��O");
		DrawFormatString(120, 280, GetColor(255, 255, 255), "���́F�Z�Z�Z�Z");
		DrawFormatString(120, 320, GetColor(255, 255, 255), "�߂�ɂ� TAB �܂��� ESC �������Ă�������");
	}
}


void Memu::Finalize() {
}

eSceneType Memu::GetNowSceneType() const {
	return eSceneType::eOption;
}

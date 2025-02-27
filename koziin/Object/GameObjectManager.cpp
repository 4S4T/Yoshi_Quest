#include "GameObjectManager.h"
#include "DxLib.h"

GameManager::GameManager() : create_objects(), destroy_objects(), game_objects(), screen_offset() {
}

GameManager::~GameManager() {
}

void GameManager::Initialize() {
}

void GameManager::Update(float delta_second) {
	CheckCreateObject();

	// �����蔻��m�F����
	for (int i = 0; i < object_list.size(); i++) {
		if (object_list[i]->GetMobility() == eMobilityType::Stationary) {
			continue;
		} 

		for (int j = 0; j < object_list.size(); j++) {

			if (i == j) {
				continue;
			}

			CheckCollision(object_list[i], object_list[j]);
		}
	}

	for (GameObject* object : object_list) {
		object->Update(delta_second);
	}

	CheckDestroyObject();
}

void GameManager::Draw() {
	// �I�u�W�F�N�g���X�g���̃I�u�W�F�N�g��`�悷��
	for (GameObject* obj : object_list) {
		// obj->Initialize();
		obj->Draw(screen_offset);
	}
}

void GameManager::Finalize() {
	// �I�u�W�F�N�g���X�g���̃I�u�W�F�N�g��j������
	DestoryAllObject();

	// ���I�z��̉��
	if (!object_list.empty()) {
		object_list.clear();
	}
	/*if (!create_objects.empty())
	{
		create_objects.clear();
	}*/
}

void GameManager::CheckCreateObject() {
	// ��������I�u�W�F�N�g������΁A�I�u�W�F�N�g���X�g���ɑ}������
	if (!create_objects.empty()) {
		for (GameObject* obj : create_objects) {
			// ���C���[������ɏ��Ԃ����ւ���
			int z_layer = obj->GetZLayer();
			std::vector<GameObject*>::iterator itr = object_list.begin(); // �I�u�W�F�N�g���X�g�̐擪
			// ���X�g�̖����ɂȂ�܂ő�������
			for (; itr != object_list.end(); itr++) {
				// Z���C���[���傫���ꏊ�ɗv�f��ǉ�����
				// �� itr->ZLayer{1, 1, 2, 3}�Az_layer = 2 �̎�
				//    itr->ZLayer{1, 1, 2, z_layer, 3}�Ƃ���
				if (z_layer < (*itr)->GetZLayer()) {
					break;
				}
			}
			// ���X�g�̓r���ɑ}������
			object_list.insert(itr, obj);
		}
		// �������X�g���������
		create_objects.clear();
	}
}

void GameManager::CheckDestroyObject() {
	// �j�����X�g������łȂ��ꍇ�A���X�g���̃I�u�W�F�N�g��j������
	if (!destroy_objects.empty()) {
		for (GameObject* obj : destroy_objects) {
			std::vector<GameObject*>::iterator itr = object_list.begin(); // �I�u�W�F�N�g���X�g�̐擪
			// ���X�g�̖����ɂȂ�܂ő�������
			for (; itr != object_list.end(); itr++) {
				// ���X�g���ɂ���΁A�폜����
				if ((*itr) == obj) {
					object_list.erase(itr);
					obj->Finalize();
					delete obj;
					break;
				}
			}
		}
		// ���I�z��̊J��
		destroy_objects.clear();
	}
}

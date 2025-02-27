#pragma once
#include "../Singleton.h"
#include "../Object/GameObject.h"
#include <vector>
#include"../Utility/Collsion.h"

class GameManager : public Singleton<class Type> {
protected:
	std::vector<GameObject*> create_objects;
	std::vector<GameObject*> object_list;
	std::vector<GameObject*> destroy_objects;
	std::vector<GameObject*> game_objects;
	Vector2D screen_offset;

public:
	GameManager();
	~GameManager();

	void Initialize();
	void Update(float);
	void Draw();
	void Finalize();
	void CheckCreateObject();
	void CheckDestroyObject();

	// const std::vector<GameObject*>& const GetObjectsList();

	template <class OBJECT>
	OBJECT* CreateGameObject(const Vector2D& generate_location) {
		// ��������Q�[���I�u�W�F�N�g�N���X�𓮓I�m��
		OBJECT* new_instance = new OBJECT();
		// GameObjectBase���p�����Ă��邩�m�F
		GameObject* new_object = dynamic_cast<GameObject*>(new_instance);
		// �G���[�`�F�b�N
		if (new_object == nullptr) {
			// ���I�m�ۂ����������̊J��
			delete new_instance;
			return nullptr;
		}
		// �V�[�����̐ݒ�
		new_object->SetOwnerScene(this);
		// �I�u�W�F�N�g�̏�����
		new_object->Initialize();
		// �ʒu���̐ݒ�
		new_object->SetLocation(generate_location);
		// �����I�u�W�F�N�g���X�g�̖����Ɋi�[����
		create_objects.push_back(new_object);
		// ���������Q�[���I�u�W�F�N�g�̃|�C���^��Ԃ�
		return new_instance;
	}

	/// <summary>
	/// �I�u�W�F�N�g�j������
	/// </summary>
	/// <param name="target">�j�����s���Q�[���I�u�W�F�N�g�̃|�C���^</param>
	void DestroyObject(GameObject* target) {
		// �k���|�`�F�b�N
		if (target == nullptr) {
			return;
		}

		// �j���I�u�W�F�N�g���X�g���ɂ���΁A�ǉ����Ȃ�
		for (GameObject* obj : destroy_objects) {
			if (obj == target) {
				return;
			}
		}

		// �j�����s���I�u�W�F�N�g���X�g�ɒǉ�����
		destroy_objects.push_back(target);
	}

private:
	/// <summary>
	/// �V�[�����I�u�W�F�N�g�j������
	/// </summary>
	void DestoryAllObject() {
		// �I�u�W�F�N�g���X�g����Ȃ珈�����I������
		if (object_list.empty()) {
			return;
		}
		// �I�u�W�F�N�g���X�g���̃I�u�W�F�N�g���폜����
		for (GameObject* obj : object_list) {
			obj->Finalize();
			delete obj;
		}
		// ���I�z��̉��
		object_list.clear();
	}

	/// <summary>
	/// �����蔻��m�F����
	/// </summary>
	/// <param name="target">1�ڂ̃Q�[���I�u�W�F�N�g</param>
	/// <param name="partner">2�ڂ̃Q�[���I�u�W�F�N�g</param>
	virtual void CheckCollision(GameObject* target, GameObject* partner) {
		// �k���|�`�F�b�N
		if (target == nullptr || partner == nullptr) {
			return;
		}

		// �����蔻������擾
		Collision tc = target->GetCollision();
		Collision pc = partner->GetCollision();

		// �����蔻�肪�L�����m�F����
		if (tc.IsCheckHitTarget(pc.object_type) || pc.IsCheckHitTarget(tc.object_type)) {

			// �����̎n�_�ƏI�_��ݒ肷��
			tc.point[0] += target->GetLocation() - target->GetBoxSize();
			tc.point[1] += target->GetLocation() + target->GetBoxSize();
			pc.point[0] += partner->GetLocation() - partner->GetBoxSize();
			pc.point[1] += partner->GetLocation() + partner->GetBoxSize();

			// �J�v�Z�����m�̓����蔻��
			if (IsCheckCollision(tc, pc)) {
				// �������Ă��邱�Ƃ�ʒm����
				target->OnHitCollision(partner);
				partner->OnHitCollision(target);
			}
		}
	}
};


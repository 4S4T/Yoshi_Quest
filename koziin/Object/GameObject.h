#pragma once
#include "../Utility/Vector2D.h"
#include"../Utility/Collsion.h"

/// <summary>
/// �����̎��
/// </summary>
enum eMobilityType {
	Stationary, // �Œ�
	Movable,	// ��
};

class GameObject {

protected:
	class GameManager* owner_scene;
	Vector2D location;
	unsigned char z_layer;
	int image;
	int image2;
	Vector2D box_size;
	Collision collision;
	eMobilityType mobility; // ����
	bool MapCollision(int x, int y);
	int attack;
	int defense;
	int hp;
	bool alive;


public:
	GameObject();
	virtual ~GameObject();;

	/// <summary>
	/// ����������
	/// </summary>
	virtual void Initialize();
	/// <summary>
	/// �X�V����
	/// </summary>
	/// <param name="delta_second">1�t���[��������̎���</param>
	virtual void Update(float delta_second);
	/// <summary>
	/// �`�揈��
	/// </summary>
	/// <param name="screen_offset">�I�t�Z�b�g�l</param>
	virtual void Draw(const Vector2D& screen_offset) const;
	/// <summary>
	/// �I��������
	/// </summary>
	virtual void Finalize();

public:
	/// <summary>
	/// �����蔻��ʒm����
	/// </summary>
	/// <param name="hit_object">���������Q�[���I�u�W�F�N�g�̃|�C���^</param>
	virtual void OnHitCollision(GameObject* hit_object);

public:
	/// <summary>
	/// ���L�V�[�����̐ݒ�
	/// </summary>
	/// <param name="scene">���L�V�[�����</param>
	void SetOwnerScene(class GameManager* scene);

	/// <summary>
	/// �ʒu���W�擾����
	/// </summary>
	/// <returns>�ʒu���W���</returns>
	const Vector2D& GetLocation() const;

	const int GetAttack();
	void SetAttack(const int attack);

	const int GetDefense();


	void SetDefense(const int defense);
	const int GetAHp();
	void SetHp(const int hp);

	/// <summary>
	/// �ʒu���ύX����
	/// </summary>
	/// <param name="location">�ύX�������ʒu���</param>
	void SetLocation(const Vector2D& location);


	/// <summary>
	/// �y���C���[���擾����
	/// </summary>
	/// <returns>�y���C���[���</returns>
	const int GetZLayer() const;

	Collision GetCollision() const;

	/// <summary>
	/// �������̎擾����
	/// </summary>
	/// <returns>�������</returns>
	const eMobilityType GetMobility() const;

	Vector2D GetBoxSize();
};
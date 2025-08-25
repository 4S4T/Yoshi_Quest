#pragma once
#include "../../Utility/Vector2D.h"
#include <string>
#include <vector>

class NCP {
public:
	NCP();
	NCP(const Vector2D& pos, const std::string& name, const std::vector<std::string>& lines);

	void SetPosition(const Vector2D& pos);
	Vector2D GetPosition() const;

	void SetName(const std::string& name);
	void SetDialog(const std::vector<std::string>& lines);

	void BeginTalk();	// ��b�J�n�i1�s�ڂ�\���j
	void AdvanceTalk(); // ���̍s�ցi�Ō�Ŏ����I���j
	void EndTalk();		// ��b�����I��
	bool IsTalking() const;

	// �v���C���[����苗���ȓ���
	bool IsPlayerInRange(const Vector2D& playerPos, float range) const;

	// 1�t���[���X�V�E�`��
	void Update(float delta);
	void Draw() const;

private:
	Vector2D position;
	std::string ncpName;
	std::vector<std::string> dialogLines;
	int currentLine;
	bool talking;
};

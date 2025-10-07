#pragma once
#include "../../Utility/Vector2D.h"
#include <string>
#include <vector>

class NCP {
public:
	NCP();
	NCP(const Vector2D& pos, const std::string& name, const std::vector<std::string>& lines);
	~NCP();

	void SetPosition(const Vector2D& pos);
	Vector2D GetPosition() const;

	void SetName(const std::string& name);
	void SetDialog(const std::vector<std::string>& lines);

	// �� �摜���蓮�Z�b�g�i���l�����Ɏg���j
	void SetImage(const std::string& path);
	void ClearImage();

	void BeginTalk();
	void AdvanceTalk();
	void EndTalk();
	bool IsTalking() const;

	bool IsPlayerInRange(const Vector2D& playerPos, float range) const;

	void Update(float delta);
	void Draw() const;

	// �ǉ������ꏊ   9/2

	void SetDrawSize(float px);
	float GetDrawSize() const;

private:
	void ReloadImage();

private:
	Vector2D position;
	std::string ncpName;
	std::vector<std::string> dialogLines;
	int currentLine;
	bool talking;

	// �摜�܂��
	int imageHandle;	   // -1: �����[�h / ���s
	std::string imagePath; // ���݂̉摜�p�X
	float drawSize;		   // �ڈ��\���T�C�Y�ipx�j
};

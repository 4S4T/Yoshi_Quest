#include "../NPC/NPC.h"
#include "DxLib.h"
#include <algorithm>

NCP::NCP()
	: position(0.0f, 0.0f), ncpName("NCP"),
	  currentLine(0), talking(false),
	  imageHandle(-1), drawSize(48.0f) {
}

NCP::NCP(const Vector2D& pos, const std::string& name, const std::vector<std::string>& lines)
	: position(pos), ncpName(name), dialogLines(lines),
	  currentLine(0), talking(false),
	  imageHandle(-1), drawSize(48.0f) {
}

NCP::~NCP() {
	if (imageHandle != -1) {
		DeleteGraph(imageHandle);
		imageHandle = -1;
	}
}

void NCP::SetPosition(const Vector2D& pos) { position = pos; }
Vector2D NCP::GetPosition() const { return position; }

void NCP::SetName(const std::string& name) { ncpName = name; }
void NCP::SetDialog(const std::vector<std::string>& lines) { dialogLines = lines; }

void NCP::SetImage(const std::string& path) {
	if (imageHandle != -1) {
		DeleteGraph(imageHandle);
		imageHandle = -1;
	}
	imagePath = path;
	ReloadImage();
}

void NCP::ClearImage() {
	if (imageHandle != -1) {
		DeleteGraph(imageHandle);
		imageHandle = -1;
	}
	imagePath.clear();
}

void NCP::ReloadImage() {
	if (imageHandle != -1) {
		DeleteGraph(imageHandle);
		imageHandle = -1;
	}
	if (!imagePath.empty()) {
		imageHandle = LoadGraph(imagePath.c_str()); // ���s: -1
	}
}

void NCP::BeginTalk() {
	if (dialogLines.empty())
		return;
	talking = true;
	currentLine = 0;
}

void NCP::AdvanceTalk() {
	if (!talking)
		return;
	currentLine++;
	if (currentLine >= (int)dialogLines.size())
		EndTalk();
}

void NCP::EndTalk() {
	talking = false;
	currentLine = 0;
}

bool NCP::IsTalking() const { return talking; }

bool NCP::IsPlayerInRange(const Vector2D& playerPos, float range) const {
	float dx = playerPos.x - position.x;
	float dy = playerPos.y - position.y;
	return (dx * dx + dy * dy) <= (range * range);
}

void NCP::Update(float /*delta*/) {
	// �A�j��������΂�����
}

void NCP::Draw() const {
	// --- �{�́i�摜 or �t�H�[���o�b�N�j ---
	if (imageHandle != -1) {
		int w = 0, h = 0;
		GetGraphSize(imageHandle, &w, &h);
		float scale = 1.0f;
		if (w > 0 && h > 0) {
			int maxSide = (w > h) ? w : h;
			scale = (maxSide > 0) ? (drawSize / (float)maxSide) : 1.0f;
		}
		// ���S��ŕ`��i����ON�j
		DrawRotaGraphF(position.x, position.y, scale, 0.0, imageHandle, TRUE);
	}
	else {
		// �摜�������Ă�������悤�Ɏl�p���o���i���܂Ō����Ă�����j
		int sz = (int)(drawSize * 0.9f);
		int x = (int)position.x, y = (int)position.y;
		DrawBox(x - sz / 2, y - sz / 2, x + sz / 2, y + sz / 2, GetColor(80, 200, 255), TRUE);
		DrawBox(x - sz / 2, y - sz / 2, x + sz / 2, y + sz / 2, GetColor(0, 40, 80), FALSE);
	}

	// --- ��bUI�i�b���Ă��鎞�����j ---
	if (!talking)
		return;

	int sw = 960, sh = 720;
	GetDrawScreenSize(&sw, &sh);

	const int boxX = 40;
	const int boxW = sw - 80;
	const int boxH = 140;
	const int boxY = sh - (boxH + 30);

	DrawBox(boxX, boxY, boxX + boxW, boxY + boxH, GetColor(20, 20, 20), TRUE);
	DrawBox(boxX, boxY, boxX + boxW, boxY + boxH, GetColor(255, 255, 255), FALSE);

	DrawBox(boxX + 12, boxY - 24, boxX + 180, boxY - 2, GetColor(20, 20, 20), TRUE);
	DrawBox(boxX + 12, boxY - 24, boxX + 180, boxY - 2, GetColor(255, 255, 255), FALSE);
	DrawString(boxX + 20, boxY - 22, ncpName.c_str(), GetColor(255, 255, 0));

	if (currentLine >= 0 && currentLine < (int)dialogLines.size()) {
		DrawString(boxX + 20, boxY + 20, dialogLines[currentLine].c_str(), GetColor(230, 230, 230));
	}

	DrawString(boxX + boxW - 160, boxY + boxH - 24, "Enter�F������", GetColor(180, 180, 180));
}
//�ǉ������ꏊ   9/2

void NCP::SetDrawSize(float px) 
{
	if (px <= 0.0f)
		px = 5.0f;
	drawSize = px;
}

float NCP::GetDrawSize() const
{ 
	return drawSize; 
}
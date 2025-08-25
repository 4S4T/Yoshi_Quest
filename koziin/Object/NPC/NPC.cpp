#include "../NPC/NPC.h"
#include "DxLib.h"
#include <algorithm>

NCP::NCP()
	: position(0.0f, 0.0f), ncpName("NCP"),
	  currentLine(0), talking(false)
{


}

NCP::NCP(const Vector2D& pos, const std::string& name, const std::vector<std::string>& lines)
	: position(pos), ncpName(name), dialogLines(lines),
	  currentLine(0), talking(false) 
{

}

void NCP::SetPosition(const Vector2D& pos) 
{
	position = pos; 
}

Vector2D NCP::GetPosition() const 
{
	return position; 
}

void NCP::SetName(const std::string& name)
{
	ncpName = name;
}

void NCP::SetDialog(const std::vector<std::string>& lines) 
{
	dialogLines = lines; 
}

void NCP::BeginTalk() 
{
	if (dialogLines.empty())
		return;
	talking = true;
	currentLine = 0;
}

void NCP::AdvanceTalk()
{
	if (!talking)
		return;
	currentLine++;
	if (currentLine >= (int)dialogLines.size())
	{
		EndTalk();
	}
}

void NCP::EndTalk() 
{
	talking = false;
	currentLine = 0;
}

bool NCP::IsTalking() const
{
	return talking; 
}

bool NCP::IsPlayerInRange(const Vector2D& playerPos, float range) const
{
	float dx = playerPos.x - position.x;
	float dy = playerPos.y - position.y;
	return (dx * dx + dy * dy) <= (range * range);
}

void NCP::Update(float /*delta*/) 
{
	/* 今は特になし（将来アニメ等）*/ 
}

void NCP::Draw() const 
{
	// 本体（簡易）：空色の丸
	DrawCircle((int)position.x, (int)position.y, 12, GetColor(180, 230, 255), TRUE);
	DrawCircle((int)position.x, (int)position.y, 12, GetColor(30, 30, 60), FALSE);

	if (!talking)
		return;

	// 画面サイズ取得（失敗時は 960x720 を仮）
	int sw = 960, sh = 720;
	GetDrawScreenSize(&sw, &sh);

	const int boxX = 40;
	const int boxW = sw - 80;
	const int boxH = 140;
	const int boxY = sh - (boxH + 30);

	// 吹き出し（下部メッセージウィンドウ）
	DrawBox(boxX, boxY, boxX + boxW, boxY + boxH, GetColor(20, 20, 20), TRUE);
	DrawBox(boxX, boxY, boxX + boxW, boxY + boxH, GetColor(255, 255, 255), FALSE);

	// 名前欄
	DrawBox(boxX + 12, boxY - 24, boxX + 180, boxY - 2, GetColor(20, 20, 20), TRUE);
	DrawBox(boxX + 12, boxY - 24, boxX + 180, boxY - 2, GetColor(255, 255, 255), FALSE);
	DrawString(boxX + 20, boxY - 22, ncpName.c_str(), GetColor(255, 255, 0));

	// 本文（現在行のみシンプル表示）
	if (currentLine >= 0 && currentLine < (int)dialogLines.size()) {
		DrawString(boxX + 20, boxY + 20, dialogLines[currentLine].c_str(), GetColor(230, 230, 230));
	}

	// 操作ヒント
	DrawString(boxX + boxW - 160, boxY + boxH - 24, "Enter：すすむ", GetColor(180, 180, 180));
}

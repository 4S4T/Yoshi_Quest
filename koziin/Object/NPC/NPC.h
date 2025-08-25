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

	void BeginTalk();	// 会話開始（1行目を表示）
	void AdvanceTalk(); // 次の行へ（最後で自動終了）
	void EndTalk();		// 会話強制終了
	bool IsTalking() const;

	// プレイヤーが一定距離以内か
	bool IsPlayerInRange(const Vector2D& playerPos, float range) const;

	// 1フレーム更新・描画
	void Update(float delta);
	void Draw() const;

private:
	Vector2D position;
	std::string ncpName;
	std::vector<std::string> dialogLines;
	int currentLine;
	bool talking;
};

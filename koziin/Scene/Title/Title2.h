#pragma once

#include "../SceneBase.h"

class TitleScene2 : public SceneBase {
private:
	int cursorSound2 = -1;
	int cursor_SE = -1;

public:
	TitleScene2();
	~TitleScene2();

	virtual void Initialize() override;
	int TitleImage;
	int Title_name;
	int select;
	int menu_cursor;
	

	virtual eSceneType Update(float delta_second) override;
	virtual void Draw() override;
	virtual void Finalize() override;
	virtual eSceneType GetNowSceneType() const override;
};

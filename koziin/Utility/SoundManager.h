#pragma once
#include "DxLib.h"
#include <string>

class SoundManager {
private:
	int currentBgmHandle = -1;	  // 今再生しているBGM
	std::string currentPath = ""; // 現在の音源パス

	SoundManager() {}
	~SoundManager() {}

public:
	static SoundManager& GetInstance() {
		static SoundManager instance;
		return instance;
	}

	// BGM 再生（同じ曲なら何もしない）
	void PlayBGM(const std::string& path, int volume = 255) {
		if (currentPath == path && CheckSoundMem(currentBgmHandle) == 1)
			return; // 既に再生中ならスキップ

		// 既存停止
		if (currentBgmHandle != -1) {
			StopSoundMem(currentBgmHandle);
			DeleteSoundMem(currentBgmHandle);
		}

		currentBgmHandle = LoadSoundMem(path.c_str());
		currentPath = path;

		if (currentBgmHandle != -1) {
			ChangeVolumeSoundMem(volume, currentBgmHandle);
			PlaySoundMem(currentBgmHandle, DX_PLAYTYPE_LOOP);
		}
	}

	void StopBGM() {
		if (currentBgmHandle != -1) {
			StopSoundMem(currentBgmHandle);
		}
	}

	// 効果音を一度だけ再生する（BGMとは別管理）
	void PlaySE(const std::string& path, int volume = 255) {
		int seHandle = LoadSoundMem(path.c_str());
		if (seHandle != -1) {
			ChangeVolumeSoundMem(volume, seHandle);
			PlaySoundMem(seHandle, DX_PLAYTYPE_BACK); // ← BACK = 同時再生OK
		}
	}
};

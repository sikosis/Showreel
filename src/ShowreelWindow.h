#ifndef SHOWREEL_WINDOW_H
#define SHOWREEL_WINDOW_H

#include <Window.h>

#include "EditorDriver.h"
#include "SourceFileDiscovery.h"

#include <String.h>

#include <vector>


class BButton;
class BCheckBox;
class BFilePanel;
class BListView;
class BMessageRunner;
class BSlider;
class BStringView;


class ShowreelWindow : public BWindow {
public:
	ShowreelWindow();
	~ShowreelWindow() override;

	void MessageReceived(BMessage* message) override;
	bool QuitRequested() override;

private:
	enum class PlayerState {
		Idle,
		TopDwell,
		Scrolling,
		BottomDwell
	};

	void AdvanceToFile(int32 direction);
	void ChooseProjectFolder();
	void CloseCurrentDocument();
	void ClearPlaylist();
	int32 FindViewableFile(int32 startIndex, int32 direction) const;
	void FinishPlayback();
	void HandlePlaybackError(const char* operation, status_t status);
	void OpenSelectedFile();
	void PausePlayback();
	void PlaybackTick();
	void PopulatePlaylist(const char* folderPath);
	void ProjectFolderChosen(BMessage* message);
	void StartFile(int32 index);
	void StopPlayback();
	bigtime_t BottomDwell() const;
	float ScrollLinesPerSecond() const;
	bigtime_t TopDwell() const;
	void UpdatePlaybackControls();
	void UpdateTimingLabels();

	BFilePanel* fFolderPanel;
	BStringView* fFolderPathView;
	BListView* fPlaylistView;
	BButton* fPlayButton;
	BButton* fPauseButton;
	BButton* fPreviousButton;
	BButton* fNextButton;
	BButton* fStopButton;
	BCheckBox* fCloseShowreelCheckBox;
	BStringView* fPlaybackStatusView;
	BSlider* fTopDwellSlider;
	BSlider* fScrollSpeedSlider;
	BSlider* fBottomDwellSlider;
	EditorDriver* fEditorDriver;
	BMessageRunner* fPlaybackRunner;
	EditorDocument fCurrentDocument;
	PlayerState fPlayerState;
	bigtime_t fStateDeadline;
	bigtime_t fPauseStarted;
	int32 fCurrentIndex;
	bool fPaused;
	std::vector<DiscoveredFile> fPlaylistFiles;
};

#endif

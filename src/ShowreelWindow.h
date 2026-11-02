#ifndef SHOWREEL_WINDOW_H
#define SHOWREEL_WINDOW_H

#include <Window.h>

#include "EditorDriver.h"
#include "SourceFileDiscovery.h"

#include <String.h>

#include <ctime>
#include <map>
#include <string>
#include <sys/types.h>
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
	static constexpr uint32 kStartPlayback = 'opsf';
	static constexpr uint32 kSetProjectFolder = 'spfd';

	ShowreelWindow();
	~ShowreelWindow() override;

	void MessageReceived(BMessage* message) override;
	bool QuitRequested() override;
	void SetCloseWhenFinished(bool closeWhenFinished);
	void SetMonitorMode(bool monitorMode);
	void SetProjectFolder(const char* folderPath);
	void SetTiming(float topDwellSeconds, int32 scrollLinesPerSecond,
		float bottomDwellSeconds);

private:
	enum class PlayerState {
		Idle,
		TopDwell,
		Scrolling,
		BottomDwell
	};

	struct MonitoredFileState {
		time_t modificationSeconds;
		long modificationNanoseconds;
		off_t size;
	};

	struct PendingMonitorFile {
		BString path;
		bigtime_t readyAt;
	};

	void AdvanceToFile(int32 direction);
	void ChooseProjectFolder();
	void CloseCurrentDocument();
	void ClearPlaylist();
	int32 FindViewableFile(int32 startIndex, int32 direction) const;
	void FinishPlayback();
	void HandlePlaybackError(const char* operation, status_t status);
	int32 IndexOfFile(const char* path) const;
	void MonitorModeChanged();
	void MonitorTick();
	void OpenSelectedFile();
	void PausePlayback();
	void PlaybackTick();
	void PopulatePlaylist(const char* folderPath);
	void ProjectFolderChosen(BMessage* message);
	void StartFile(int32 index);
	void StartMonitorRunner();
	void StartMonitoredFile(const char* path);
	void StopPlayback();
	void StopMonitorRunner();
	void UpdateMonitoredFiles(bool queueChanges);
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
	BCheckBox* fMonitorModeCheckBox;
	BStringView* fPlaybackStatusView;
	BSlider* fTopDwellSlider;
	BSlider* fScrollSpeedSlider;
	BSlider* fBottomDwellSlider;
	EditorDriver* fEditorDriver;
	BMessageRunner* fPlaybackRunner;
	BMessageRunner* fMonitorRunner;
	EditorDocument fCurrentDocument;
	PlayerState fPlayerState;
	bigtime_t fStateDeadline;
	bigtime_t fPauseStarted;
	int32 fCurrentIndex;
	bool fPaused;
	bool fMonitorPlayback;
	BString fProjectFolderPath;
	std::map<std::string, MonitoredFileState> fMonitoredFiles;
	std::vector<PendingMonitorFile> fPendingMonitorFiles;
	std::vector<DiscoveredFile> fPlaylistFiles;
};

#endif

#include "ShowreelWindow.h"

#include "EditorDriver.h"
#include "GlobeDriver.h"
#include "GlobeProtocol.h"
#include "SourceFileDiscovery.h"

#include <Alert.h>
#include <Application.h>
#include <Button.h>
#include <CheckBox.h>
#include <Entry.h>
#include <FilePanel.h>
#include <GroupLayout.h>
#include <GroupView.h>
#include <InterfaceDefs.h>
#include <ListView.h>
#include <Message.h>
#include <MessageRunner.h>
#include <OS.h>
#include <Path.h>
#include <ScrollView.h>
#include <Slider.h>
#include <StringItem.h>
#include <StringView.h>

#include <algorithm>
#include <cstring>
#include <sys/stat.h>


namespace {
	const uint32 kChooseProjectFolder = 'chpf';
	const uint32 kProjectFolderChosen = 'pfch';
	const uint32 kPausePlayback = 'pasp';
	const uint32 kPreviousFile = 'prvf';
	const uint32 kNextFile = 'nxtf';
	const uint32 kStopPlayback = 'stpp';
	const uint32 kPlaybackTick = 'ptck';
	const uint32 kTimingChanged = 'tmch';
	const uint32 kMonitorModeChanged = 'mmch';
	const uint32 kMonitorTick = 'mntk';
	const uint32 kShowAbout = 'sabt';
	const bigtime_t kPlaybackTickInterval = 50000;
	const bigtime_t kMonitorTickInterval = 1000000;
	const bigtime_t kMonitorDisplayDelay = 5000000;
	const int32 kDefaultTopDwellTenths = 50;
	const int32 kDefaultScrollLinesPerSecond = 8;
	const int32 kDefaultBottomDwellTenths = 50;
	const int32 kDefaultFontSize = 16;
}


ShowreelWindow::ShowreelWindow() : BWindow(BRect(5, 28, 730, 470), "ShowReel v0.48", B_TITLED_WINDOW,	B_AUTO_UPDATE_SIZE_LIMITS),
	fFolderPanel(NULL),
	fFolderPathView(NULL),
	fPlaylistView(NULL),
	fPlayButton(NULL),
	fPauseButton(NULL),
	fPreviousButton(NULL),
	fNextButton(NULL),
	fStopButton(NULL),
	fCloseShowreelCheckBox(NULL),
	fMonitorModeCheckBox(NULL),
	fPlaybackStatusView(NULL),
	fTopDwellSlider(NULL),
	fScrollSpeedSlider(NULL),
	fBottomDwellSlider(NULL),
	fFontSizeSlider(NULL),
	fEditorDriver(new GlobeDriver()),
	fPlaybackRunner(NULL),
	fMonitorRunner(NULL),
	fPlayerState(PlayerState::Idle),
	fStateDeadline(0),
	fPauseStarted(0),
	fCurrentIndex(-1),
	fPaused(false),
	fMonitorPlayback(false) {
	fCurrentDocument.session = 0;
	BButton* chooseButton = new BButton("Choose Project Folder" B_UTF8_ELLIPSIS,
		new BMessage(kChooseProjectFolder));
	fFolderPathView = new BStringView("folder path", "No Project Folder Selected");

	fPlaylistView = new BListView("playlist");
	BScrollView* playlistScrollView = new BScrollView("playlist scroll view",
		fPlaylistView, 0, false, true);

	fPlayButton = new BButton("Play", new BMessage(kStartPlayback));
	fPlayButton->SetEnabled(false);
	fPauseButton = new BButton("Pause", new BMessage(kPausePlayback));
	fPreviousButton = new BButton("Previous", new BMessage(kPreviousFile));
	fNextButton = new BButton("Next", new BMessage(kNextFile));
	fStopButton = new BButton("Stop", new BMessage(kStopPlayback));
	fPauseButton->SetEnabled(false);
	fPreviousButton->SetEnabled(false);
	fNextButton->SetEnabled(false);
	fStopButton->SetEnabled(false);
	BButton* aboutButton = new BButton("About", new BMessage(kShowAbout));
	fTopDwellSlider = new BSlider("top dwell", "", new BMessage(kTimingChanged), 0, 100, B_HORIZONTAL);
	fTopDwellSlider->SetValue(kDefaultTopDwellTenths);
	fScrollSpeedSlider = new BSlider("scroll speed", "", new BMessage(kTimingChanged), 2, 40, B_HORIZONTAL);
	fScrollSpeedSlider->SetValue(kDefaultScrollLinesPerSecond);
	fBottomDwellSlider = new BSlider("bottom dwell", "", new BMessage(kTimingChanged), 0, 300, B_HORIZONTAL);
	fBottomDwellSlider->SetValue(kDefaultBottomDwellTenths);
	fFontSizeSlider = new BSlider("font size", "", new BMessage(kTimingChanged),
		10, 28, B_HORIZONTAL);
	fFontSizeSlider->SetValue(kDefaultFontSize);
	fCloseShowreelCheckBox = new BCheckBox("close showreel", "Close ShowReel when Finished", NULL);
	fMonitorModeCheckBox = new BCheckBox("monitor mode", "Monitor Mode",
		new BMessage(kMonitorModeChanged));
	fMonitorModeCheckBox->SetEnabled(false);
	fPlaybackStatusView = new BStringView("playback status", "Ready");
	UpdateTimingLabels();

	BGroupView* controls = new BGroupView(B_VERTICAL, 8.0f);
	controls->AddChild(new BStringView("timing heading", "Timing"));
	controls->AddChild(fPlaybackStatusView);
	controls->AddChild(fTopDwellSlider);
	controls->AddChild(fScrollSpeedSlider);
	controls->AddChild(fBottomDwellSlider);
	controls->AddChild(fFontSizeSlider);
	controls->AddChild(fMonitorModeCheckBox);
	controls->AddChild(fCloseShowreelCheckBox);
	controls->AddChild(fPlayButton);
	controls->AddChild(fPauseButton);
	controls->AddChild(fPreviousButton);
	controls->AddChild(fNextButton);
	controls->AddChild(fStopButton);
	controls->AddChild(aboutButton);

	BGroupView* folderRow = new BGroupView(B_HORIZONTAL, 8.0f);
	folderRow->AddChild(chooseButton);
	folderRow->AddChild(fFolderPathView);

	BGroupView* playlistRow = new BGroupView(B_HORIZONTAL, 8.0f);
	playlistRow->AddChild(playlistScrollView);
	playlistRow->AddChild(controls);

	BGroupView* contentView = new BGroupView(B_VERTICAL, 8.0f);
	contentView->GroupLayout()->SetInsets(10.0f, 10.0f, 10.0f, 10.0f);
	contentView->AddChild(folderRow);
	contentView->AddChild(playlistRow);

	SetLayout(new BGroupLayout(B_VERTICAL));
	AddChild(contentView);
	AddShortcut(B_ESCAPE, 0, new BMessage(kStopPlayback));
}
//---------------------------------------------------------------------------------------------------------------------------------//


ShowreelWindow::~ShowreelWindow() {
	StopMonitorRunner();
	StopPlayback();
	delete fFolderPanel;
	delete fEditorDriver;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::MessageReceived(BMessage* message) {
	switch (message->what) {
		case kChooseProjectFolder:
			ChooseProjectFolder();
			break;
		case kProjectFolderChosen:
			ProjectFolderChosen(message);
			break;
		case kStartPlayback:
			OpenSelectedFile();
			break;
		case kSetProjectFolder:
		{
			const char* folderPath = NULL;
			if (message->FindString("path", &folderPath) == B_OK)
				SetProjectFolder(folderPath);
			break;
		}
		case kPausePlayback:
			PausePlayback();
			break;
		case kPreviousFile:
			AdvanceToFile(-1);
			break;
		case kNextFile:
			AdvanceToFile(1);
			break;
		case kStopPlayback:
			StopPlayback();
			break;
		case kMessageShowreelStop:
			StopPlayback();
			Activate(true);
			break;
		case kPlaybackTick:
			PlaybackTick();
			break;
		case kTimingChanged:
			UpdateTimingLabels();
			break;
		case kMonitorModeChanged:
			MonitorModeChanged();
			break;
		case kMonitorTick:
			MonitorTick();
			break;
		case kShowAbout:
			be_app->AboutRequested();
			break;
		default:
			BWindow::MessageReceived(message);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool ShowreelWindow::QuitRequested() {
	StopPlayback();
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::SetCloseWhenFinished(bool closeWhenFinished) {
	fCloseShowreelCheckBox->SetValue(closeWhenFinished ? B_CONTROL_ON : B_CONTROL_OFF);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::SetFontSize(int32 fontSize) {
	fFontSizeSlider->SetValue(fontSize);
	UpdateTimingLabels();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::SetMonitorMode(bool monitorMode) {
	fMonitorModeCheckBox->SetValue(monitorMode
		? B_CONTROL_ON : B_CONTROL_OFF);
	MonitorModeChanged();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::SetProjectFolder(const char* folderPath) {
	if (folderPath == NULL || folderPath[0] == '\0')
		return;

	PopulatePlaylist(folderPath);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::SetTiming(float topDwellSeconds,
	int32 scrollLinesPerSecond, float bottomDwellSeconds) {
	fTopDwellSlider->SetValue(static_cast<int32>(topDwellSeconds * 10.0f + 0.5f));
	fScrollSpeedSlider->SetValue(scrollLinesPerSecond);
	fBottomDwellSlider->SetValue(static_cast<int32>(bottomDwellSeconds * 10.0f + 0.5f));
	UpdateTimingLabels();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::AdvanceToFile(int32 direction) {
	if (fPlayerState == PlayerState::Idle)
		return;

	int32 nextIndex = FindViewableFile(fCurrentIndex + direction, direction);
	if (nextIndex < 0) {
		FinishPlayback();
		return;
	}

	StartFile(nextIndex);
}
//---------------------------------------------------------------------------------------------------------------------------------//


bigtime_t ShowreelWindow::BottomDwell() const {
	return static_cast<bigtime_t>(fBottomDwellSlider->Value()) * 100000;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::ChooseProjectFolder() {
	if (fFolderPanel == NULL) {
		BMessenger target(this);
		fFolderPanel = new BFilePanel(B_OPEN_PANEL, &target, NULL, B_DIRECTORY_NODE, false, new BMessage(kProjectFolderChosen));
	}

	fFolderPanel->Show();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::CloseCurrentDocument() {
	if (fCurrentDocument.session != 0)
		fEditorDriver->CloseDocument(fCurrentDocument);

	fCurrentDocument.session = 0;
	fCurrentDocument.target = BMessenger();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::ClearPlaylist() {
	StopPlayback();
	while (fPlaylistView->CountItems() > 0)
		delete fPlaylistView->RemoveItem(static_cast<int32>(0));

	fPlaylistFiles.clear();
	fPlayButton->SetEnabled(false);
}
//---------------------------------------------------------------------------------------------------------------------------------//


int32 ShowreelWindow::FindViewableFile(int32 startIndex, int32 direction) const {
	if (direction == 0)
		return -1;

	for (int32 index = startIndex; index >= 0 && index < static_cast<int32>(fPlaylistFiles.size()); index += direction) {
		if (!fPlaylistFiles[index].skipped)
			return index;
	}

	return -1;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::FinishPlayback() {
	StopPlayback();
	fEditorDriver->QuitEditor();
	if (fCloseShowreelCheckBox->Value() == B_CONTROL_ON)
		PostMessage(B_QUIT_REQUESTED);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::HandlePlaybackError(const char* operation,
	status_t status) {
	StopPlayback();
	BString message(operation);
	message << ": " << strerror(status);
	(new BAlert("Playback stopped", message.String(), "OK", NULL, NULL,	B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
}
//---------------------------------------------------------------------------------------------------------------------------------//


int32 ShowreelWindow::IndexOfFile(const char* path) const {
	for (int32 index = 0;
		index < static_cast<int32>(fPlaylistFiles.size()); ++index) {
		if (fPlaylistFiles[index].path == path)
			return index;
	}

	return -1;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::MonitorModeChanged() {
	if (fMonitorModeCheckBox->Value() == B_CONTROL_ON
		&& !fProjectFolderPath.IsEmpty()) {
		StartMonitorRunner();
		return;
	}

	StopMonitorRunner();
	if (fPlayerState == PlayerState::Idle)
		fPlaybackStatusView->SetText("Ready");
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::MonitorTick() {
	if (fMonitorModeCheckBox->Value() != B_CONTROL_ON
		|| fProjectFolderPath.IsEmpty()) {
		return;
	}

	UpdateMonitoredFiles(true);
	if (fPlayerState != PlayerState::Idle || fPendingMonitorFiles.empty())
		return;

	auto nextFile = std::min_element(fPendingMonitorFiles.begin(),
		fPendingMonitorFiles.end(),
		[](const PendingMonitorFile& left, const PendingMonitorFile& right) {
			return left.readyAt < right.readyAt;
		});
	bigtime_t now = system_time();
	if (nextFile->readyAt > now) {
		BPath path(nextFile->path.String());
		BString status("Changed: ");
		status << path.Leaf() << " (displaying in "
			<< static_cast<int32>((nextFile->readyAt - now
				+ 999999) / 1000000) << " s)";
		fPlaybackStatusView->SetText(status.String());
		return;
	}

	BString path(nextFile->path);
	fPendingMonitorFiles.erase(nextFile);
	StartMonitoredFile(path.String());
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::OpenSelectedFile() {
	int32 selectedIndex = fPlaylistView->CurrentSelection();
	if (selectedIndex < 0 || selectedIndex >= static_cast<int32>(fPlaylistFiles.size())	|| fPlaylistFiles[selectedIndex].skipped) {
		selectedIndex = FindViewableFile(0, 1);
	}

	if (selectedIndex < 0 || selectedIndex >= static_cast<int32>(fPlaylistFiles.size())) {
		return;
	}

	fMonitorPlayback = false;
	StartFile(selectedIndex);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::PausePlayback() {
	if (fPlayerState == PlayerState::Idle)
		return;

	bigtime_t now = system_time();
	if (fPaused) {
		fStateDeadline += now - fPauseStarted;
		fPaused = false;
		fPlaybackStatusView->SetText("Resuming");
	} else {
		fPauseStarted = now;
		fPaused = true;
		fPlaybackStatusView->SetText("Paused");
	}
	UpdatePlaybackControls();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::PlaybackTick() {
	if (fPlayerState == PlayerState::Idle || fPaused)
		return;

	bigtime_t now = system_time();
	if (fPlayerState == PlayerState::TopDwell) {
		if (now < fStateDeadline) {
			BString status;
			status.SetToFormat("Waiting at top: %.1f s", static_cast<double>(fStateDeadline - now) / 1000000.0);
			fPlaybackStatusView->SetText(status.String());
			return;
		}

		EditorScrollState state;
		status_t status = fEditorDriver->GetScrollState(fCurrentDocument, state);
		if (status != B_OK) {
			HandlePlaybackError("Could not read Globe's scroll state", status);
			return;
		}
		if (!state.ready)
			return;
		if (state.maximum <= state.minimum) {
			fPlayerState = PlayerState::BottomDwell;
			fStateDeadline = now + BottomDwell();
			fPlaybackStatusView->SetText(
				"File fits in Globe; no scrolling needed");
		} else
			fPlayerState = PlayerState::Scrolling;
		return;
	}

	if (fPlayerState == PlayerState::Scrolling) {
		EditorScrollState state;
		status_t status = fEditorDriver->GetScrollState(fCurrentDocument, state);
		if (status != B_OK) {
			HandlePlaybackError("Could not read Globe's scroll state", status);
			return;
		}

		float lineHeight = state.lineHeight > 0.0f ? state.lineHeight : 16.0f;
		float step = ScrollLinesPerSecond() * lineHeight * static_cast<float>(kPlaybackTickInterval) / 1000000.0f;
		float nextValue = std::min(state.value + step, state.maximum);
		status = fEditorDriver->SetScrollValue(fCurrentDocument, nextValue,	state);
		if (status != B_OK) {
			HandlePlaybackError("Could not scroll the Globe document", status);
			return;
		}
		BString playbackStatus;
		playbackStatus.SetToFormat("Scrolling: %.0f / %.0f px",	static_cast<double>(state.value), static_cast<double>(state.maximum));
		fPlaybackStatusView->SetText(playbackStatus.String());
		if (state.value >= state.maximum - 0.5f) {
			fPlayerState = PlayerState::BottomDwell;
			fStateDeadline = now + BottomDwell();
			fPlaybackStatusView->SetText("Waiting at bottom");
		}
		return;
	}

	if (fPlayerState == PlayerState::BottomDwell && now >= fStateDeadline) {
		if (fMonitorPlayback)
			StopPlayback();
		else
			AdvanceToFile(1);
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::PopulatePlaylist(const char* folderPath) {
	StopMonitorRunner();
	ClearPlaylist();
	fProjectFolderPath = "";

	SourceFileDiscovery discovery;
	status_t status = discovery.Discover(folderPath, fPlaylistFiles);
	if (status != B_OK) {
		BString folderLabel(folderPath);
		folderLabel << " (could not read folder)";
		fFolderPathView->SetText(folderLabel.String());
		fMonitorModeCheckBox->SetEnabled(false);
		return;
	}
	fProjectFolderPath = folderPath;

	BString rootPrefix(folderPath);
	if (!rootPrefix.EndsWith("/"))
		rootPrefix << "/";

	for (const DiscoveredFile& file : fPlaylistFiles) {
		BString displayPath(file.path);
		if (displayPath.StartsWith(rootPrefix))
			displayPath.Remove(0, rootPrefix.Length());
		BStringItem* item = new BStringItem(displayPath.String());
		item->SetEnabled(!file.skipped);
		fPlaylistView->AddItem(item);
	}

	BString folderLabel(folderPath);
	folderLabel << " (" << static_cast<int32>(fPlaylistFiles.size()) << " files)";
	fFolderPathView->SetText(folderLabel.String());
	bool hasViewableFile = false;
	for (const DiscoveredFile& file : fPlaylistFiles) {
		if (!file.skipped) {
			hasViewableFile = true;
			break;
		}
	}
	fPlayButton->SetEnabled(hasViewableFile);
	fMonitorModeCheckBox->SetEnabled(true);
	if (fMonitorModeCheckBox->Value() == B_CONTROL_ON)
		StartMonitorRunner();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::ProjectFolderChosen(BMessage* message) {
	entry_ref folderReference;
	if (message->FindRef("refs", &folderReference) != B_OK)
		return;

	BPath folderPath(&folderReference);
	if (folderPath.InitCheck() != B_OK)
		return;

	PopulatePlaylist(folderPath.Path());
}
//---------------------------------------------------------------------------------------------------------------------------------//


float ShowreelWindow::ScrollLinesPerSecond() const {
	return static_cast<float>(fScrollSpeedSlider->Value());
}
//---------------------------------------------------------------------------------------------------------------------------------//


float ShowreelWindow::PresentationFontSize() const {
	return static_cast<float>(fFontSizeSlider->Value());
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::StartFile(int32 index) {
	if (index < 0 || index >= static_cast<int32>(fPlaylistFiles.size())	|| fPlaylistFiles[index].skipped) {
		return;
	}

	CloseCurrentDocument();
	status_t status = fEditorDriver->OpenDocument(
		fPlaylistFiles[index].path.String(), PresentationFontSize(),
		fCurrentDocument);
	if (status != B_OK) {
		if (fMonitorPlayback) {
			StopPlayback();
			return;
		}
		HandlePlaybackError("Globe could not open the selected file", status);
		return;
	}

	fCurrentIndex = index;
	fPlaylistView->Select(index);
	fPlaylistView->ScrollToSelection();
	fPlayerState = PlayerState::TopDwell;
	fStateDeadline = system_time() + TopDwell();
	fPaused = false;
	fPlaybackStatusView->SetText("Waiting at top");

	if (fPlaybackRunner == NULL) {
		BMessage tickMessage(kPlaybackTick);
		fPlaybackRunner = new BMessageRunner(BMessenger(this), &tickMessage,
			kPlaybackTickInterval);
	}
	UpdatePlaybackControls();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::StartMonitorRunner() {
	if (fProjectFolderPath.IsEmpty() || fMonitorRunner != NULL)
		return;

	UpdateMonitoredFiles(false);
	BMessage tickMessage(kMonitorTick);
	fMonitorRunner = new BMessageRunner(BMessenger(this), &tickMessage,
		kMonitorTickInterval);
	if (fPlayerState == PlayerState::Idle)
		fPlaybackStatusView->SetText("Monitoring for changes");
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::StartMonitoredFile(const char* path) {
	int32 index = IndexOfFile(path);
	if (index < 0) {
		BString projectFolder(fProjectFolderPath);
		PopulatePlaylist(projectFolder.String());
		index = IndexOfFile(path);
	}
	if (index < 0 || fPlaylistFiles[index].skipped)
		return;

	fMonitorPlayback = true;
	StartFile(index);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::StopPlayback() {
	delete fPlaybackRunner;
	fPlaybackRunner = NULL;
	CloseCurrentDocument();
	fPlayerState = PlayerState::Idle;
	fCurrentIndex = -1;
	fPaused = false;
	fMonitorPlayback = false;
	fPlaybackStatusView->SetText(fMonitorRunner != NULL
		? "Monitoring for changes" : "Ready");
	UpdatePlaybackControls();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::StopMonitorRunner() {
	delete fMonitorRunner;
	fMonitorRunner = NULL;
	fMonitoredFiles.clear();
	fPendingMonitorFiles.clear();
}
//---------------------------------------------------------------------------------------------------------------------------------//


bigtime_t ShowreelWindow::TopDwell() const {
	return static_cast<bigtime_t>(fTopDwellSlider->Value()) * 100000;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::UpdateMonitoredFiles(bool queueChanges) {
	SourceFileDiscovery discovery;
	std::vector<DiscoveredFile> files;
	if (discovery.Discover(fProjectFolderPath.String(), files) != B_OK)
		return;

	std::map<std::string, MonitoredFileState> currentFiles;
	bigtime_t readyAt = system_time() + kMonitorDisplayDelay;
	for (const DiscoveredFile& file : files) {
		if (file.skipped)
			continue;

		BEntry entry(file.path.String(), true);
		struct stat fileInformation;
		if (entry.GetStat(&fileInformation) != B_OK)
			continue;

		std::string path(file.path.String());
		MonitoredFileState state;
		state.modificationSeconds = fileInformation.st_mtim.tv_sec;
		state.modificationNanoseconds = fileInformation.st_mtim.tv_nsec;
		state.size = fileInformation.st_size;
		currentFiles[path] = state;

		auto previous = fMonitoredFiles.find(path);
		bool changed = previous == fMonitoredFiles.end()
			|| previous->second.modificationSeconds
				!= state.modificationSeconds
			|| previous->second.modificationNanoseconds
				!= state.modificationNanoseconds
			|| previous->second.size != state.size;
		if (!queueChanges || !changed)
			continue;

		bool alreadyPending = false;
		for (PendingMonitorFile& pending : fPendingMonitorFiles) {
			if (pending.path == file.path) {
				pending.readyAt = readyAt;
				alreadyPending = true;
				break;
			}
		}
		if (!alreadyPending) {
			PendingMonitorFile pending;
			pending.path = file.path;
			pending.readyAt = readyAt;
			fPendingMonitorFiles.push_back(pending);
		}
	}

	fPendingMonitorFiles.erase(std::remove_if(fPendingMonitorFiles.begin(),
		fPendingMonitorFiles.end(),
		[&currentFiles](const PendingMonitorFile& pending) {
			return currentFiles.find(pending.path.String())
				== currentFiles.end();
		}), fPendingMonitorFiles.end());
	fMonitoredFiles.swap(currentFiles);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::UpdatePlaybackControls() {
	bool playing = fPlayerState != PlayerState::Idle;
	bool hasViewableFile = FindViewableFile(0, 1) >= 0;
	fPlayButton->SetEnabled(!playing && hasViewableFile);
	fPauseButton->SetEnabled(playing);
	fPreviousButton->SetEnabled(playing && !fMonitorPlayback
		&& FindViewableFile(fCurrentIndex - 1, -1) >= 0);
	fNextButton->SetEnabled(playing && !fMonitorPlayback
		&& FindViewableFile(fCurrentIndex + 1, 1) >= 0);
	fStopButton->SetEnabled(playing);
	fPauseButton->SetLabel(fPaused ? "Resume" : "Pause");
	fTopDwellSlider->SetEnabled(!playing);
	fScrollSpeedSlider->SetEnabled(!playing);
	fBottomDwellSlider->SetEnabled(!playing);
	fFontSizeSlider->SetEnabled(!playing);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::UpdateTimingLabels() {
	BString label;
	label.SetToFormat("Top dwell: %.1f s", static_cast<double>(fTopDwellSlider->Value()) / 10.0);
	fTopDwellSlider->SetLabel(label.String());
	label.SetToFormat("Scroll speed: %ld lines/s", static_cast<long>(fScrollSpeedSlider->Value()));
	fScrollSpeedSlider->SetLabel(label.String());
	label.SetToFormat("Bottom dwell: %.1f s", static_cast<double>(fBottomDwellSlider->Value()) / 10.0);
	fBottomDwellSlider->SetLabel(label.String());
	label.SetToFormat("Font size: %ld pt",
		static_cast<long>(fFontSizeSlider->Value()));
	fFontSizeSlider->SetLabel(label.String());
}
//---------------------------------------------------------------------------------------------------------------------------------//


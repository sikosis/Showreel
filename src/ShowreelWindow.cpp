#include "ShowreelWindow.h"

#include "EditorDriver.h"
#include "GlobeDriver.h"
#include "SourceFileDiscovery.h"

#include <Alert.h>
#include <Application.h>
#include <Button.h>
#include <FilePanel.h>
#include <GroupLayout.h>
#include <GroupView.h>
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


namespace {
	const uint32 kChooseProjectFolder = 'chpf';
	const uint32 kProjectFolderChosen = 'pfch';
	const uint32 kOpenSelectedFile = 'opsf';
	const uint32 kPausePlayback = 'pasp';
	const uint32 kPreviousFile = 'prvf';
	const uint32 kNextFile = 'nxtf';
	const uint32 kStopPlayback = 'stpp';
	const uint32 kPlaybackTick = 'ptck';
	const uint32 kTimingChanged = 'tmch';
	const uint32 kShowAbout = 'sabt';
	const bigtime_t kPlaybackTickInterval = 50000;
	const int32 kDefaultTopDwellTenths = 15;
	const int32 kDefaultScrollLinesPerSecond = 8;
	const int32 kDefaultBottomDwellTenths = 30;
}


ShowreelWindow::ShowreelWindow()
	:
	BWindow(BRect(100, 100, 820, 560), "Showreel v0.32", B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS),
	fFolderPanel(NULL),
	fFolderPathView(NULL),
	fPlaylistView(NULL),
	fPlayButton(NULL),
	fPauseButton(NULL),
	fPreviousButton(NULL),
	fNextButton(NULL),
	fStopButton(NULL),
	fTopDwellSlider(NULL),
	fScrollSpeedSlider(NULL),
	fBottomDwellSlider(NULL),
	fEditorDriver(new GlobeDriver()),
	fPlaybackRunner(NULL),
	fPlayerState(PlayerState::Idle),
	fStateDeadline(0),
	fPauseStarted(0),
	fCurrentIndex(-1),
	fPaused(false) {
	fCurrentDocument.session = 0;
	BButton* chooseButton = new BButton("Choose project folder" B_UTF8_ELLIPSIS,
		new BMessage(kChooseProjectFolder));
	fFolderPathView = new BStringView("folder path", "No project folder selected");

	fPlaylistView = new BListView("playlist");
	BScrollView* playlistScrollView = new BScrollView("playlist scroll view",
		fPlaylistView, 0, false, true);

	fPlayButton = new BButton("Play", new BMessage(kOpenSelectedFile));
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
	fTopDwellSlider = new BSlider("top dwell", "",
		new BMessage(kTimingChanged), 0, 100, B_HORIZONTAL);
	fTopDwellSlider->SetValue(kDefaultTopDwellTenths);
	fScrollSpeedSlider = new BSlider("scroll speed", "",
		new BMessage(kTimingChanged), 2, 40, B_HORIZONTAL);
	fScrollSpeedSlider->SetValue(kDefaultScrollLinesPerSecond);
	fBottomDwellSlider = new BSlider("bottom dwell", "",
		new BMessage(kTimingChanged), 0, 300, B_HORIZONTAL);
	fBottomDwellSlider->SetValue(kDefaultBottomDwellTenths);
	UpdateTimingLabels();

	BGroupView* controls = new BGroupView(B_VERTICAL, 8.0f);
	controls->AddChild(new BStringView("timing heading", "Timing"));
	controls->AddChild(fTopDwellSlider);
	controls->AddChild(fScrollSpeedSlider);
	controls->AddChild(fBottomDwellSlider);
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
}
//---------------------------------------------------------------------------------------------------------------------------------//


ShowreelWindow::~ShowreelWindow() {
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
		case kOpenSelectedFile:
			OpenSelectedFile();
			break;
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
		case kPlaybackTick:
			PlaybackTick();
			break;
		case kTimingChanged:
			UpdateTimingLabels();
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


void ShowreelWindow::AdvanceToFile(int32 direction) {
	if (fPlayerState == PlayerState::Idle)
		return;

	int32 nextIndex = FindViewableFile(fCurrentIndex + direction, direction);
	if (nextIndex < 0) {
		StopPlayback();
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
		fFolderPanel = new BFilePanel(B_OPEN_PANEL, &target, NULL,
			B_DIRECTORY_NODE, false, new BMessage(kProjectFolderChosen));
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

	for (int32 index = startIndex;
		index >= 0 && index < static_cast<int32>(fPlaylistFiles.size());
		index += direction) {
		if (!fPlaylistFiles[index].skipped)
			return index;
	}

	return -1;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::HandlePlaybackError(const char* operation,
	status_t status) {
	StopPlayback();
	BString message(operation);
	message << ": " << strerror(status);
	(new BAlert("Playback stopped", message.String(), "OK", NULL, NULL,
		B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::OpenSelectedFile() {
	int32 selectedIndex = fPlaylistView->CurrentSelection();
	if (selectedIndex < 0
		|| selectedIndex >= static_cast<int32>(fPlaylistFiles.size())
		|| fPlaylistFiles[selectedIndex].skipped) {
		selectedIndex = FindViewableFile(0, 1);
	}

	if (selectedIndex < 0
		|| selectedIndex >= static_cast<int32>(fPlaylistFiles.size())) {
		return;
	}

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
	} else {
		fPauseStarted = now;
		fPaused = true;
	}
	UpdatePlaybackControls();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::PlaybackTick() {
	if (fPlayerState == PlayerState::Idle || fPaused)
		return;

	bigtime_t now = system_time();
	if (fPlayerState == PlayerState::TopDwell) {
		if (now < fStateDeadline)
			return;

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
		float step = ScrollLinesPerSecond() * lineHeight
			* static_cast<float>(kPlaybackTickInterval) / 1000000.0f;
		float nextValue = std::min(state.value + step, state.maximum);
		status = fEditorDriver->SetScrollValue(fCurrentDocument, nextValue,
			state);
		if (status != B_OK) {
			HandlePlaybackError("Could not scroll the Globe document", status);
			return;
		}
		if (state.value >= state.maximum - 0.5f) {
			fPlayerState = PlayerState::BottomDwell;
			fStateDeadline = now + BottomDwell();
		}
		return;
	}

	if (fPlayerState == PlayerState::BottomDwell && now >= fStateDeadline)
		AdvanceToFile(1);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::PopulatePlaylist(const char* folderPath) {
	ClearPlaylist();

	SourceFileDiscovery discovery;
	status_t status = discovery.Discover(folderPath, fPlaylistFiles);
	if (status != B_OK) {
		BString folderLabel(folderPath);
		folderLabel << " (could not read folder)";
		fFolderPathView->SetText(folderLabel.String());
		return;
	}

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


void ShowreelWindow::StartFile(int32 index) {
	if (index < 0 || index >= static_cast<int32>(fPlaylistFiles.size())
		|| fPlaylistFiles[index].skipped) {
		return;
	}

	CloseCurrentDocument();
	status_t status = fEditorDriver->OpenDocument(
		fPlaylistFiles[index].path.String(), fCurrentDocument);
	if (status != B_OK) {
		HandlePlaybackError("Globe could not open the selected file", status);
		return;
	}

	fCurrentIndex = index;
	fPlaylistView->Select(index);
	fPlaylistView->ScrollToSelection();
	fPlayerState = PlayerState::TopDwell;
	fStateDeadline = system_time() + TopDwell();
	fPaused = false;

	if (fPlaybackRunner == NULL) {
		BMessage tickMessage(kPlaybackTick);
		fPlaybackRunner = new BMessageRunner(BMessenger(this), &tickMessage,
			kPlaybackTickInterval);
	}
	UpdatePlaybackControls();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::StopPlayback() {
	delete fPlaybackRunner;
	fPlaybackRunner = NULL;
	CloseCurrentDocument();
	fPlayerState = PlayerState::Idle;
	fCurrentIndex = -1;
	fPaused = false;
	UpdatePlaybackControls();
}
//---------------------------------------------------------------------------------------------------------------------------------//


bigtime_t ShowreelWindow::TopDwell() const {
	return static_cast<bigtime_t>(fTopDwellSlider->Value()) * 100000;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::UpdatePlaybackControls() {
	bool playing = fPlayerState != PlayerState::Idle;
	bool hasViewableFile = FindViewableFile(0, 1) >= 0;
	fPlayButton->SetEnabled(!playing && hasViewableFile);
	fPauseButton->SetEnabled(playing);
	fPreviousButton->SetEnabled(playing
		&& FindViewableFile(fCurrentIndex - 1, -1) >= 0);
	fNextButton->SetEnabled(playing
		&& FindViewableFile(fCurrentIndex + 1, 1) >= 0);
	fStopButton->SetEnabled(playing);
	fPauseButton->SetLabel(fPaused ? "Resume" : "Pause");
	fTopDwellSlider->SetEnabled(!playing);
	fScrollSpeedSlider->SetEnabled(!playing);
	fBottomDwellSlider->SetEnabled(!playing);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::UpdateTimingLabels() {
	BString label;
	label.SetToFormat("Top dwell: %.1f s",
		static_cast<double>(fTopDwellSlider->Value()) / 10.0);
	fTopDwellSlider->SetLabel(label.String());
	label.SetToFormat("Scroll speed: %ld lines/s",
		static_cast<long>(fScrollSpeedSlider->Value()));
	fScrollSpeedSlider->SetLabel(label.String());
	label.SetToFormat("Bottom dwell: %.1f s",
		static_cast<double>(fBottomDwellSlider->Value()) / 10.0);
	fBottomDwellSlider->SetLabel(label.String());
}
//---------------------------------------------------------------------------------------------------------------------------------//


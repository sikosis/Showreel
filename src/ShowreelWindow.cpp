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
#include <Path.h>
#include <ScrollView.h>
#include <StringItem.h>
#include <StringView.h>

#include <cstring>


namespace {
	const uint32 kChooseProjectFolder = 'chpf';
	const uint32 kProjectFolderChosen = 'pfch';
	const uint32 kOpenSelectedFile = 'opsf';
	const uint32 kShowAbout = 'sabt';
}


ShowreelWindow::ShowreelWindow()
	:
	BWindow(BRect(100, 100, 820, 560), "Showreel v0.25", B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS),
	fFolderPanel(NULL),
	fFolderPathView(NULL),
	fPlaylistView(NULL),
	fPlayButton(NULL),
	fEditorDriver(new GlobeDriver()) {
	BButton* chooseButton = new BButton("Choose project folder" B_UTF8_ELLIPSIS,
		new BMessage(kChooseProjectFolder));
	fFolderPathView = new BStringView("folder path", "No project folder selected");

	fPlaylistView = new BListView("playlist");
	BScrollView* playlistScrollView = new BScrollView("playlist scroll view",
		fPlaylistView, 0, false, true);

	fPlayButton = new BButton("Play", new BMessage(kOpenSelectedFile));
	fPlayButton->SetEnabled(false);
	BButton* previousButton = new BButton("Previous", NULL);
	previousButton->SetEnabled(false);
	BButton* nextButton = new BButton("Next", NULL);
	nextButton->SetEnabled(false);
	BButton* stopButton = new BButton("Stop", NULL);
	stopButton->SetEnabled(false);
	BButton* aboutButton = new BButton("About", new BMessage(kShowAbout));

	BGroupView* controls = new BGroupView(B_VERTICAL, 8.0f);
	controls->AddChild(fPlayButton);
	controls->AddChild(previousButton);
	controls->AddChild(nextButton);
	controls->AddChild(stopButton);
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
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
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


void ShowreelWindow::ClearPlaylist() {
	while (fPlaylistView->CountItems() > 0)
		delete fPlaylistView->RemoveItem(static_cast<int32>(0));

	fPlaylistFiles.clear();
	fPlayButton->SetEnabled(false);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelWindow::OpenSelectedFile() {
	int32 selectedIndex = fPlaylistView->CurrentSelection();
	if (selectedIndex < 0
		|| selectedIndex >= static_cast<int32>(fPlaylistFiles.size())
		|| fPlaylistFiles[selectedIndex].skipped) {
		selectedIndex = -1;
		for (int32 index = 0;
			index < static_cast<int32>(fPlaylistFiles.size()); ++index) {
			if (!fPlaylistFiles[index].skipped) {
				selectedIndex = index;
				fPlaylistView->Select(selectedIndex);
				break;
			}
		}
	}

	if (selectedIndex < 0
		|| selectedIndex >= static_cast<int32>(fPlaylistFiles.size())) {
		return;
	}

	status_t status = fEditorDriver->OpenDocument(
		fPlaylistFiles[selectedIndex].path.String());
	if (status != B_OK) {
		BString message("Globe could not open the selected file: ");
		message << strerror(status);
		(new BAlert("Open failed", message.String(), "OK", NULL, NULL,
			B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
	}
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


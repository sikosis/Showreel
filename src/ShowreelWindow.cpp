#include "ShowreelWindow.h"

#include <Application.h>
#include <Button.h>
#include <FilePanel.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <ListView.h>
#include <Message.h>
#include <Path.h>
#include <ScrollView.h>
#include <StringView.h>


namespace {
	const uint32 kChooseProjectFolder = 'chpf';
	const uint32 kProjectFolderChosen = 'pfch';
}


ShowreelWindow::ShowreelWindow()
	:
	BWindow(BRect(100, 100, 820, 560), "Showreel v0.11", B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS),
	fFolderPanel(NULL),
	fFolderPathView(NULL),
	fPlayButton(NULL) {
	BButton* chooseButton = new BButton("Choose project folder" B_UTF8_ELLIPSIS,
		new BMessage(kChooseProjectFolder));
	fFolderPathView = new BStringView("folder path", "No project folder selected");

	BListView* playlistView = new BListView("playlist");
	BScrollView* playlistScrollView = new BScrollView("playlist scroll view",
		playlistView, 0, false, true);

	fPlayButton = new BButton("Play", NULL);
	fPlayButton->SetEnabled(false);
	BButton* previousButton = new BButton("Previous", NULL);
	previousButton->SetEnabled(false);
	BButton* nextButton = new BButton("Next", NULL);
	nextButton->SetEnabled(false);
	BButton* stopButton = new BButton("Stop", NULL);
	stopButton->SetEnabled(false);

	BView* controls = BGroupLayoutBuilder(B_VERTICAL, B_USE_DEFAULT_SPACING)
		.Add(fPlayButton)
		.Add(previousButton)
		.Add(nextButton)
		.Add(stopButton)
		.AddGlue();

	SetLayout(new BGroupLayout(B_VERTICAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL, B_USE_WINDOW_SPACING)
		.SetInsets(B_USE_WINDOW_SPACING)
		.Add(BGroupLayoutBuilder(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(chooseButton)
			.Add(fFolderPathView, 1.0f))
		.Add(BGroupLayoutBuilder(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(playlistScrollView, 1.0f)
			.Add(controls)));
}
//---------------------------------------------------------------------------------------------------------------------------------//


ShowreelWindow::~ShowreelWindow() {
	delete fFolderPanel;
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


void ShowreelWindow::ProjectFolderChosen(BMessage* message) {
	entry_ref folderReference;
	if (message->FindRef("refs", &folderReference) != B_OK)
		return;

	BPath folderPath(&folderReference);
	if (folderPath.InitCheck() != B_OK)
		return;

	fFolderPathView->SetText(folderPath.Path());
}
//---------------------------------------------------------------------------------------------------------------------------------//


#include "ShowreelWindow.h"

#include <Application.h>
#include <Button.h>
#include <FilePanel.h>
#include <GroupLayout.h>
#include <GroupView.h>
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
	BWindow(BRect(100, 100, 820, 560), "Showreel v0.12", B_TITLED_WINDOW,
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

	BGroupView* controls = new BGroupView(B_VERTICAL, 8.0f);
	controls->AddChild(fPlayButton);
	controls->AddChild(previousButton);
	controls->AddChild(nextButton);
	controls->AddChild(stopButton);

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


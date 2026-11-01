#include "ShowreelApplication.h"

#include "ShowreelWindow.h"

#include <Alert.h>
#include <AppFileInfo.h>
#include <Bitmap.h>
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <Message.h>
#include <Path.h>
#include <Roster.h>
#include <String.h>


ShowreelApplication::ShowreelApplication(const ShowreelOptions& options)
	:
	BApplication("application/x-vnd.Sikosis-Showreel"),
	fOptions(options),
	fWindow(NULL) {
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelApplication::AboutRequested() {
	BString information("Showreel");
	BBitmap* applicationIcon = NULL;

	app_info applicationInformation;
	if (GetAppInfo(&applicationInformation) == B_OK) {
		BFile applicationFile(&applicationInformation.ref, B_READ_ONLY);
		BAppFileInfo fileInformation(&applicationFile);
		version_info versionInformation;
		if (applicationFile.InitCheck() == B_OK
			&& fileInformation.InitCheck() == B_OK
			&& fileInformation.GetVersionInfo(&versionInformation,
				B_APP_VERSION_KIND) == B_OK) {
			information = versionInformation.long_info;
		}

		applicationIcon = new BBitmap(BRect(0, 0, 31, 31), B_CMAP8);
		if (!applicationIcon->IsValid()
			|| fileInformation.GetIcon(applicationIcon, B_LARGE_ICON) != B_OK) {
			delete applicationIcon;
			applicationIcon = NULL;
		}
	}

	information << "\n\nApplication signature: " << Signature();
	BAlert* alert = new BAlert("About Showreel", information.String(), "OK",
		NULL, NULL, B_WIDTH_AS_USUAL, B_INFO_ALERT);
	if (applicationIcon != NULL)
		alert->SetIcon(applicationIcon);
	alert->Go();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelApplication::RefsReceived(BMessage* message) {
	entry_ref reference;
	for (int32 index = 0;
		message->FindRef("refs", index, &reference) == B_OK; ++index) {
		BDirectory directory(&reference);
		if (directory.InitCheck() == B_OK) {
			SetProjectFolder(reference);
			return;
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelApplication::ReadyToRun() {
	fWindow = new ShowreelWindow();
	fWindow->SetTiming(fOptions.topDwellSeconds,
		fOptions.scrollLinesPerSecond, fOptions.bottomDwellSeconds);
	fWindow->SetCloseWhenFinished(fOptions.closeShowreel);
	if (!fOptions.projectFolder.IsEmpty())
		fWindow->SetProjectFolder(fOptions.projectFolder.String());
	fWindow->Show();
	if (fOptions.play)
		fWindow->PostMessage(ShowreelWindow::kStartPlayback);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelApplication::SetProjectFolder(const entry_ref& reference) {
	BEntry entry(&reference, true);
	BPath path;
	if (entry.GetPath(&path) != B_OK)
		return;

	if (fWindow == NULL) {
		fOptions.projectFolder = path.Path();
		return;
	}

	BMessage message(ShowreelWindow::kSetProjectFolder);
	message.AddString("path", path.Path());
	fWindow->PostMessage(&message);
}
//---------------------------------------------------------------------------------------------------------------------------------//


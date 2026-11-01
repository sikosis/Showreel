#include "ShowreelApplication.h"

#include "ShowreelWindow.h"

#include <Alert.h>
#include <AppFileInfo.h>
#include <Bitmap.h>
#include <File.h>
#include <Roster.h>
#include <String.h>


ShowreelApplication::ShowreelApplication(const ShowreelOptions& options)
	:
	BApplication("application/x-vnd.Sikosis-Showreel"),
	fOptions(options) {
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


void ShowreelApplication::ReadyToRun() {
	ShowreelWindow* window = new ShowreelWindow();
	window->SetTiming(fOptions.topDwellSeconds,
		fOptions.scrollLinesPerSecond, fOptions.bottomDwellSeconds);
	window->SetCloseWhenFinished(fOptions.closeShowreel);
	if (!fOptions.projectFolder.IsEmpty())
		window->SetProjectFolder(fOptions.projectFolder.String());
	window->Show();
	if (fOptions.play)
		window->PostMessage(ShowreelWindow::kStartPlayback);
}
//---------------------------------------------------------------------------------------------------------------------------------//


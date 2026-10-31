#include "ShowreelApplication.h"

#include "ShowreelWindow.h"

#include <Alert.h>
#include <AppFileInfo.h>
#include <File.h>
#include <String.h>


ShowreelApplication::ShowreelApplication()
	:
	BApplication("application/x-vnd.Sikosis-Showreel") {
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelApplication::AboutRequested() {
	BString information("Showreel");

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
	}

	information << "\n\nApplication signature: " << Signature();
	(new BAlert("About Showreel", information.String(), "OK", NULL, NULL,
		B_WIDTH_AS_USUAL, B_INFO_ALERT))->Go();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelApplication::ReadyToRun() {
	ShowreelWindow* window = new ShowreelWindow();
	window->Show();
}
//---------------------------------------------------------------------------------------------------------------------------------//


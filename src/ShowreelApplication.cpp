#include "ShowreelApplication.h"

#include "ShowreelWindow.h"


ShowreelApplication::ShowreelApplication()
	:
	BApplication("application/x-vnd.Sikosis-Showreel") {
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ShowreelApplication::ReadyToRun() {
	ShowreelWindow* window = new ShowreelWindow();
	window->Show();
}
//---------------------------------------------------------------------------------------------------------------------------------//


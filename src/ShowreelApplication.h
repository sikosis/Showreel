#ifndef SHOWREEL_APPLICATION_H
#define SHOWREEL_APPLICATION_H

#include <Application.h>
#include <Entry.h>
#include <String.h>


class ShowreelWindow;


struct ShowreelOptions {
	BString projectFolder;
	bool play;
	float topDwellSeconds;
	int32 scrollLinesPerSecond;
	float bottomDwellSeconds;
	bool closeShowreel;
	bool monitorMode;
};


class ShowreelApplication : public BApplication {
	public:
		ShowreelApplication(const ShowreelOptions& options);

		void AboutRequested() override;
		void MessageReceived(BMessage* message) override;
		void RefsReceived(BMessage* message) override;
		void ReadyToRun() override;

	private:
		void SetProjectFolder(const entry_ref& reference);

		ShowreelOptions fOptions;
		ShowreelWindow* fWindow;
	};

#endif

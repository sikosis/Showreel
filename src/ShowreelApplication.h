#ifndef SHOWREEL_APPLICATION_H
#define SHOWREEL_APPLICATION_H

#include <Application.h>
#include <String.h>


struct ShowreelOptions {
	BString projectFolder;
	bool play;
	float topDwellSeconds;
	int32 scrollLinesPerSecond;
	float bottomDwellSeconds;
	bool closeShowreel;
};


class ShowreelApplication : public BApplication {
	public:
		ShowreelApplication(const ShowreelOptions& options);

		void AboutRequested() override;
		void ReadyToRun() override;

	private:
		ShowreelOptions fOptions;
	};

#endif

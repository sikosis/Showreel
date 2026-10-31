#ifndef SHOWREEL_APPLICATION_H
#define SHOWREEL_APPLICATION_H

#include <Application.h>


class ShowreelApplication : public BApplication {
	public:
		ShowreelApplication();

		void AboutRequested() override;
		void ReadyToRun() override;
	};

#endif

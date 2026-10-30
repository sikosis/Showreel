#ifndef SHOWREEL_WINDOW_H
#define SHOWREEL_WINDOW_H

#include <Window.h>


class BButton;
class BFilePanel;
class BStringView;


class ShowreelWindow : public BWindow {
public:
	ShowreelWindow();
	~ShowreelWindow() override;

	void MessageReceived(BMessage* message) override;
	bool QuitRequested() override;

private:
	void ChooseProjectFolder();
	void ProjectFolderChosen(BMessage* message);

	BFilePanel* fFolderPanel;
	BStringView* fFolderPathView;
	BButton* fPlayButton;
};

#endif

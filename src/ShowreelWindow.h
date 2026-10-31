#ifndef SHOWREEL_WINDOW_H
#define SHOWREEL_WINDOW_H

#include <Window.h>

#include "SourceFileDiscovery.h"

#include <String.h>

#include <vector>


class BButton;
class BFilePanel;
class BListView;
class BStringView;
class EditorDriver;


class ShowreelWindow : public BWindow {
public:
	ShowreelWindow();
	~ShowreelWindow() override;

	void MessageReceived(BMessage* message) override;
	bool QuitRequested() override;

private:
	void ChooseProjectFolder();
	void ClearPlaylist();
	void OpenSelectedFile();
	void PopulatePlaylist(const char* folderPath);
	void ProjectFolderChosen(BMessage* message);

	BFilePanel* fFolderPanel;
	BStringView* fFolderPathView;
	BListView* fPlaylistView;
	BButton* fPlayButton;
	EditorDriver* fEditorDriver;
	std::vector<DiscoveredFile> fPlaylistFiles;
};

#endif

#ifndef SHOWREEL_EDITOR_DRIVER_H
#define SHOWREEL_EDITOR_DRIVER_H

#include <Messenger.h>
#include <SupportDefs.h>


struct EditorDocument {
	int64 session;
	BMessenger target;
};


struct EditorScrollState {
	bool ready;
	float minimum;
	float maximum;
	float value;
	float lineHeight;
};


class EditorDriver {
	public:
		virtual ~EditorDriver();

		virtual status_t CloseDocument(const EditorDocument& document) = 0;
		virtual status_t GetScrollState(const EditorDocument& document,	EditorScrollState& state) = 0;
		virtual status_t OpenDocument(const char* path, float fontSize,
			EditorDocument& document) = 0;
		virtual status_t QuitEditor() = 0;
		virtual status_t SetScrollValue(const EditorDocument& document,	float value, EditorScrollState& state) = 0;
};

#endif

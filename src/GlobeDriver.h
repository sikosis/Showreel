#ifndef SHOWREEL_GLOBE_DRIVER_H
#define SHOWREEL_GLOBE_DRIVER_H

#include "EditorDriver.h"


class BMessage;


class GlobeDriver : public EditorDriver {
public:
	status_t CloseDocument(const EditorDocument& document) override;
	status_t GetScrollState(const EditorDocument& document,
		EditorScrollState& state) override;
	status_t OpenDocument(const char* path,
		EditorDocument& document) override;
	status_t SetScrollValue(const EditorDocument& document,
		float value, EditorScrollState& state) override;

private:
	status_t ReadScrollReply(const BMessage& reply,
		EditorScrollState& state) const;
};

#endif

#ifndef SHOWREEL_EDITOR_DRIVER_H
#define SHOWREEL_EDITOR_DRIVER_H

#include <SupportDefs.h>


class EditorDriver {
public:
	virtual ~EditorDriver();

	virtual status_t OpenDocument(const char* path) = 0;
};

#endif

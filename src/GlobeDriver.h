#ifndef SHOWREEL_GLOBE_DRIVER_H
#define SHOWREEL_GLOBE_DRIVER_H

#include "EditorDriver.h"


class GlobeDriver : public EditorDriver {
public:
	status_t OpenDocument(const char* path) override;
};

#endif

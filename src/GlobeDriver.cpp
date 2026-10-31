#include "GlobeDriver.h"

#include <AppDefs.h>
#include <Entry.h>
#include <Message.h>
#include <Roster.h>


namespace {
	const char* const kGlobeSignature = "application/x-vnd.hGlobe";
}


status_t GlobeDriver::OpenDocument(const char* path) {
	if (path == NULL || path[0] == '\0')
		return B_BAD_VALUE;

	entry_ref document;
	status_t status = get_ref_for_path(path, &document);
	if (status != B_OK)
		return status;

	BMessage references(B_REFS_RECEIVED);
	status = references.AddRef("refs", &document);
	if (status != B_OK)
		return status;

	status = be_roster->Launch(kGlobeSignature, &references);
	return status == B_ALREADY_RUNNING ? B_OK : status;
}
//---------------------------------------------------------------------------------------------------------------------------------//


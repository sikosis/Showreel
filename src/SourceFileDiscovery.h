#ifndef SOURCE_FILE_DISCOVERY_H
#define SOURCE_FILE_DISCOVERY_H

#include <String.h>
#include <SupportDefs.h>

#include <vector>


class SourceFileDiscovery {
public:
	status_t Discover(const char* folderPath, std::vector<BString>& filePaths) const;

private:
	status_t DiscoverDirectory(const char* folderPath,
		std::vector<BString>& filePaths) const;
	bool IsExcludedDirectory(const char* name) const;
	bool IsSourceFile(const char* path, const char* name) const;
};

#endif

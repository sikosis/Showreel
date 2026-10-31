#include "SourceFileDiscovery.h"

#include <Directory.h>
#include <Entry.h>
#include <Mime.h>
#include <Node.h>
#include <NodeInfo.h>
#include <Path.h>

#include <algorithm>
#include <cstring>


namespace {
	struct DirectoryEntry {
		BString name;
		BString path;
		bool isDirectory;
	};


	bool IsResourceDefinition(const BString& name) {
		return name.IEndsWith(".rdef");
	}
	//---------------------------------------------------------------------------------------------------------------------------------//


	bool CompareEntries(const DirectoryEntry& left, const DirectoryEntry& right) {
		if (left.isDirectory != right.isDirectory)
			return left.isDirectory;
		if (!left.isDirectory) {
			bool leftIsResource = IsResourceDefinition(left.name);
			bool rightIsResource = IsResourceDefinition(right.name);
			if (leftIsResource != rightIsResource)
				return leftIsResource;
		}

		return left.name.ICompare(right.name) < 0;
	}
	//---------------------------------------------------------------------------------------------------------------------------------//
}


status_t SourceFileDiscovery::Discover(const char* folderPath,
	std::vector<BString>& filePaths) const {
	filePaths.clear();
	if (folderPath == NULL || folderPath[0] == '\0')
		return B_BAD_VALUE;

	return DiscoverDirectory(folderPath, filePaths);
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t SourceFileDiscovery::DiscoverDirectory(const char* folderPath,
	std::vector<BString>& filePaths) const {
	BDirectory directory(folderPath);
	status_t status = directory.InitCheck();
	if (status != B_OK)
		return status;

	std::vector<DirectoryEntry> entries;
	BEntry entry;
	while (directory.GetNextEntry(&entry, false) == B_OK) {
		char name[B_FILE_NAME_LENGTH];
		if (entry.GetName(name) != B_OK || name[0] == '.')
			continue;

		BPath path;
		if (entry.GetPath(&path) != B_OK)
			continue;

		DirectoryEntry candidate;
		candidate.name = name;
		candidate.path = path.Path();
		candidate.isDirectory = entry.IsDirectory();
		entries.push_back(candidate);
	}

	std::sort(entries.begin(), entries.end(), CompareEntries);
	for (const DirectoryEntry& candidate : entries) {
		if (candidate.isDirectory) {
			if (!IsExcludedDirectory(candidate.name.String()))
				DiscoverDirectory(candidate.path.String(), filePaths);
		} else if (IsSourceFile(candidate.path.String(), candidate.name.String())) {
			filePaths.push_back(candidate.path);
		}
	}

	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool SourceFileDiscovery::IsExcludedDirectory(const char* name) const {
	BString directoryName(name);
	directoryName.ToLower();
	return directoryName == ".git"
		|| directoryName == "bin"
		|| directoryName == "build"
		|| directoryName == "dist"
		|| directoryName == "obj"
		|| directoryName == "out"
		|| directoryName == "package-build"
		|| directoryName.StartsWith("objects.");
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool SourceFileDiscovery::IsSourceFile(const char* path, const char* name) const {
	BNode node(path);
	if (node.InitCheck() == B_OK) {
		BNodeInfo nodeInfo(&node);
		char mimeType[B_MIME_TYPE_LENGTH];
		if (nodeInfo.GetType(mimeType) == B_OK
			&& std::strncmp(mimeType, "text/", 5) == 0) {
			return true;
		}
	}

	BString fileName(name);
	fileName.ToLower();
	if (fileName == "jamfile" || fileName == "makefile")
		return true;

	const char* extension = std::strrchr(fileName.String(), '.');
	if (extension == NULL)
		return false;

	static const char* const extensions[] = {
		".c", ".cc", ".cpp", ".css", ".cxx", ".h", ".hh", ".hpp",
		".html", ".java", ".js", ".json", ".md", ".py", ".rdef",
		".recipe", ".sh", ".ts", ".txt", ".xml", ".yaml", ".yml"
	};

	for (const char* sourceExtension : extensions) {
		if (std::strcmp(extension, sourceExtension) == 0)
			return true;
	}

	return false;
}
//---------------------------------------------------------------------------------------------------------------------------------//


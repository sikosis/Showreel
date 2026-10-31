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


	int32 FilePriority(const BString& name) {
		if (name.IEndsWith(".rdef"))
			return 0;
		if (name.IEndsWith(".h") || name.IEndsWith(".hh")
			|| name.IEndsWith(".hpp")) {
			return 1;
		}
		if (name.IEndsWith(".c") || name.IEndsWith(".cc")
			|| name.IEndsWith(".cpp") || name.IEndsWith(".cxx")) {
			return 2;
		}
		return 3;
	}
	//---------------------------------------------------------------------------------------------------------------------------------//


	bool CompareEntries(const DirectoryEntry& left, const DirectoryEntry& right) {
		if (left.isDirectory != right.isDirectory)
			return left.isDirectory;
		if (!left.isDirectory) {
			int32 leftPriority = FilePriority(left.name);
			int32 rightPriority = FilePriority(right.name);
			if (leftPriority != rightPriority)
				return leftPriority < rightPriority;
		}

		return left.name.ICompare(right.name) < 0;
	}
	//---------------------------------------------------------------------------------------------------------------------------------//
}


status_t SourceFileDiscovery::Discover(const char* folderPath,
	std::vector<DiscoveredFile>& files) const {
	files.clear();
	if (folderPath == NULL || folderPath[0] == '\0')
		return B_BAD_VALUE;

	return DiscoverDirectory(folderPath, files);
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t SourceFileDiscovery::DiscoverDirectory(const char* folderPath,
	std::vector<DiscoveredFile>& files) const {
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
				DiscoverDirectory(candidate.path.String(), files);
		} else if (IsSourceFile(candidate.path.String(), candidate.name.String())) {
			DiscoveredFile file;
			file.path = candidate.path;
			file.skipped = ShouldSkipFile(candidate.name.String());
			files.push_back(file);
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
	BString fileName(name);
	fileName.ToLower();
	BNode node(path);
	if (node.InitCheck() == B_OK) {
		BNodeInfo nodeInfo(&node);
		char mimeType[B_MIME_TYPE_LENGTH];
		if (nodeInfo.GetType(mimeType) == B_OK
			&& std::strncmp(mimeType, "text/", 5) == 0) {
			return true;
		}
	}

	if (fileName == "jamfile" || fileName == "makefile")
		return true;

	const char* extension = std::strrchr(fileName.String(), '.');
	if (extension == NULL)
		return false;

	static const char* const extensions[] = {
		".bash", ".c", ".cc", ".command", ".cpp", ".css", ".cxx",
		".fish", ".h", ".hh", ".hpp", ".html", ".java", ".js",
		".json", ".markdown", ".md", ".pl", ".py", ".rb", ".rdef",
		".recipe", ".sh", ".ts", ".txt", ".xml", ".yaml", ".yml",
		".zsh"
	};

	for (const char* sourceExtension : extensions) {
		if (std::strcmp(extension, sourceExtension) == 0)
			return true;
	}

	return false;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool SourceFileDiscovery::ShouldSkipFile(const char* name) const {
	BString fileName(name);
	fileName.ToLower();
	static const char* const skippedExtensions[] = {
		".bash", ".command", ".fish", ".md", ".markdown", ".pl", ".py",
		".rb", ".sh", ".zsh"
	};
	for (const char* skippedExtension : skippedExtensions) {
		if (fileName.EndsWith(skippedExtension))
			return true;
	}

	return false;
}
//---------------------------------------------------------------------------------------------------------------------------------//


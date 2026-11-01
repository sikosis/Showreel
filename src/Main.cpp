#include "ShowreelApplication.h"

#include <Directory.h>
#include <Entry.h>
#include <Path.h>

#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>


namespace {

void PrintUsage(const char* programName) {
	printf("Usage: %s [options] PROJECT_FOLDER\n"
		"\n"
		"Options:\n"
		"  --project PATH          Project folder to display\n"
		"  --play                  Start playback automatically\n"
		"  --top-dwell SECONDS     Pause before scrolling (0-10)\n"
		"  --scroll-speed LINES    Scroll speed in lines/second (2-40)\n"
		"  --bottom-dwell SECONDS  Pause after scrolling (0-30)\n"
		"  --close                 Close Showreel when playback finishes\n"
		"  -h, --help              Show this help\n",
		programName);
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool ParseFloat(const char* text, float minimum, float maximum,
	float& value) {
	errno = 0;
	char* end = NULL;
	double parsed = strtod(text, &end);
	if (errno != 0 || end == text || *end != '\0' || !std::isfinite(parsed)
		|| parsed < minimum || parsed > maximum) {
		return false;
	}

	value = static_cast<float>(parsed);
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool ParseInteger(const char* text, int32 minimum, int32 maximum,
	int32& value) {
	errno = 0;
	char* end = NULL;
	long parsed = strtol(text, &end, 10);
	if (errno != 0 || end == text || *end != '\0'
		|| parsed < minimum || parsed > maximum) {
		return false;
	}

	value = static_cast<int32>(parsed);
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool SetProjectFolder(const char* path, ShowreelOptions& options) {
	BDirectory directory(path);
	if (directory.InitCheck() != B_OK)
		return false;

	BEntry entry(path, true);
	BPath resolvedPath;
	if (entry.GetPath(&resolvedPath) != B_OK)
		return false;

	options.projectFolder = resolvedPath.Path();
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


} // namespace


int main(int argc, char** argv) {
	ShowreelOptions options;
	options.play = false;
	options.topDwellSeconds = 5.0f;
	options.scrollLinesPerSecond = 8;
	options.bottomDwellSeconds = 5.0f;
	options.closeShowreel = false;

	for (int32 index = 1; index < argc; ++index) {
		const char* argument = argv[index];
		if (strcmp(argument, "-h") == 0 || strcmp(argument, "--help") == 0) {
			PrintUsage(argv[0]);
			return 0;
		}
		if (strcmp(argument, "--play") == 0) {
			options.play = true;
			continue;
		}
		if (strcmp(argument, "--close") == 0) {
			options.closeShowreel = true;
			continue;
		}

		if (strcmp(argument, "--project") == 0
			|| strcmp(argument, "--top-dwell") == 0
			|| strcmp(argument, "--scroll-speed") == 0
			|| strcmp(argument, "--bottom-dwell") == 0) {
			if (++index >= argc) {
				fprintf(stderr, "%s requires a value\n", argument);
				return 2;
			}
			const char* value = argv[index];
			if (strcmp(argument, "--project") == 0) {
				if (!options.projectFolder.IsEmpty()) {
					fprintf(stderr,
						"Only one project folder may be specified\n");
					return 2;
				}
				if (!SetProjectFolder(value, options)) {
					fprintf(stderr, "Not a project folder: %s\n", value);
					return 2;
				}
			} else if (strcmp(argument, "--top-dwell") == 0) {
				if (!ParseFloat(value, 0.0f, 10.0f,
					options.topDwellSeconds)) {
					fprintf(stderr, "Invalid top dwell: %s\n", value);
					return 2;
				}
			} else if (strcmp(argument, "--scroll-speed") == 0) {
				if (!ParseInteger(value, 2, 40,
					options.scrollLinesPerSecond)) {
					fprintf(stderr, "Invalid scroll speed: %s\n", value);
					return 2;
				}
			} else if (!ParseFloat(value, 0.0f, 30.0f,
				options.bottomDwellSeconds)) {
				fprintf(stderr, "Invalid bottom dwell: %s\n", value);
				return 2;
			}
			continue;
		}

		if (argument[0] == '-') {
			fprintf(stderr, "Unknown option: %s\n", argument);
			PrintUsage(argv[0]);
			return 2;
		}
		if (!options.projectFolder.IsEmpty()) {
			fprintf(stderr, "Only one project folder may be specified\n");
			return 2;
		}
		if (!SetProjectFolder(argument, options)) {
			fprintf(stderr, "Not a project folder: %s\n", argument);
			return 2;
		}
	}

	if (options.play && options.projectFolder.IsEmpty()) {
		fprintf(stderr, "--play requires a project folder\n");
		return 2;
	}

	ShowreelApplication application(options);
	application.Run();
	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//


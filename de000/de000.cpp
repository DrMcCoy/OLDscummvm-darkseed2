#include <cstdio>
#include <cstring>

#include <list>
#include <fstream>

#include "util.h"

struct FileInfo {
	char name[13];
	uint32 offset;
	uint32 size;

	FileInfo(const char *n = "", uint32 o = 0, uint32 s = 0) : offset(o), size(s) {
		strncpy(name, n, 12);
		name[12] = '\0';
	}
};

void printHelp(const char *binName);

int main(int argc, char **argv) {
	if (argc < 2) {
		printHelp(argv[0]);
		return -1;
	}

	std::list<FileInfo> files;
	std::ifstream file000;

	file000.open(argv[1]);

	if (!file000.is_open()) {
		printf("Error opening file \"%s\"\n", argv[1]);
		return -1;
	}

	if (readUint8(file000) == 0xFF) {
		printf("Compressed format not yet supported\n");
		return -1;
	}

	file000.seekg(0, std::ios_base::beg);

	uint16 fileCount = readUint16LE(file000);

	printf("Number of file: %d\n", fileCount);

	while (fileCount-- > 0) {
		FileInfo file;
	}

	return 0;
}

void printHelp(const char *binName) {
	printf("Usage: %s <file>\n\n", binName);
	printf("Files will be extracted into the current directory\n");
}

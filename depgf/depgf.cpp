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
void readFileList(std::ifstream &stream, std::list<FileInfo> &files, uint32 count);
void extractFiles(std::ifstream &stream, std::list<FileInfo> &files);

int main(int argc, char **argv) {
	if (argc < 2) {
		printHelp(argv[0]);
		return -1;
	}

	std::ifstream pgfFile;

	pgfFile.open(argv[1]);

	if (!pgfFile.is_open()) {
		printf("Error opening file \"%s\"\n", argv[1]);
		return -1;
	}

	uint32 fileCount = readUint32BE(pgfFile);

	printf("Number of file: %d\n", fileCount);

	std::list<FileInfo> files;

	readFileList(pgfFile, files, fileCount);

	extractFiles(pgfFile, files);

	files.clear();
	pgfFile.close();

	return 0;
}

void printHelp(const char *binName) {
	printf("Usage: %s <file>\n\n", binName);
	printf("Files will be extracted into the current directory\n");
}

void readFileList(std::ifstream &stream, std::list<FileInfo> &files, uint32 count) {
	int startOffset = count * 20 + 4;

	while (count-- > 0) {
		FileInfo file;

		readFixedString(stream, file.name, 12);

		file.size   = readUint32BE(stream);
		file.offset = readUint32BE(stream) + startOffset;

		files.push_back(file);
	}
}

void extractFiles(std::ifstream &stream, std::list<FileInfo> &files) {
	for (std::list<FileInfo>::iterator it = files.begin(); it != files.end(); ++it) {
		printf("%12s: %10d, %10d\n", it->name, it->offset, it->size);

		std::ofstream outFile;

		outFile.open(it->name);

		if (!outFile.is_open()) {
			printf("	Can't open file \"%s\" for writing\n", it->name);
			continue;
		}

		stream.seekg(it->offset, std::ios_base::beg);

		if (stream.tellg() != it->offset) {
			printf("	Can't seek to offset %d", it->offset);
			continue;
		}

		uint32 size = it->size;
		char buffer[4096];
		while (size > 0) {
			uint32 toRead = MIN<uint32>(size, 4096);

			stream.read(buffer, toRead);
			outFile.write(buffer, toRead);

			size -= toRead;
		}

		outFile.flush();
		outFile.close();
	}
}

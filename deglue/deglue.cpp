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
void readFileList(std::ifstream &stream, std::list<FileInfo> &files, uint16 count);
void extractFiles(std::ifstream &stream, std::list<FileInfo> &files);

int main(int argc, char **argv) {
	if (argc < 2) {
		printHelp(argv[0]);
		return -1;
	}

	std::ifstream glueFile;

	glueFile.open(argv[1]);

	if (!glueFile.is_open()) {
		printf("Error opening file \"%s\"\n", argv[1]);
		return -1;
	}

	if (readUint8(glueFile) == 0xFF) {
		printf("Compressed format not yet supported\n");
		return -1;
	}

	glueFile.seekg(0, std::ios_base::beg);

	uint16 fileCount = readUint16LE(glueFile);

	printf("Number of file: %d\n", fileCount);

	std::list<FileInfo> files;

	readFileList(glueFile, files, fileCount);

	extractFiles(glueFile, files);


	files.clear();
	glueFile.close();

	return 0;
}

void printHelp(const char *binName) {
	printf("Usage: %s <file>\n\n", binName);
	printf("Files will be extracted into the current directory\n");
}

void readFileList(std::ifstream &stream, std::list<FileInfo> &files, uint16 count) {
	while (count-- > 0) {
		FileInfo file;

		readFixedString(stream, file.name, 12);

		file.size   = readUint32LE(stream);
		file.offset = readUint32LE(stream);

		files.push_back(file);
	}
}

void extractFiles(std::ifstream &stream, std::list<FileInfo> &files) {
	for (std::list<FileInfo>::iterator it = files.begin(); it != files.end(); ++it) {
		printf("%12s: %d, %d\n", it->name, it->offset, it->size);

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

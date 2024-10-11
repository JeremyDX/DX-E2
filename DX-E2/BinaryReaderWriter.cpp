#include "BinaryReaderWriter.h"
#include <Windows.h>
#include <fstream>

char* buffer = NULL;
uint16_t writer_position = 0;
uint16_t reader_position = 0;
uint16_t capacity = 0;

void BinaryReaderWriter::ReadBinaryData(const char* name, char* &buffer_reference, int& length)
{
	std::ifstream file(name, std::ios::binary | std::ios::in | std::ios::ate);
	int file_size = static_cast<int>(file.tellg());

	if (buffer_reference == NULL)
	{
		buffer_reference = new char[file_size];
	} 
	else if (file_size > length)
	{
		char* temp = new char[file_size];
		memcpy(buffer_reference, temp, sizeof(char) * length);
		delete[] buffer_reference;
		buffer_reference = temp;
	}

	length = file_size;
	file.read(buffer_reference, file_size);
	file.close();
}

bool BinaryReaderWriter::ReadTextFileIntoBuffer(const char* filename, char*& buffer, int& length)
{
	std::ifstream file(filename, std::ios::binary); // Open the file in binary mode
	if (!file.is_open()) return false;               // Return false if the file cannot be opened

	// Move to the end of the file to determine its size
	file.seekg(0, std::ios::end);
	length = file.tellg();                           // Get the file size
	file.seekg(0, std::ios::beg);                   // Move back to the beginning of the file

	buffer = (char*)malloc(length + 1);              // Allocate memory for the buffer
	if (!buffer) {
		file.close();
		return false;                                // Return false if memory allocation fails
	}

	file.read(buffer, length);                       // Read the file into the buffer
	buffer[length] = '\0';                           // Null-terminate the buffer

	file.close();                                    // Close the file
	return true;                                     // Return true on success
}

void WriteBinaryDataToBuffer(const WCHAR* name)
{
	std::ifstream file(name, std::ios::binary | std::ios::in | std::ios::ate);
	int length = static_cast<int>(file.tellg());

	if (buffer != NULL)
	{
		char* temp = new char[capacity + length];
		memcpy(temp, buffer, (capacity + length) * sizeof(char));
		capacity = writer_position += length;
	}
	else {
		buffer = new char[length];
		writer_position = length;
		reader_position = 0;
		capacity = length;
	}
	file.read(buffer, length);
	file.close();
}

void BinaryReaderWriter::AddShadersToCache()
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(L"*", &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
			{
				WCHAR c = NULL;
				int len = -1;
				int compare_index = 0;
				const WCHAR* compare = L".cso";

				while ((c = fd.cFileName[++len]) != NULL)
				{
					if (compare[compare_index] == c)
						++compare_index;
					else
						compare_index = 0;
				}
				if (compare_index == 4)
				{
					WriteBinaryDataToBuffer(fd.cFileName);
				}
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
}

void BinaryReaderWriter::SkipBytes(int length)
{
	writer_position += length;
}

int BinaryReaderWriter::SkipAndTellBytes(int length)
{
	writer_position += length;
	return writer_position - length;
}

void BinaryReaderWriter::WriteIndexSizeByte(int seek, int8_t value) {
	int old_index = writer_position;
	writer_position = seek;
	buffer[writer_position]		= (int8_t) value;
	writer_position = old_index;
}

void BinaryReaderWriter::WriteIndexSizeShort(int seek, int value) {
	int old_index = writer_position;
	writer_position = seek;
	buffer[writer_position]		= (int8_t)(value >> 8);
	buffer[writer_position + 1] = (int8_t) value;
	writer_position = old_index;
}


void BinaryReaderWriter::WriteIndexSizeInt(int seek, int value) {
	int old_index = writer_position;
	writer_position = seek;
	buffer[writer_position]		= (int8_t)(value >> 24);
	buffer[writer_position + 1] = (int8_t)(value >> 16);
	buffer[writer_position + 2] = (int8_t)(value >> 8);
	buffer[writer_position + 3] = (int8_t) value;
	writer_position = old_index;
}


void BinaryReaderWriter::WriteByte(int8_t value)
{
	buffer[writer_position++] = value;
}

void BinaryReaderWriter::WriteByte(int8_t value, int position)
{
	buffer[position] = value;
}

void BinaryReaderWriter::WriteShort(int16_t value)
{
	buffer[writer_position++] = (int8_t)(value >> 8);
	buffer[writer_position++] = (int8_t)value;
}

void BinaryReaderWriter::WriteShort(int16_t value, int position)
{
	buffer[position] = (int8_t)(value >> 8);
	buffer[++position] = (int8_t)value;
}

void BinaryReaderWriter::WriteInt(int32_t value)
{
	buffer[writer_position++] = (int8_t)(value >> 24);
	buffer[writer_position++] = (int8_t)(value >> 16);
	buffer[writer_position++] = (int8_t)(value >> 8);
	buffer[writer_position++] = (int8_t)value;
}

void BinaryReaderWriter::WriteInt(int32_t value, int position)
{
	buffer[position] = (int8_t)(value >> 24);
	buffer[++position] = (int8_t)(value >> 16);
	buffer[++position] = (int8_t)(value >> 8);
	buffer[++position] = (int8_t)value;
}

void BinaryReaderWriter::WriteLong(int64_t value)
{
	buffer[writer_position++] = (int8_t)(value >> 56L);
	buffer[writer_position++] = (int8_t)(value >> 48L);
	buffer[writer_position++] = (int8_t)(value >> 40L);
	buffer[writer_position++] = (int8_t)(value >> 32L);
	buffer[writer_position++] = (int8_t)(value >> 24L);
	buffer[writer_position++] = (int8_t)(value >> 16L);
	buffer[writer_position++] = (int8_t)(value >> 8L);
	buffer[writer_position++] = (int8_t)value;
}

void BinaryReaderWriter::WriteLEShort(int16_t value)
{
	buffer[writer_position++] = (int8_t)value;
	buffer[writer_position++] = (int8_t)(value >> 8);
}

void BinaryReaderWriter::WriteLEInt(int32_t value)
{
	buffer[writer_position++] = (int8_t)value;
	buffer[writer_position++] = (int8_t)(value >> 8);
	buffer[writer_position++] = (int8_t)(value >> 16);
	buffer[writer_position++] = (int8_t)(value >> 24);
}

void BinaryReaderWriter::WriteLELong(int64_t value)
{
	buffer[writer_position++] = (int8_t)value;
	buffer[writer_position++] = (int8_t)(value >> 8L);
	buffer[writer_position++] = (int8_t)(value >> 16L);
	buffer[writer_position++] = (int8_t)(value >> 24L);
	buffer[writer_position++] = (int8_t)(value >> 32L);
	buffer[writer_position++] = (int8_t)(value >> 40L);
	buffer[writer_position++] = (int8_t)(value >> 48L);
	buffer[writer_position++] = (int8_t)(value >> 56L);
}

int8_t BinaryReaderWriter::ReadByte()
{
	return  buffer[reader_position++];
}

int16_t BinaryReaderWriter::ReadShort()
{
	return (buffer[reader_position++] <<  8) |  buffer[reader_position++];
}

int32_t BinaryReaderWriter::ReadInt()
{
	return (buffer[reader_position++] << 24) | (buffer[reader_position++] << 16) |
		   (buffer[reader_position++] <<  8) |  buffer[reader_position++];
}

int64_t BinaryReaderWriter::ReadLong()
{
	return (static_cast<int64_t>(buffer[reader_position++]) << 56LL) |
		(static_cast<int64_t>(buffer[reader_position++]) << 48LL) |
		(static_cast<int64_t>(buffer[reader_position++]) << 40LL) |
		(static_cast<int64_t>(buffer[reader_position++]) << 32LL) |
		(static_cast<int64_t>(buffer[reader_position++]) << 24LL) |
		(static_cast<int64_t>(buffer[reader_position++]) << 16LL) |
		(static_cast<int64_t>(buffer[reader_position++]) << 8LL) |
		(static_cast<int64_t>(buffer[reader_position++]) << 0LL);
}

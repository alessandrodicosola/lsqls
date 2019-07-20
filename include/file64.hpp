#ifndef __FILE64__
#define __FILE64__

#include <string>
#include <locale>
#include <functional>
#include <cstdio>


static const char* FILE_MODE_READ = "r";
static const char* FILE_MODE_WRITE = "w";
static const char* FILE_MODE_APPEND = "a";
static const char* FILE_MODE_READ_AND_WRITING = "r+";
//The file is created if not exist
static const char* FILE_MODE_WRITE_AND_READ = "w+";
static const char* FILE_MODE_APPEND_AND_READ = "a+";

class file64
{
private:
	FILE* ptr;
	const int BUFFER_SIZE;

	const char* _filename;

	uint64_t _size = 0;
	uint64_t _line_number = 0;
	bool _is_open = false;

public:
	file64(const char* filename, const char* mode, int buffer_size);
	~file64();

	void close();

	bool getline(std::string&);
	void writeline(const std::string&);
	

	void flush();
	void start();

	const uint64_t size() const { return _size; }
	const bool is_open() const { return _is_open; }
	const uint64_t position() const;
	uint64_t line() const { return _line_number; }
	const char* filename() const { return _filename; }

private:
	std::function<bool(int)> not_isspace_func = [](char c) -> bool { return !std::isspace(c,std::locale("")); };
};

#endif

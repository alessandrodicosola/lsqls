#include "file64.hpp"
#include <cinttypes>
#include <algorithm>
#include "utility.hpp"


#ifdef __linux__
#defind _seek_(FILE, OFFSET, ORIGIN) fseeko(FILE, OFFSET, ORIGIN)
#define _tell_(FILE) ftello(FILE)
#elif __WIN32 || _WIN64
#define _seek_(FILE,OFFSET,ORIGIN) _fseeki64(FILE,OFFSET,ORIGIN)
#define _tell_(FILE) _ftelli64(FILE)
#endif

file64::file64(const char* filename, const char* mode, int buffer_size) : BUFFER_SIZE(buffer_size), _filename{ filename }
{

	ptr = fopen(filename, mode);

	if (ptr == nullptr)
	{
		utility::__throw("error opening file: %s", filename);
	}

	_is_open = true;

	char result = setvbuf(ptr, nullptr, _IOFBF, BUFFER_SIZE);
	if (result != 0)
	{
		fclose(ptr);
		utility::__throw("error allocating buffer");
	}

	result = _seek_(ptr, 0, SEEK_END);

	if (result != 0)
	{
		fclose(ptr);
		utility::__throw("error obtaining file size");
	}

	_size = _tell_(ptr);

	rewind(ptr);
}

file64::~file64()
{
	close();
}

void file64::close()
{
	if (ptr)
	{
		fclose(ptr);
		if (ferror(ptr)>0) perror("error closing file");
	}

	_line_number = 0;
	_size = 0;
	_is_open = false;
	_filename = "";

}

bool file64::getline(std::string& line)
{
	char* buffer = new char[BUFFER_SIZE];
	char* result = fgets(&buffer[0], BUFFER_SIZE, ptr);
	if (!result)
	{
		const bool eof = feof(ptr);
		const bool error = ferror(ptr);
		if (error)
		{
			fclose(ptr);
			utility::__throw("error reading at line" PRIu64 "%d in %s", _line_number, _filename);
		}
		else if (eof)
		{
			return false;
		}
	}

	_line_number++;

	line = std::string(buffer);

	delete[] buffer;

	/* trim string and delete last \n char */
	auto last_lf = line.find_last_of('\n');
	if (last_lf != std::string::npos)
		line.erase(last_lf);
	auto left = std::find_if(line.cbegin(), line.cend(), not_isspace_func);
	auto right = std::find_if(line.crbegin(), line.crend(), not_isspace_func).base();
	if (left > line.cbegin())
		line.erase(line.cbegin(), left);
	if (right < line.cend())
		line.erase(right, line.cend());
	return true;
}

void file64::writeline(const std::string& line)
{
	const int result1 = fputs(line.c_str(), ptr);
	const int result2 = fputc('\n', ptr);
	
	if (ferror(ptr))
	{
		flush();
		fclose(ptr);
		utility::__throw("error writing data");
	}
}



void file64::flush()
{
	const int result = fflush(ptr);
	if (result != 0)
	{
		perror("error flushing buffer");
		clearerr(ptr);
	}
}

void file64::start()
{
	rewind(ptr);
}

const uint64_t file64::position() const
{
	return _tell_(ptr);
}

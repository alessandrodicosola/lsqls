#ifndef __FILE64__
#define __FILE64__

#include <string>
#include <functional>

static const char *FILE_MODE_READ = "r";
static const char *FILE_MODE_WRITE = "w";
static const char *FILE_MODE_APPEND = "a";
static const char *FILE_MODE_READ_AND_WRITING = "r+";
//The file is created if not exist
static const char *FILE_MODE_WRITE_AND_READ = "w+";
static const char *FILE_MODE_APPEND_AND_READ = "a+";

class file64
{
private:
    FILE *ptr;
    const char *_filename;
    uint64_t _file_size = 0;

    bool _is_open = false;

    const unsigned int _BUFFER_SIZE;
    uint64_t _line_number = 0;

public:
    file64(const char *filename, const char *mode, const unsigned int BUFFER_SIZE);
    ~file64();
    
    void close();

    const bool getline(std::string &line);
    void writeline(const std::string &line);

    void flush();
    void start();

    const uint64_t size() const { return _file_size; }
    const bool is_open() const { return _is_open; }
    const uint64_t position() const;
    uint64_t line() const { return _line_number; }
    const char* filename() const { return _filename; }

private:
    std::function<bool(int)> not_isspace_func = [](int c) -> bool { return !std::isspace(c); };
};

#endif
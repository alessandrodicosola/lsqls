#include "file64.hpp"
#include <stdexcept>
#include <algorithm>

file64::file64(const char *filename, const char *mode, const unsigned int BUFFER_SIZE) : _BUFFER_SIZE{BUFFER_SIZE}
{
    ptr = fopen(filename, mode);

    if (ptr == nullptr)
    {
        perror("Error occured opening file");
        throw std::runtime_error("Error occured opening file");
    }

    _is_open = true;

    char result = setvbuf(ptr, NULL, _IOFBF, _BUFFER_SIZE);
    if (result != 0)
    {
        perror("Error occured allocating the buffer");
        pclose(ptr);
        throw std::runtime_error("Error occured allocating buffer");
    }

    result = fseeko(ptr, 0, SEEK_END);

    if (result != 0)
    {
        perror("Error occured reading file to the end");
        fclose(ptr);
        throw std::runtime_error("It's impossibile to determine file size");
    }

    _file_size = ftello(ptr);

    rewind(ptr);
}

file64::~file64()
{
    close();
}

void file64::close()
{

    _line_number = 0;
    if (ptr != nullptr)
    {
        fflush(ptr);
        fclose(ptr);
        if (ferror(ptr))
            perror("Error closing file");
        _is_open = false;
    }

    _is_open = false;
}
const bool file64::get_line(std::string &line)
{
    char *buffer = new char[_BUFFER_SIZE];
    char *result = fgets(buffer, _BUFFER_SIZE, ptr);
    if (result == nullptr)
    {
        bool eof = feof(ptr);
        bool error = ferror(ptr);
        if (error)
        {
            perror("Error reading string from file");
            fclose(ptr);
            throw std::runtime_error("Error rreading line " + std::to_string(_line_number));
        }
        else if (eof)
        {
            return false;
        }
    }

    _line_number++;

    line = std::string(buffer);

    /* trim string and delete last \n char */
    auto last_lf = line.find_last_of('\n');
    if (last_lf != std::string::npos) line.erase(last_lf);
    auto left = std::find_if(line.cbegin(), line.cend(), not_isspace_func);
    auto right = std::find_if(line.crbegin(), line.crend(), not_isspace_func).base(); // .base() change iterator direction
    line.erase(line.cbegin(), left);
    line.erase(right, line.cend());
    return true;
}

void file64::write_line(const std::string &line)
{
    std::string buffer = line;
    buffer.append("\n");

    size_t result = fputs(buffer.c_str(), ptr);
    if (ferror(ptr))
    {
        perror("Error writing data into the stream");
        clearerr(ptr);
        flush();
        pclose(ptr);
        throw std::runtime_error("Error writing data into the stream");
    }
}

void file64::flush()
{
    fflush(ptr);
}

void file64::start()
{
    rewind(ptr);
}

const uint64_t file64::current_position() const
{
    return ftello(ptr);
}

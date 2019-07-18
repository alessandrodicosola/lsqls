#include "file64.hpp"

file64::file64(const char *filename, const char *mode, const unsigned int BUFFER_SIZE) : _BUFFER_SIZE{BUFFER_SIZE}, _filename{filename}
{
    ptr = fopen(filename, mode);

    if (ptr == nullptr)
    {
        perror("Error occured opening file");
        std::string msg = "Error occured opening file: ";
        msg.append(_filename);
        throw std::runtime_error(msg);
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
        throw std::runtime_error("It is impossibile to determine file size");
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
        fclose(ptr); /* delete pointer */
        if (ferror(ptr)) perror("error closing file");
    }

    _is_open = false;

}
const bool file64::getline(std::string &line)
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
            throw std::runtime_error("Error reading line " + std::to_string(++_line_number));
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

void file64::writeline(const std::string &line)
{

    std::string buffer = line;
    buffer.append("\n");

    char result = fputs(buffer.c_str(), ptr);

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
    char result = fflush(ptr);
    if (result != 0)
    {
        std::string msg = "error occured flushing data from buffer to file: ";
        msg.append(_filename);

        perror(msg.c_str());
        clearerr(ptr);
    }
}

void file64::start()
{
    rewind(ptr);
}

const uint64_t file64::position() const
{
    return ftello(ptr);
}

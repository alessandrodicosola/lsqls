#ifndef __MYSQL64__
#define __MYSQL64__

#include "sql_operation.hpp"
#include "sql_utility.hpp"

class mysql64 : public sql_operations
{
public:
    mysql64(const std::string &filename, const char *mode, const unsigned int BUFFER_SIZE = 1024);

    void close()
    {
        if (file.is_open())
            file.close();
    }

    virtual void write(const statement &statement) override;
    virtual const bool read(statement &statement) override;

    const uint64_t size() const { return file.file_size(); }
    const bool is_open() const { return file.is_open(); }
    const uint64_t current_position() { return file.current_position(); }

private:
    file64 file;
};

/* implementation inside header for avoiding ld error */
mysql64::mysql64(const std::string &filename, const char *mode,const unsigned int BUFFER_SIZE) : file{filename.c_str(), mode, BUFFER_SIZE}
{
}

void mysql64::write(const statement &statement)
{
    file.write_line(statement.line.c_str());
}
const bool mysql64::read(statement &statement)
{
    std::string line_read;

    const bool no_eof = file.get_line(line_read);
    if (!no_eof)
        return false; /* eof */

    if (line_read.empty())
    {
        statement.type = statement_type::NONE;
        statement.line_number = file.current_last_line_number();
        statement.table = "";
        statement.line = "";
        return true;
    }

    statement.line = line_read;
    statement.line_number = file.current_last_line_number();
    statement.table = sql_utility::get_table(line_read);

    if (sql_utility::is_comment(line_read))
        statement.type = statement_type::COMMENT;
    else if (sql_utility::is_executable_comment(line_read))
        statement.type = statement_type::EXECUTABLE_COMMENT;
    else if (sql_utility::is_starting_multiline_comment(line_read))
        statement.type = statement_type::START_MULTILINE_COMMENT;
    else if (sql_utility::is_ending_multiline_comment(line_read))
        statement.type = statement_type::END_MULTILINE_COMMENT;
    else
        statement.type = sql_utility::get_statement_type(line_read);

    return true;
}

#endif
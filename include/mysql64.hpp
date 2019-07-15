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
mysql64::mysql64(const std::string &filename, const char *mode, const unsigned int BUFFER_SIZE) : file{filename.c_str(), mode, BUFFER_SIZE}
{
}

void mysql64::write(const statement &statement)
{
    file.write_line(statement.line.c_str());
}
const bool mysql64::read(statement &statement)
{
    std::string line_read;
    std::string out;

    const bool no_eof = file.get_line(line_read);

    if (!no_eof)
        return false; /* eof */

    statement.line_number = file.current_last_line_number();
    statement.table = "";

    if (line_read.empty())
    {
        statement.type = statement_type::NONE;
        statement.line = "";
        return true;
    }
    else if (sql_utility::is_comment(line_read))
    {
        statement.line = line_read;
        statement.type = statement_type::COMMENT;
    }
    else if (sql_utility::is_executable_comment(line_read))
    {
        statement.type = statement_type::EXECUTABLE_COMMENT;
        statement.line = line_read;
        statement.table = sql_utility::get_table(line_read);
    }
    else if (sql_utility::is_starting_multiline_comment(line_read))
    {
        std::string comment = line_read;
        while (sql_utility::is_ending_multiline_comment(line_read) && file.get_line(line_read))
        {
            comment.append("\n");
            comment.append(line_read);
        }
        statement.line = comment;
        statement.type = statement_type::COMMENT;
    }
    else
    {
        std::string out = line_read;
        while (line_read.back() != ';' && file.get_line(line_read))
        {
            out.append("\n");
            out.append(line_read);
        }
        statement.line = out;
        statement.table = sql_utility::get_table(out);
        statement.type = sql_utility::get_statement_type(out);
    }

    return true;
}

#endif
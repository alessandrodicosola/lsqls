#ifndef __MYSQL64__
#define __MYSQL64__

#include "sql_operation.hpp"
#include "sql_utility.hpp"
#include "statement.hpp"

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

    const bool is_open() const { return file.is_open(); }
    void flush() { file.flush(); }

    const uint64_t size() const { return file.size(); }
    const std::string filename() const { return std::string(file.filename()); }
    const uint64_t position() { return file.position(); }

private:
    void assert_ending_with_column(const statement &) const;

private:
    file64 file;
};

mysql64::mysql64(const std::string &filename, const char *mode, const unsigned int BUFFER_SIZE) : file{filename.c_str(), mode, BUFFER_SIZE}
{
}

void mysql64::write(const statement &statement)
{
    file.writeline(statement.line.c_str());
}
const bool mysql64::read(statement &statement)
{
    std::string line_read;
    std::string out;

    const bool no_eof = file.getline(line_read);

    if (!no_eof)
        return false; /* eof */

    statement.line_number = file.line();
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

        assert_ending_with_column(statement);
    }
    else if (sql_utility::is_starting_multiline_comment(line_read))
    {
        std::string comment = line_read;
        while (sql_utility::is_ending_multiline_comment(line_read) && file.getline(line_read))
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
        while (line_read.back() != ';' && file.getline(line_read))
        {
            out.append(line_read);
        }
        statement.line = out;
        statement.table = sql_utility::get_table(out);
        statement.type = sql_utility::get_statement_type(out);

        assert_ending_with_column(statement);
    }

    return true;
}

void mysql64::assert_ending_with_column(const statement &statement) const
{
    /* assert line ending with ; */
    char *error = new char[100];
    sprintf(error, "statement [%s] at line %d not ending with comma", enum_to_string.at(statement.type).c_str(), statement.line_number);
    if (statement.line.back() != ';')
        throw std::runtime_error(error);
}

#endif
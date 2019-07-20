#ifndef __MYSQL64__
#define __MYSQL64__

#include "sql_operation.hpp"
#include "sql_utility.hpp"
#include "statement.hpp"
#include <cinttypes>
#include <filesystem>
#include "utility.hpp"


class mysql64 : public sql_operations
{
public:
	mysql64(const std::string& filename, const char* mode, int BUFFER_SIZE = 1024);

	void close()
	{
		if (file.is_open())
			file.close();
	}

	virtual void write(const statement& statement) override;
	virtual const bool read(statement& statement) override;

	const bool is_open() const { return file.is_open(); }
	void flush() { file.flush(); }

	const uint64_t size() const { return file.size(); }
	const std::string filename() const { return _filename; }
	const uint64_t position() const { return file.position(); }

private:
	void assert_ending_with_column(const statement&) const;

private:
	file64 file;
	std::string _filename;
};

/* implementation inside header file for avoiding strange linking error */

mysql64::mysql64(const std::string& filename, const char* mode, int BUFFER_SIZE) : _filename(filename), file{ filename.c_str(), mode, BUFFER_SIZE }
{
}

void mysql64::write(const statement& statement)
{
	file.writeline(statement.line);
}
const bool mysql64::read(statement& statement)
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

void mysql64::assert_ending_with_column(const statement& statement) const
{
	if (statement.line.back() != ';')
		utility::__throw("statement %s at " PRIu64 "%d not ending with column", statement_type_strings.at(statement.type), statement.line);
}

#endif
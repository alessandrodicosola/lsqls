#ifndef __STATEMENT__
#define __STATEMENT__

#define __MAX_LINE_STATEMENT_LENGTH__ 110

#include <string>
#include <ostream>
#include <unordered_map>

enum class statement_type : int
{
    INSERT = 1,
    CREATE = 2,
    DROP = 4,
    LOCK = 8,
    UNLOCK = 16,
    COMMENT = 32,
    EXECUTABLE_COMMENT = 64, /* only for MySQL */
    START_MULTILINE_COMMENT = 128,
    END_MULTILINE_COMMENT = 256,
    NONE = 512,
    TRASH = NONE | COMMENT | EXECUTABLE_COMMENT | START_MULTILINE_COMMENT | END_MULTILINE_COMMENT
};
const static std::unordered_map<statement_type, std::string> statement_type_strings{
    {statement_type::INSERT, "INSERT"},

    {statement_type::CREATE, "CREATE"},
    {statement_type::DROP, "DROP"},
    {statement_type::LOCK, "LOCK"},
    {statement_type::UNLOCK, "UNLOCK"},
    {statement_type::COMMENT, "COMMENT"},
    {statement_type::EXECUTABLE_COMMENT, "EXECUTABLE_COMMENT"},
    {statement_type::NONE, "NONE"}};

struct statement
{
	statement() : line(""),table(""),line_number(0),type(statement_type::NONE){}

	std::string line;
    std::string table;
    uint64_t line_number;
    statement_type type;
    const bool has_table() const { return !table.empty(); }
    const bool empty() const { return line.empty() || line == "\n"; }
    friend inline std::ostream &operator<<(std::ostream &out, const statement &s)
    {
        std::string trim_line = s.line;
        if (trim_line.length() > __MAX_LINE_STATEMENT_LENGTH__)
        {
            trim_line.erase(__MAX_LINE_STATEMENT_LENGTH__ + 1, trim_line.length() - 1);
            trim_line += "...";
        }
        out << s.line_number << "[" << statement_type_strings.at(s.type) << "]" << trim_line;
        return out;
    }
};

#endif
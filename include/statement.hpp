#ifndef __STATEMENT__
#define __STATEMENT__

#include <string>
#include <ostream>
#include <unordered_map>

#define ENUM_TO_STRING(enum) #enum
enum class statement_type : int
{
    INSERT=1,
    CREATE=2,
    DROP=4,
    LOCK=8,
    UNLOCK=16,
    COMMENT=32,
    EXECUTABLE_COMMENT=64, /* only for MySQL */
    START_MULTILINE_COMMENT=128,
    END_MULTILINE_COMMENT=256,
    NONE=512,
    TRASH = NONE | COMMENT | EXECUTABLE_COMMENT | START_MULTILINE_COMMENT | END_MULTILINE_COMMENT
};
const static std::unordered_map<statement_type,std::string> enum_to_string {
    {statement_type::INSERT,"INSERT"},

    {statement_type::CREATE,"CREATE"},
    {statement_type::DROP,"DROP"},
    {statement_type::LOCK,"LOCK"},
    {statement_type::UNLOCK,"UNLOCK"},
    {statement_type::COMMENT,"COMMENT"},
    {statement_type::EXECUTABLE_COMMENT,"EXECUTABLE_COMMENT"},
    {statement_type::START_MULTILINE_COMMENT,"START_MULTILINE_COMMENT"},
    {statement_type::END_MULTILINE_COMMENT,"END_MULTILINE_COMMENT"},
    {statement_type::NONE,"NONE"},
};


struct statement
{
std::string line;
std::string table;
uint64_t line_number;
statement_type type;
const bool has_table() const { return !table.empty(); }
};

std::ostream& operator <<(std::ostream& out,const statement& statement){
     std::string trim_line = statement.line;
    if (statement.line.length() > 16) {
        trim_line.erase(16,statement.line.length()-1);
        trim_line += "...";
    }
    out << statement.line_number << "[" << enum_to_string.at(statement.type) << "] " << trim_line; 
    return out;
}

#endif
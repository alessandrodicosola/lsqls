#ifndef __SQL_UTILITY__
#define __SQL_UTILITY__

#include <string>
#include <statement.hpp>
#include <regex>


class sql_utility
{
private:
    sql_utility()
    { /* singleton */
    }



public:
    static const bool is_comment(const std::string &line) { return line[0] == '#' || (line[0] == '-' && line[1] == '-'); }
    static const bool is_starting_multiline_comment(const std::string &line) { return line[0] == '/' && line[1] == '*'; }
    static const bool is_ending_multiline_comment(const std::string &line) { return line.substr(line.length() - 1, 2) == "*/"; };
    static const bool is_executable_comment(const std::string& line) {return line.substr(0,3) == "/*!";}
    
    static const statement_type get_statement_type(const std::string &statement)
    {
        std::string line = statement;
        line.erase(line.cbegin(),std::find_if(line.cbegin(),line.cend(),[](int c){ return !std::isspace(c);}));

        const std::string line6 = line.substr(0, 6);
        const std::string line4 = line.substr(0, 4);
        return line4 == "DROP" ? statement_type::DROP : line4 == "LOCK" ? statement_type::LOCK : line6 == "INSERT" ? statement_type::INSERT : line6 == "CREATE" ? statement_type::CREATE : line6 == "UNLOCK" ? statement_type::UNLOCK : statement_type::NONE;
    
    
    
    }
    static const std::string get_table(const std::string &line)
    {

    const static std::regex table_pattern{"`([0-9a-zA-Z$_]+?)`"};

        if (!line.find_first_of('`') && (!line.find_first_of("INSERT") || !line.find_first_of("CREATE") || !line.find_first_of("DROP"))) return "";
        std::smatch result;
        if (std::regex_search(line, result, table_pattern))
        {
            return result[1].str();
        }
        return "";
    }
};

#endif
#include "mysql64.hpp"
#include "assert.hpp"

int main(void){

    mysql64 file = mysql64{"/home/alessandro/src/lsqls/test/test.txt", FILE_MODE_WRITE};

    ASSERT_NOMSG(file.is_open());
    
    statement s1;
    s1.line = "INSERT INTO `Players` VALUES (\"name1\", 10)";
    s1.line_number = 1;
    s1.table = "Players";
    s1.type = statement_type::INSERT;

    file.write(s1);
    
    mysql64 file2 = mysql64{"/home/alessandro/src/lsqls/test/test.txt", FILE_MODE_READ};
    
    s1 = statement();
    
    ASSERT_NOMSG(file2.read(s1));

    ASSERT(s1.line == "INSERT INTO `Players` VALUES (\"name1\", 10)",s1.line);
    ASSERT(s1.line_number == 1,s1.line_number);
    ASSERT(s1.table == "Players",s1.table);
    ASSERT(s1.type == statement_type::INSERT,enum_to_string.at(s1.type));


    return 0;
}
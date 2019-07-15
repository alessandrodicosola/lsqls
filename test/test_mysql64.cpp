#include "mysql64.hpp"
#include "assert.hpp"

int main(void)
{

    mysql64 file = mysql64{"/home/alessandro/src/lsqls/test/sql_statement.txt", FILE_MODE_READ};

    ASSERT_NOMSG(file.is_open());

    statement s;

    while (file.read(s))
    {
        if (s.type == statement_type::EXECUTABLE_COMMENT)
        {
            if (s.line.find("ALTER TABLE"))
                ASSERT(s.has_table(), "expected table");
        }
    }

    return 0;
}
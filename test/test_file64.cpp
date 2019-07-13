#include <file64.hpp>
#include "assert.hpp"

int main()
{

    file64 file = file64{"/home/alessandro/src/lsqls/test/test.txt", FILE_MODE_WRITE_AND_READ, 1024};
    file.write_line("1");
    file.write_line("2");
    file.write_line("3\n4");
    file.start();

    std::string line1, line2, line3, line4;
    ASSERT(file.get_line(line1), line1);
    ASSERT(file.get_line(line2), line2);
    ASSERT(file.get_line(line3), line3);
    ASSERT(file.get_line(line4), line4);

    ASSERT_NOMSG(line1 == "1");
    ASSERT_NOMSG(line2 == "2");
    ASSERT_NOMSG(line3 == "3");
    ASSERT_NOMSG(line4 == "4");

    return 0;
}

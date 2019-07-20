#include "file64.hpp"
#include "assert.hpp"
#include <filesystem>


int main()
{
	auto path = std::filesystem::current_path() / "test.txt";

    file64 file = file64{path.string().c_str(), FILE_MODE_WRITE_AND_READ, 1024};
    file.writeline("1");
    file.writeline("2");
    file.writeline("3\n4");
    file.start();

    std::string line1, line2, line3, line4;
    ASSERT(file.getline(line1), line1);
    ASSERT(file.getline(line2), line2);
    ASSERT(file.getline(line3), line3);
    ASSERT(file.getline(line4), line4);

    ASSERT_NOMSG(line1 == "1");
    ASSERT_NOMSG(line2 == "2");
    ASSERT_NOMSG(line3 == "3");
    ASSERT_NOMSG(line4 == "4");

    return 0;
}

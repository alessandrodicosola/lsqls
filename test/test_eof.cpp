
#include <mysql64.hpp>
#include "assert.hpp"

const int expected_count = 52;



#if _WIN32 || _WIN64
#define TEST_DIR "C:\\Users\\aless\\source\\repos\\lsqls\\test\\eof.txt"
#elif __linux__
#define TEST_DIR "/home/alessandro/src/lsqls/test/eof.txt"
#endif


int main(void)
{
	mysql64 file(TEST_DIR,FILE_MODE_READ);

    statement s;
    int count = 0;
    while (file.read(s))
    {
        if (s.type == statement_type::COMMENT || s.line.empty() || s.line == "\n")
            continue;
        ++count;
        std::cout << s << ">>>>>" << count << "\n";
    }

    char *msg = new char[50];
    sprintf(msg, "expected: %d; read: %d ", expected_count,count);

    ASSERT(count == expected_count, msg);

    return 0;
}
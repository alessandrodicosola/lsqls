#include "mysql64.hpp"
#include "assert.hpp"

#if _WIN32 || _WIN64
#define TEST_DIR "C:\\Users\\aless\\source\\repos\\lsqls\\test\\statements.txt"
#elif __linux__
#define TEST_DIR "/home/alessandro/src/lsqls/test/statements.txt"
#endif
int main(void) {

  mysql64 file = mysql64{TEST_DIR, FILE_MODE_READ};

  ASSERT_NOMSG(file.is_open());

  statement s;

  while (file.read(s)) {
    char *val = new char[1024];
    sprintf(val, "Expected table for %s",
            statement_type_strings.at(s.type).c_str());

    if (s.type == statement_type::COMMENT || s.type == statement_type::NONE ||
        s.type == statement_type::UNLOCK)
      ASSERT_NOMSG(!s.has_table());
    else if (s.type == statement_type::EXECUTABLE_COMMENT) {
      if (s.line.find("ALTER TABLE") != std::string::npos)
        ASSERT(s.has_table(), val);
      else
        ASSERT_NOMSG(!s.has_table());
    } else
      ASSERT(s.has_table(), val);
  }

  return 0;
}
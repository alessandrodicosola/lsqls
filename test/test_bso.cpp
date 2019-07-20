#include "utility.hpp"
#include "assert.hpp"
#include "cinttypes"

int main(void)
{
    std::string ss = "500MB";
    uint64_t sn = utility::byte::bytes_from_string(ss);
    
    const uint64_t expected = 500 * 1024 * 1024;

    char *msg = new char[100];
    sprintf(msg, "expected" PRIu64 "%d; found: " PRIu64 "%d",expected,sn);
    ASSERT(sn == expected, msg);

    std::string ss2 = utility::byte::bytes_to_string(sn);
    sprintf(msg, "found %s", ss2.c_str());
    ASSERT(ss2 == "500MB", msg);
}
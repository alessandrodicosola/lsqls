#include "path.hpp"
#include "assert.hpp"

int main(void)
{
    path path{"/mnt/data/folder1/sub1/file.txt"};
    ASSERT(path.dir() == "/mnt/data/folder1/sub1",path.dir().c_str());
    ASSERT(path.filename() == "file.txt", path.filename().c_str());
    
    return 0;
}
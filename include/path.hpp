#ifndef __PATH__
#define __PATH__

#include <string>

class path
{
public:
    path() { }
    path(const std::string& path) : internal_path{path} { }

    const std::string filename() const { return internal_path.substr(internal_path.find_last_of("/")+1,internal_path.length()-1); }
    const std::string dir() const { return internal_path.substr(0,internal_path.find_last_of("/")); }    
private:
    std::string internal_path;
};

#endif
#ifndef __SQL_FILE__
#define __SQL_FILE__

#include "statement.hpp"
#include "file64.hpp"


struct sql_operations
{
    virtual void write(const statement &statement) = 0;
    virtual const bool  read(statement& statement) = 0;
};
#endif
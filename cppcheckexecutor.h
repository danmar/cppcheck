#ifndef CPPCHECKEXECUTOR_H
#define CPPCHECKEXECUTOR_H

#include "errorlogger.h"

class CppCheckExecutor : public ErrorLogger
{
    public:
        CppCheckExecutor();
        virtual ~CppCheckExecutor();
        void check( int argc, char* argv[] );
        void reportErr( const std::string &errmsg);
        void reportOut( const std::string &outmsg);
    protected:
    private:
};

#endif // CPPCHECKEXECUTOR_H

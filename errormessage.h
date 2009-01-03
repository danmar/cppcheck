#ifndef ErrorMessageH
#define ErrorMessageH

/**
 * This class is used by the Cppcheck application to get
 * informative error messages when e.g. memory leak is found
 * from the inspected source file. This is also used another
 * program to generate text for wiki and man page.
 */
class ErrorMessage
{
    public:
        ErrorMessage();
        virtual ~ErrorMessage();
    protected:
    private:
};

#endif // ErrorMessageH

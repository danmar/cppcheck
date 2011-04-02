#ifndef APPLICATION_H
#define APPLICATION_H

/**
* @brief A class containing information of the application to execute.
*
*/
class Application
{
public:
    /**
    * @brief Application's name
    *
    */
    QString Name;

    /**
    * @brief Application's path
    *
    */
    QString Path;

    /**
    * @brief Application's parameters
    *
    */
    QString Parameters;
};

#endif // APPLICATION_H

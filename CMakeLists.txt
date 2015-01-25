PROJECT(CppCheck)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE(cmake/versions.cmake REQUIRED)
INCLUDE(cmake/options.cmake REQUIRED)
INCLUDE(cmake/findDependencies.cmake REQUIRED)
INCLUDE(cmake/compileroptions.cmake REQUIRED)
INCLUDE(cmake/buildFiles.cmake REQUIRED)

ADD_SUBDIRECTORY(externals/tinyxml)
ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(cli) # Client application
#ADD_SUBDIRECTORY(utils)
#ADD_SUBDIRECTORY(utilsTests)

INCLUDE(cmake/printInfo.cmake REQUIRED)

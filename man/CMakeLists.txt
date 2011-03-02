# Build Docbook manual
# Run 'make html' to build manual

set(DOC_TARGETS "html")

if(CYGWIN)
   # Build also htmlhelp in Cygwin - this can be used to build
   # htmlhelp manual for Windows.
   set(DOC_TARGETS ${DOC_TARGETS} "htmlhelp")
endif()

if(UNIX OR CYGWIN)
   # Build manual in Linux/Cygwin - xmlto is not available in Windows
   find_package(XmlTo REQUIRED)
   xmlto("" "manual.docbook" MODES ${DOC_TARGETS})
endif()

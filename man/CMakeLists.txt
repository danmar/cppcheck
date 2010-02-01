# Build Docbook manual
# Run 'make html' to build manual

# Build manual in Linux/Cygwin - xmlto is not available in Windows

SET(DOC_TARGETS "html")

# Build also htmlhelp in Cygwin - this can be used to build
# htmlhelp manual for Windows.

IF (CYGWIN)
  SET(DOC_TARGETS
    ${DOC_TARGETS}
    "htmlhelp"
  )
ENDIF(CYGWIN)

IF (UNIX OR CYGWIN)
  #macro XMLTO(outfiles infiles... MODES modes...)
  INCLUDE("../CMake/xmlto.cmake")
  XMLTO("" "manual.docbook" MODES ${DOC_TARGETS})
ENDIF (UNIX OR CYGWIN)

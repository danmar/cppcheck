# Build Docbook manual
# Run 'make html' to build manual

# Build manual in Linux - xmlto is not available in Windows
IF (UNIX OR CYGWIN)
  #macro XMLTO(outfiles infiles... MODES modes...)
  INCLUDE("../CMake/xmlto.cmake")
  XMLTO("" "manual.docbook" MODES "html")
ENDIF (UNIX OR CYGWIN)

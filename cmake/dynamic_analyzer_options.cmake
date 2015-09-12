IF (USE_CLANG)
   SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -fdiagnostics-show-category=name")
ENDIF()

IF(ANALYZE_MEMORY)

   SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -fsanitize=memory")
   SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -fsanitize-memory-track-origins=2")
   SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -fno-omit-frame-pointer")
   # NOTE: tail call elimination -fno-optimize-sibling-calls

ELSEIF(ANALYZE_ADDRESS)

   SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -fsanitize=address")
   SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -fno-omit-frame-pointer")

ELSEIF(ANALYZE_THREAD)

   SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -fsanitize=thread")

ENDIF()

IF(ANALYZE_UNDEFINED)
   SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -fsanitize=undefined-trap")
   SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -fsanitize-undefined-trap-on-error")
   SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -fno-sanitize-recover")
ENDIF()

IF(ANALYZE_DATAFLOW)
   SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -fsanitize=dataflow")
ENDIF()

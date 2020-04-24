if(ANALYZE_MEMORY)
    add_compile_options(-fsanitize=memory)
    add_compile_options(-fsanitize-memory-track-origins=2)
    add_compile_options(-fno-omit-frame-pointer)
    add_compile_options(-fno-optimize-sibling-calls)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=memory")
elseif(ANALYZE_ADDRESS)
    add_compile_options(-fsanitize=address)
    add_compile_options(-fno-omit-frame-pointer)
    add_compile_options(-fno-optimize-sibling-calls)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
elseif(ANALYZE_THREAD)
    add_compile_options(-fsanitize=thread)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread")
endif()

if(ANALYZE_UNDEFINED)
    add_compile_options(-fsanitize=undefined)
    add_compile_options(-fno-sanitize-recover=all)
    add_compile_options(-fno-omit-frame-pointer)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
endif()

if(ANALYZE_DATAFLOW)
    add_compile_options(-fsanitize=dataflow)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=dataflow")
endif()

aux_source_directory(. DIR_LIB_SRCS)


IF (APPLE)
	MESSAGE(STATUS "Apple systens.")
    add_subdirectory(mac)
    add_library (net ${DIR_LIB_SRCS})
    target_link_libraries(net macnet)

ELSEIF (UNIX)
	MESSAGE(STATUS "UNIX-like OS's")
    add_subdirectory(linux)
    add_library (net ${DIR_LIB_SRCS})
    target_link_libraries(net linuxnet)
ENDIF ()


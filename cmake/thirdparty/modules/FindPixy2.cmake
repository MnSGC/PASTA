if (LIBPIXY2_LIBRARIES AND LIBPIXY2_INCLUDE_DIRS)
    # in cache already
    set(LIBPIXY2_FOUND TRUE)
else (LIBPIXY2_LIBRARIES AND LIBPIXY2_INCLUDE_DIRS)
    find_path(LIBPIXY2_INCLUDE_DIR
            NAMES
            libpixyusb2.h
            PATHS
            /usr/include
            /usr/local/include
            /opt/local/include
            /sw/include
            PATH_SUFFIXES
            libpixy2
            )

    find_library(LIBPIXY2_LIBRARY
            NAMES
            libpixy2.a
            PATHS
            /usr/lib
            /usr/local/lib
            /opt/local/lib
            /sw/lib
            PATH_SUFFIXES
            libpixy2
            )

    set(LIBPIXY2_INCLUDE_DIRS
            ${LIBPIXY2_INCLUDE_DIR}
            )
    set(LIBPIXY2_LIBRARIES
            ${LIBPIXY2_LIBRARY}
            )

    if (LIBPIXY2_INCLUDE_DIRS AND LIBPIXY2_LIBRARIES)
        set(LIBPIXY2_FOUND TRUE)
    endif (LIBPIXY2_INCLUDE_DIRS AND LIBPIXY2_LIBRARIES)

    if (LIBPIXY2_FOUND)
        if (NOT Pixy2_FIND_QUIETLY)
            message(STATUS "Found libpixy2:")
            message(STATUS " - Includes: ${LIBPIXY2_INCLUDE_DIRS}")
            message(STATUS " - Libraries: ${LIBPIXY2_LIBRARIES}")
        endif (NOT Pixy2_FIND_QUIETLY)
    else (LIBPIXY2_FOUND)
        if (Pixy2_FIND_REQUIRED)
            message(FATAL_ERROR "Could not find libpixy2")
        endif (Pixy2_FIND_REQUIRED)
    endif (LIBPIXY2_FOUND)

    # show the LIBPIXY2_INCLUDE_DIRS and LIBPIXY2_LIBRARIES variables only in the advanced view
    mark_as_advanced(LIBPIXY2_INCLUDE_DIRS LIBPIXY2_LIBRARIES)

endif (LIBPIXY2_LIBRARIES AND LIBPIXY2_INCLUDE_DIRS)

function(create_source_groups)
    set(GROUP_SEPARATOR "\\\\")
    cmake_parse_arguments(ARG "GROUP_PREFIX" "FILE_PREFIX" "FILES" ${ARGN})
    foreach (FILE ${ARG_FILES})
        string(REPLACE "${ARG_FILE_PREFIX}" "" GROUP_NAME ${FILE})
        string(REGEX REPLACE "^/" "" GROUP_NAME ${GROUP_NAME})
        get_filename_component(GROUP_NAME ${GROUP_NAME} PATH)
        string(REPLACE "/" "${GROUP_SEPARATOR}" GROUP_NAME "${GROUP_NAME}")
        list(APPEND _GROUP_LIST "${GROUP_NAME}")
        list(APPEND _GROUP_${GROUP_NAME}_LIST ${FILE})
    endforeach ()
    if (NOT ARG_GROUP_PREFIX OR (ARG_GROUP_PREFIX STREQUAL ""))
        set(GROUP_BASE "")
    else ()
        string(REPLACE "\\\\" "${GROUP_SEPARATOR}" ARG_GROUP_PREFIX "${ARG_GROUP_PREFIX}")
        set(GROUP_BASE "${ARG_GROUP_PREFIX}${GROUP_SEPARATOR}")
    endif ()
    foreach (GROUP ${_GROUP_LIST})
        source_group("${GROUP_BASE}${GROUP}" FILES ${_GROUP_${GROUP}_LIST})
    endforeach ()
    if (_GROUP__LIST AND (NOT GROUP_BASE STREQUAL ""))
        source_group("${GROUP_BASE}" FILES ${_GROUP__LIST})
    endif ()
endfunction ()
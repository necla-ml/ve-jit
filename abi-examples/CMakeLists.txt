# vim: et ts=4 sw=4 ai ft=cmake:
# minimally, just need to compile our own cmake
cmake_minimum_required(VERSION 2.6)
cmake_policy(SET CMP0011 NEW)
cmake_policy(SET CMP0012 NEW)
cmake_policy(SET CMP0026 NEW)
cmake_policy(SET CMP0048 NEW) # allow project VERSION
cmake_policy(SET CMP0056 NEW) # honor flags
cmake_policy(SET CMP0060 NEW)
cmake_policy(SET CMP0066 NEW) # honor flags
set(CMAKE_BUILD_TYPE Debug)
message(FATAL_ERROR "This is a relic from asm-examples subdirectory of the vtorch project to investigate C ABI")

project(asm-examples VERSION 0.1
	DESCRIPTION "assembler outputs (ABI testing)"
	LANGUAGES C)

# including asm.cmake gives very verbose listings
# (probably should output .lst file instead of .s
#  since it is not compilable)
message(STATUS "src ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "mod ${CMAKE_MODULE_PATH}")
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/asm.cmake)
    #set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH} ${CMAKE_CURRECT_SOURCE_DIR}/../../cmake")
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)
    message(STATUS "mod -> ${CMAKE_MODULE_PATH}")
    include(../../cmake/asm.cmake)
elseif(EXISTS $ENV{VE_ROOT}/cmake/asm.cmake)
    list(APPEND CMAKE_MODULE_PATH $ENV{VE_ROOT}/cmake)
    message(STATUS "mod -> ${CMAKE_MODULE_PATH}")
    include($ENV{VE_ROOT}/cmake/asm.cmake)
else()
    message(FATAL_ERROR "could not find asm.cmake from parent project")
endif()
message(STATUS "VE_ROOT=${VE_ROOT}")
message(STATUS "VE_ROOT=$ENV{VE_ROOT}")

file(GLOB _csources *.c)
list(SORT _csources)
message(STATUS "_csources = ${_csources}")

file(GLOB _casmSources *.S)
list(SORT _casmSources)
message(STATUS "_casmSources = ${_casmSources}")

string(REGEX REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/(.*)" "\\1.s"
    asm_targets ${_csources})
message(STATUS "asm_targets ${asm_targets}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11") # want to test bool support too, and extended asm syntax

add_library(simple SHARED ${_csources} ${_casmSources})
#set_target_properties(simple PROPERTIES EXCLUDE_FROM_ALL 1)
#get_target_property(_usrc simple SOURCES)
#message(STATUS "_usrc ${_usrc}")

set(asmList "")
foreach(_cfile IN LISTS _csources)
    #string(CONCAT _asmfile ${_cfile} ".s")
    string(REGEX REPLACE ".c$" ".s" _asmout ${_cfile})
    list(APPEND asmList ${_asmout})
    get_filename_component(_asm_fname ${_asmout} NAME)
    message(STATUS "${_cfile} --> ${_asmout} --> ${_asm_fname}")
    add_custom_command(TARGET simple PRE_BUILD
        COMMAND cd ${CMAKE_BINARY_DIR} && ${CMAKE_MAKE_PROGRAM} ${_asm_fname}
        )
endforeach()
foreach(_casmfile IN LISTS _casmSources)
    string(REGEX REPLACE ".S$" ".s" _asmout ${_casmfile})
    list(APPEND asmList ${_asmout})
    get_filename_component(_asm_fname ${_asmout} NAME)
    message(STATUS "${_cfile} --> ${_asmout} --> ${_asm_fname}")
    #ncc -Dunused_EXPORTS  -std=gnu11 -fPIC -Wa,-adhln -c void_fn_bool.c -o /dev/null > void_fn_bool.c.s.0
    #/usr/bin/cat void_fn_bool.c.s.0 | nc++filt > void_fn_bool.c.s
    #make -E copy void_fn_bool.c.s .../asm-examples/asm/

    # not sure that this works :)
    add_custom_command(TARGET simple PRE_BUILD
        COMMAND cd ${CMAKE_BINARY_DIR} && ${CMAKE_C_COMPILER} -std=gnu11 -fPIC $ENV{CFLAGS} -Wa,-adhln -c ${_casmfile} -o /dev/null > ${_asm_fname}
        #COMMAND ${CAT} ${_asm_fname}.0 | ${CXXFILT} > ${_asm_fname}
        COMMAND ${CMAKE_COMMAND} -E copy ${_asm_fname} ${CMAKE_CURRENT_SOURCE_DIR}/asm/
        )
endforeach()
add_custom_command(TARGET simple PRE_BUILD COMMAND echo "PRE_BUILD")
add_custom_command(TARGET simple PRE_LINK COMMAND echo "PRE_LINK")
add_custom_command(TARGET simple POST_BUILD COMMAND echo "POST_BUILD")

install(TARGETS simple LIBRARY DESTINATION lib)
#install(FILES simple.h DESTINATION include/simple)


# did not seem to work:
#install(DIRECTORY ./ DESTINATION share/asm-examples
#    FILES_MATCHING PATTERN "*.c")
#install(DIRECTORY asm/ DESTINATION share/asm-examples
#    FILES_MATCHING PATTERN "*.s")
#

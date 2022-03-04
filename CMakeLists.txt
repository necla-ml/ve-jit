cmake_minimum_required(VERSION 3.0)

# 1.7.x cannot compile libveli
set(ncc "/opt/nec/ve/bin/ncc")
set(ncxx "/opt/nec/ve/bin/nc++")
#set(ncc "/opt/nec/ve/bin/ncc-2.1.28" cache filepath "path of ncc")
#set(ncxx "/opt/nec/ve/bin/nc++-2.1.28" cache filepath "path of nc++")

option(USE_GCC      "Use gcc (target x86)" OFF)
option(USE_NCLANG   "Use clang (target VE)" OFF)
option(USE_NCC      "Use ncc (target VE)" ON)
option(USE_OPENMP   "Use OpenMP" ON)
option(BUILD_SHARED "Build Shared Library" OFF)
set(USE_FTRACE CACHE STRING "ftrace+veperf support [0]|1|2, 1 instruments less than 2" )
set_property(CACHE USE_FTRACE PROPERTY STRINGS OFF 1 2) # informative only
option(LOCAL_LLVM   "Try llvm+clang from .local? (use to avoid system clang, if nec.)" OFF)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT) # avoid default install --> /usr/local
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

######################################################
if(USE_GCC)
    set(USE_NCC OFF CACHE BOOL "" FORCE)
    set(USE_NCLANG OFF CACHE BOOL "" FORCE)
    set(USE_FTRACE "0" CACHE STRING "ftrace" FORCE)
endif()
if(USE_NCLANG)
    set(USE_NCC OFF CACHE BOOL "" FORCE)
    set(USE_FTRACE "0" CACHE STRING "ftrace" FORCE)
endif()
if(NOT USE_NCC)
    set(USE_NCC ON CACHE BOOL "" FORCE)
endif()

if(USE_GCC)
    set(CMAKE_C_COMPILER gcc)
    set(CMAKE_CXX_COMPILER g++)
elseif(USE_NCC)
    set(CMAKE_C_COMPILER ${ncc})
    set(CMAKE_CXX_COMPILER ${ncxx})
endif()
if(USE_NCLANG)
    set(CMAKE_C_COMPILER ${ncc})
    set(CMAKE_CXX_COMPILER ${ncxx})
endif()
message(STATUS "CMAKE_C_COMPILER   ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER}")


if(LOCAL_LLVM)
    # Tailor this if cmake still does not pick up your desired clang
    set(VEDNN_LLVM_HINT "~/.local")
    find_package(LLVM REQUIRED CONFIG
        # 0   : -DLLVM_DIR on cmake command line
        # 1,2 : CMAKE_PREFIX_PATH or CMAKE_PROGRAM_PATH (a: from cache, b: from env variable)
        # 3   : try a local path
        HINTS ${VEDNN_LLVM_HINT}
        # 4   : try a "system location"
        PATHS /proj/share/local/x86_64/llvm
        #PATH_SUFFIXES lib/cmake/clang lib/cmake/llvm
		NO_SYSTEM_ENVIRONMENT_PATH
		)
else()
	find_package(LLVM REQUIRED CONFIG)
endif()
message(STATUS "LLVM_TOOLS_BINARY_DIR : ${LLVM_TOOLS_BINARY_DIR}")

if(NOT USE_FTRACE) # enforce 0 or 1 or 2
    set(USE_FTRACE "0")
else() # specify USE_FTRACE 2 to instrument EVERY function, else just selected regions are ftraced
    if(USE_FTRACE STREQUAL "1")
    else()
        set(USE_FTRACE "2")
    endif()
endif()
message(STATUS "USE_FTRACE --------> ${USE_FTRACE}")
if(USE_FTRACE)
	# ftrace/veperf location
	find_file(found_VEPERF_H NAME veperf.h
	    NO_DEFAULT_PATH
	    PATHS /usr/uhome/aurora/mpc/pub/veperf/latest
	    PATH_SUFFIXES include
	    )
	message(STATUS "veperf.h --> ${found_VEPERF_H}")
	if(NOT found_VEPERF_H)
	    message(WARNING "ve.cmake: veperf.h not found (looking for veperf.h [ftrace.h might be there too])")
    else()
        get_filename_component(VE_VEPERF_INCLUDE "${found_VEPERF_H}" DIRECTORY)
        get_filename_component(VE_VEPERF_DIR "${VE_VEPERF_INCLUDE}" DIRECTORY)
        set(VE_VEPERF_DIR "${VE_VEPERF_DIR}" CACHE PATH "Aurora ftrace/veperf root directory" FORCE)
        set(VE_VEPERF_INCLUDE "${VE_VEPERF_INCLUDE}" CACHE PATH "Aurora ftrace/veperf include path" FORCE)
        set(VE_VEPERF_INCFLAGS "-I${VE_VEPERF_INCLUDE}" CACHE STRING "Aurora ftrace/veperf include path" FORCE)
        set(VE_VEPERF_LDFLAGS "-L${VE_VEPERF_DIR}/lib" CACHE STRING "Aurora ftrace/veperf C/CXX compile/link options" FORCE)
        set(VE_VEPERF_LIBS "-lveperf") # NOTE: there are static and shared libs
        mark_as_advanced(VE_VEPERF_DIR VE_VEPERF_INCLUDE VE_VEPERF_LDFLAGS VE_VEPERF_LIBS)
        # Q: What is difference between   libveperf and libveperf_sp
        # TODO: add library existence and compilation checks
        message(STATUS "veperf.h                         : ${found_VEPERF_H}")
        message(STATUS "VE_VEPERF_INCLUDE                : ${VE_VEPERF_INCLUDE}")
        message(STATUS "VE_VEPERF_DIR [ftrace|veperf]    : ${VE_VEPERF_DIR}")
        message(STATUS "VE_VEPERF_INCFLAGS               : ${VE_VEPERF_INCFLAGS}")
        message(STATUS "VE_VEPERF_LDFLAGS                : ${VE_VEPERF_LDFLAGS}")
        message(STATUS "VE_VEPERF_LIBS                   : ${VE_VEPERF_LIBS}")
        # Expected libraries: libveperf.{a|so} headers: ftrace.h veperf.h
        # add LDFLAGS="-lveperf" (nothing for ftrace)
        # VE_VEPERF_DIR for ftrace/veperf
        #
        # Note: If wanting to use CMAKE_FIND_ROOT_PATH, how does one automatically add lib subdirs?
        #  ... so I set up SYSTEM search paths instead ...
        #
        #set(CMAKE_SYSTEM_PREFIX_PATH  ${CMAKE_SYSTEM_PREFIX_PATH} ${VE_VEPERF_DIR})
        #set(CMAKE_SYSTEM_LIBRARY_PATH ${CMAKE_SYSTEM_LIBRARY_PATH} ${VE_VEPERF_DIR}/lib)
        #set(CMAKE_SYSTEM_INCLUDE_PATH ${CMAKE_SYSTEM_INCLUDE_PATH} ${VE_VEPERF_DIR}/include)
        #set(CMAKE_SYSTEM_PROGRAM_PATH ${CMAKE_SYSTEM_PROGRAM_PATH} ${VE_VEPERF_DIR}/bin)
        #
        set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -DFTRACE=${USE_FTRACE} -I${VE_VEPERF_INCLUDE}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFTRACE=${USE_FTRACE} -I${VE_VEPERF_INCLUDE}")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-rpath,${VEPERF_LIB_DIR}")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-rpath,${VEPERF_LIB_DIR}")
    endif()
    find_library(veperf NAMES veperf.a PATHS ${VE_VEPERF_DIR}/lib NO_DEFAULT_PATH)
endif()

# gcc needs gnuXX for MAP_ANONYMOUS to exist
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -std=gnu11 -D_GNU_SOURCE")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wall -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
#
# Changed: link with pthread compile removed from default flags because
#          ncc >= 2.0 cannot load dlls that need the VE pthread library.
# illegal instruction loading  "/opt/nec/ve/lib64/libthread_db.so.1"
#
#set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -pthread")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
if(BUILD_SHARED)
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
	set(LIB_MODE SHARED)
else()
	set(LIB_MODE STATIC)
endif()

message(STATUS "PROJECT_ROOT/ CMAKE_C_FLAGS=${CMAKE_C_FLAGS}")
#add_subdirectory(src)
#add_subdirectory(src/intrinsic)
#add_subdirectory(src/C)
#add_subdirectory(src/wrap)
set(LIBJIT jit1)
set(LIBVELI veli)
set(LIBSUFFIX "")
if(USE_OPENMP)
    set(LIBSUFFIX ${LIBSUFFIX}_omp)
endif()
if(USE_FTRACE)
    set(LIBSUFFIX ${LIBSUFFIX}_ft)
endif()
if(USE_GCC)
    set(LIBSUFFIX ${LIBSUFFIX}-x86)
elseif(USENCLANG)
    set(LIBSUFFIX ${LIBSUFFIX}-vi)
endif()
set(LIBJIT jit1${LIBSUFFIX})
set(LIBVELI veli${LIBSUFFIX})


if(USE_OPENMP)
    add_definitions(-DVEDNN_USE_OPENMP)
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fopenmp -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -pthread")
endif()

if(BUILD_SHARED_LIB)
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

add_library(${LIBJIT} ${LIB_MODE}
    $<TARGET_OBJECTS:jit1_code>
    )
add_library(jit1_code OBJECT
    asmfmt.cpp cblock.cpp dllbuild.cpp # original codes
    vechash.cpp asmblock.cpp cblock.cpp fuseloop.cpp ve_divmod.cpp # new codes
    jitpage.c bin_mk.c intutil.c
    )
add_custom_command(
    OUTPUT bin_mk.c
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ftostring
            ${CMAKE_CURRENT_SOURCE_DIR}/bin.mk
            bin_mk > bin_mk.c
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/bin.mk ${CMAKE_CURRENT_BINARY_DIR}/ftostring
    )
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ftostring
    COMMAND gcc
    ARGS    ${CMAKE_CURRENT_SOURCE_DIR}/ftostring.c -o ftostring
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ftostring.c
    COMMENT "Building ftostring file convert utility"
    )

if(NOT USE_GCC AND CAN_HANDLE_VELIFOO_CPP)
    add_library(${LIBVELI} ${LIB_MODE}
        $<TARGET_OBJECTS:veli_code>
        )
    add_library(veli_code OBJECT
        veliFoo.cpp wrpiFoo.cpp
        )
endif()

# vim: et ts=4 sw=4 ai

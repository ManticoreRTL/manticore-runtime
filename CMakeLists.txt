cmake_minimum_required(VERSION 3.21.2)

project(Manticore-Runtime)

# External libraries used
add_subdirectory(extlibs)

set(Boost_USE_STATIC_LIBS   ON)
# Boost::program_options are required for command line argument parsing
find_package(Boost 1.71
    COMPONENTS program_options filesystem
    REQUIRED)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(XRT REQUIRED)

# Include directories
set(INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${XRT_INCLUDE_DIR} ## XRT
    ${TFM_INCLUDE_DIR} ## Tiny Format
    ${INFINT_INCLUDE_DIR} ## InfInt for large integers
    ${JSON_INCLUDE_DIR} ## for json
)

# Source files
set(CPP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/manticore-manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/string-format.cpp
)

# Linkable libraries

set(EXT_LIBS
    ${XRT_LIBRARIES}
    ${Boost_LIBRARIES}
)

add_library(mantilib STATIC ${CPP_SOURCES})
target_include_directories(mantilib PUBLIC ${INCLUDE_DIRS})
target_link_libraries(mantilib PUBLIC ${EXT_LIBS})
target_compile_features(mantilib PUBLIC cxx_std_14)


add_executable(manticore ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_include_directories(manticore PUBLIC ${INCLUDE_DIRS})
# target_link_directories(manticore PUBLIC ${XRT_LIB_DIR})
target_link_libraries(manticore PUBLIC mantilib)
target_compile_features(manticore PUBLIC cxx_std_14)



# Tests
enable_testing()

add_subdirectory(test)


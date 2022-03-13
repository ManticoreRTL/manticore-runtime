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
)

# Source files
set(CPP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/manticore-manager.cpp
)

# Linkable libraries

set(EXT_LIBS

    ${XRT_LIBRARIES}
    ${Boost_LIBRARIES}
)

add_executable(manticore ${CPP_SOURCES})
target_include_directories(manticore PUBLIC ${INCLUDE_DIRS})
# target_link_directories(manticore PUBLIC ${XRT_LIB_DIR})
target_link_libraries(manticore PUBLIC ${XRT_LIBRARIES} ${Boost_LIBRARIES})
target_compile_features(manticore PRIVATE cxx_std_14)



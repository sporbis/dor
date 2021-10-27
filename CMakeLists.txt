cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(dor)

find_package(OpenSSL REQUIRED)

add_subdirectory(ThirdParty EXCLUDE_FROM_ALL)

file(
    GLOB_RECURSE SOURCES
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "Source/*.cpp"
)

add_executable(dor ${SOURCES})

foreach(source IN LISTS SOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach()

target_link_libraries(dor cxxopts ${OPENSSL_LIBRARIES})

target_include_directories(dor PUBLIC ${OPENSSL_INCLUDE_DIR})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT dor)
set_target_properties(
    dor PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

project(server)

include_directories(./)

file(GLOB SOURCES
    ${PROJECT_SOURCE_DIR}/*.h
    ${PROJECT_SOURCE_DIR}/*.c
)

add_executable(server.out ${SOURCES})
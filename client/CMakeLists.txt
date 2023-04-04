project(client)

include_directories(./)
include_directories(../common)

file(GLOB SOURCES
    ${PROJECT_SOURCE_DIR}/*.h
    ${PROJECT_SOURCE_DIR}/*.c
    ${PROJECT_SOURCE_DIR}/../common/*.h
    ${PROJECT_SOURCE_DIR}/../common/*.c
)

add_executable(client.out ${SOURCES})

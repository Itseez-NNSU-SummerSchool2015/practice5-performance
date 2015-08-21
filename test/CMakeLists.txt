set(target practice5_tests)

FILE(GLOB hdrs "*.h*")
FILE(GLOB srcs "*.cpp")

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/gtest")

# VS2012 doesn't support correctly the tuples yet,
# see http://code.google.com/p/googletest/issues/detail?id=412
if(MSVC)
    ADD_DEFINITIONS(/D _VARIADIC_MAX=10)
endif()

ADD_EXECUTABLE(${target} ${srcs} ${hdrs})
TARGET_LINK_LIBRARIES(${target} gtest retrofilter)

add_custom_command(
    TARGET practice5_tests
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/testdata" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/testdata"
)

set(INCLUDE_DIR "${voxel-io_SOURCE_DIR}/include")
set(TEST_INCLUDE_DIR "${INCLUDE_DIR}/voxelio/test")

add_executable(voxelio-test
               main.cpp
               test.cpp

               bits.cpp
               color.cpp
               stringmanip.cpp
               stream.cpp
               hextree.cpp
               palette.cpp

               "${TEST_INCLUDE_DIR}/test.hpp"
               "${TEST_INCLUDE_DIR}/random.hpp")

target_include_directories(voxelio-test PUBLIC "${INCLUDE_DIR}")

target_link_libraries(voxelio-test PRIVATE voxelio)

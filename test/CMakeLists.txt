set(INCLUDE_DIR "${voxel-io_SOURCE_DIR}/include")
set(TEST_INCLUDE_DIR "${INCLUDE_DIR}/voxelio/test")

add_executable(voxelio-test
               main.cpp
               test.cpp
               io.cpp
               random.cpp

               test_bits.cpp
               test_color.cpp
               test_stringmanip.cpp
               test_stream.cpp
               test_hextree.cpp
               test_palette.cpp

               "${TEST_INCLUDE_DIR}/test.hpp"
               "${TEST_INCLUDE_DIR}/io.hpp"
               "${TEST_INCLUDE_DIR}/random.hpp")

target_include_directories(voxelio-test PUBLIC "${INCLUDE_DIR}")

target_link_libraries(voxelio-test PRIVATE voxelio)

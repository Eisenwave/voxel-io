#include "voxelio/test/test.hpp"

namespace voxelio::test {

static int testFailureCount = 0;

static constexpr const char *TEST_ORDER[]{
    "intlog", "ileave", "wileave", "stringify", "parse", "stringmanip", "color32", "hextree", "palette"};

int runTests()
{
    setLogLevel(voxelio::LogLevel::DEBUG);
    enableLoggingSourceLocation(voxelio::build::DEBUG);

    VXIO_LOG(INFO, "Running " + stringify(testCount()) + " tests ...");

    setTestOrder(TEST_ORDER, std::size(TEST_ORDER));

    static const char *previousCategory = "";
    forEachTest([](Test &test) {
        if (std::strcmp(previousCategory, test.category) != 0) {
            previousCategory = test.category;
            VXIO_LOG(IMPORTANT, "Category: \"" + std::string{test.category} + "\" tests");
        }
        VXIO_LOG(INFO, "Running \"" + std::string{test.name} + "\" ...");
        testFailureCount += not test.operator()();
    });

    if (testFailureCount == 0) {
        VXIO_LOG(IMPORTANT, "All tests passed");
    }
    else {
        VXIO_LOG(ERROR, "Some tests failed (" + stringify(testFailureCount) + " total)");
    }
    return testFailureCount;
}

}  // namespace voxelio::test

int main()
{
    return voxelio::test::runTests();
}

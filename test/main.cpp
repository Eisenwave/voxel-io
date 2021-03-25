#include "voxelio/test/io.hpp"
#include "voxelio/test/test.hpp"

namespace voxelio::test {
namespace {

static int testFailureCount = 0;

static constexpr const char *TEST_ORDER[]{"intlog",
                                          "intdiv",
                                          "ileave",
                                          "wileave",
                                          "stringify",
                                          "parse",
                                          "stringmanip",
                                          "color32",
                                          "hextree",
                                          "palette",
                                          "stream",
                                          "deflate",
                                          "binvox",
                                          "vl32",
                                          "qef",
                                          "qb",
                                          "vox"};

void runTest(const Test &test)
{
    static thread_local const char *previousCategory = "";

    if (std::strcmp(previousCategory, test.category) != 0) {
        previousCategory = test.category;
        VXIO_LOG(IMPORTANT, "Category: \"" + std::string{test.category} + "\" tests");
    }
    VXIO_LOG(INFO, "Running \"" + std::string{test.name} + "\" ...");
    testFailureCount += not test.operator()();
}

int runTests()
{
    VXIO_LOG(INFO, "Running " + stringify(getTestCount()) + " tests ...");
    VXIO_LOG(DEBUG, "Test assets at: " + std::string{ASSET_PATH});

    setTestOrder(TEST_ORDER, std::size(TEST_ORDER));
    forEachTest(&runTest);

    if (testFailureCount == 0) {
        VXIO_LOG(IMPORTANT, "All " + stringify(getTestCount()) + " tests passed");
    }
    else {
        VXIO_LOG(ERROR,
                 "Some tests failed (" + stringify(testFailureCount) + "/" + stringify(getTestCount()) + ") total");
    }
    return testFailureCount;
}

}  // namespace
}  // namespace voxelio::test

int main()
{
    voxelio::setLogLevel(voxelio::LogLevel::DEBUG);
    voxelio::enableLoggingSourceLocation(voxelio::build::DEBUG);
    voxelio::enableLoggingTimestamp(false);

    return voxelio::test::runTests();
}

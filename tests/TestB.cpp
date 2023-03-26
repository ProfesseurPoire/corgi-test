#include <corgi/test/test.h>

using namespace corgi::test;

TEST(TestB, Check)
{
    assert_that(true, equals(true));
}

TEST(TestB, CheckEquals)
{
    assert_that(true, equals(true));
}

TEST(TestB, CheckNonEquals)
{
    assert_that(10, non_equals(5));
}
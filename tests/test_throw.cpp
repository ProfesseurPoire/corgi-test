#include <corgi/test/test.h>

#include <stdexcept>

static void nothrow_function() {}
static void throw_function()
{
    throw std::invalid_argument("hi");
}

TEST(check_exceptions, nothrow)
{
    check_no_throw(nothrow_function());
}

TEST(check_exceptions, throw_exception)
{
    check_throw(throw_function());
}
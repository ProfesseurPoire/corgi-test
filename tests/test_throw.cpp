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
    check_throw(throw_function(), std::invalid_argument);
    check_throw(throw_function(), std::invalid_argument);
}

TEST(check_exceptions, throw_any)
{
    check_any_throw(throw_function());
}
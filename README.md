# corgi-test

## Introduction

corgi-test is a lightweight C++ header only testing library. It provides a set of features designed to facilitate testing applications, unit testing and benchmarking.

## How to Install

Since it's a header only library, the simplest way to install it is to copy the test.h header file directly somewhere into your project and update the include path of the build system of your choice.

If you're using CMake to build your project, you could also simply copy the repository inside your project and link your project/executable with the corgi-test target.

```cmake
target_link_libraries(your_project corgi-test)
```

You can also run cmake and then install the library (using for instance make install) and use the following command to find it in your cmake.

```cmake
find_package(corgi-test CONFIG)
```

## How to use

First, we create a new file "main_test.cpp" that will contain the main function of our testing program. In the main function, we call the run_all() function that will run every testing function detected by the testing framework.

>:page_facing_up:  main_test.cpp

```cpp
#include <corgi/test/test.h>

int main()
{
    return corgi::test::run_all();
}
```

Now, we create another file "test_math.cpp". This file will contain tests about our mathematic library.

We start by include the deader to our library, and we use the TEST macro to declare a new test.

The macro takes 2 parameters. The first one correspond to the group name, used to regroup tests that share a common theme, and the second one is the actual test name. 

```cpp
#include <corgi/test/test.h>

float mult(float a, float b){ return a*b;}

TEST(Math, Multiplication)
{
    const int a=2;
    const int b=2;

    auto result = mult(a,b);

    check_equal(result, 4);
}
```

Inside the macro, we often want to do 3 things : Arrange, Act and Assert (AAA).
* Arrange : Setting up data for testing
* Act  : Perform an action.
* Assert : Checking if the result of the action is what we expected.

In our example, the Arrange correspond to the part where we declare the a and b variables. Act correspond to the part where we call the mult function. And the assert section correspond to the check_equal function.

The check_equal function is an assertion that will pass if the first argument is equal to the second one. If an assertion isn't met, the test won't pass and it'll be logged.

Note that check_equal isn't the only assertion available.

This is what the console should display for our test example after we run the exectable.

```cpp
+----------------------------------------------------------------------------+
|    Running 1 tests grouped in Math                                         |
+----------------------------------------------------------------------------+
  * Running Math.Multiplication (1/1)
       Passed in 0.017000 ms
+----------------------------------------------------------------------------+
|    Results                                                                 |
+----------------------------------------------------------------------------+
    * Every test passed
```

## Assertions

### check_equals

The assertion pass if value1 is equals to value2

```cpp
check_equals(value1, value2)
```

### check_non_equals

The assertion pass if value1 is not equals to value2

```cpp
check_non_equals(value1, value2)
```

### check_throw

Checks that an exception of a specific **type** was thrown by **statement**.

```cpp
check_throw(statement, type)
```

**Example :**

```cpp
static void throw_function()
{
    throw std::invalid_argument("hi");
}

TEST(check_exceptions, throw_exception)
{
    check_throw(throw_function(), std::invalid_argument);
}
```

### check_any_throw

Checks that an exception of any type was thrown by **statement**.

```cpp
check_any_throw(statement)
```

**Example :**

```cpp
static void throw_function()
{
    throw std::invalid_argument("hi");
}
TEST(check_exceptions, throw_any)
{
    check_any_throw(throw_function());
}
```

### check_no_throw

Checks that no exception was thrown by **statement**.

```cpp
    check_no_throw(statement)
```

**Example :**

```cpp
static void nothrow_function() {}

TEST(check_exceptions, nothrow)
{
    check_no_throw(nothrow_function());
}
```
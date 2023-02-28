#include <corgi/test/test.h>

std::vector<int> v1(10000);
std::vector<int> v2(20000);

void first_function()
{
    std::sort(v1.begin(), v1.end());
}

void second_function()
{
    std::sort(v2.begin(), v2.end());
}

int main()
{
    std::srand(unsigned(std::time(nullptr)));
    std::generate(v1.begin(), v1.end(), std::rand);
    std::generate(v2.begin(), v2.end(), std::rand);

    // I'm considering going this way instead of the TEST macro
    corgi::test::add_benchmark("first_benchmark", 10, first_function, "small vector",
                               second_function, "big_vector");

    corgi::test::add_test("group_test", "name_test",
                          []() -> void { assert_that(true, corgi::test::equals(true)); });

    return corgi::test::run_all();
}
/**
 * 
+------------+----------+----------+-----------+           
| Total Time | Max Time | Min Time | Mean Time |
+------------+----------+----------+-----------+
|            |          |          |           |
+------------+----------+----------+-----------+
*/
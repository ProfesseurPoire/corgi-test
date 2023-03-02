#pragma once

#include <chrono>
#include <ctime>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

/*!
 * @brief      Provides a framework to make test driven development easier
 * @details    Use the TEST macro to define your testing functions. 
 * The framework use this macro to register your tests inside the 
 * @ref _test_functions variable.
 *    
 * After defining a test function through the TEST macro,use the CHECK, CHECK_EQ, CHECK_NE
 * on conditions that can fail. If a check fails, an error will be automatically
 * notified and logged in the console through std::cerr
 *
 * To launch all the register test functions, use the @ref start function
*/
namespace corgi
{
namespace test
{

template<class T>
using unique_ptr = std::unique_ptr<T>;
template<class T>
using vector = std::vector<T>;
template<class T, class U>
using map    = std::map<T, U>;
using string = std::string;

namespace detail
{
inline void log_failed_functions();
template<class T>
inline int register_fixture(const string& class_name, const string& test_name);
}    // namespace detail

/*!
 * @brief Base class used to define fixtures
 */
class Test
{
    friend void detail::log_failed_functions();
    template<class T>
    friend int detail::register_fixture(const string& class_name,
                                        const string& test_name);

    friend void run_fixtures();

public:
    /*!
     * @brief Override this function to initialize the fixture resources
     */
    virtual void set_up() {}

    /*!
     * @brief Override this function to release the fixture resources
     */
    virtual void tear_down() {}
    virtual ~Test() = default;

private:
    string _class_name;
    string _test_name;

    /*!
     * @brief Overriden by the TEST_F macro
     */
    virtual void run() {}
};

template<class T>
class Equals
{
public:
    Equals(const T vv1)
        : _val1(vv1)
    {
    }

    template<class U>
    bool run(U val2)
    {
        return this->_val1 == val2;
    }

    T _val1;
};

template<class T>
class NonEquals
{
public:
    NonEquals(T v1)
        : _val1(v1)
    {
    }

    template<class U>
    bool run(U val2)
    {
        return this->_val1 != val2;
    }

    T _val1;
};

template<class T>
class AlmostEquals
{
public:
    AlmostEquals(T v1, T precision)
        : _val1(v1)
        , precision(precision)
    {
    }

    bool run(T val2)
    {
        return (this->_val1 > (val2 - precision)) && (this->_val1 < (val2 + precision));
    }

    T _val1;
    T precision;
};

// These functions allows us to write equals(4) instead of Equals<int>(4)
template<class T>
unique_ptr<Equals<T>> equals(T val)
{
    return std::make_unique<Equals<T>>(val);
}
template<class T>
unique_ptr<NonEquals<T>> non_equals(T val)
{
    return std::make_unique<NonEquals<T>>(val);
}
template<class T>
unique_ptr<AlmostEquals<T>> almost_equals(T val, T precision)
{
    return std::make_unique<AlmostEquals<T>>(val, precision);
}

namespace detail
{
enum class color
{
    Black,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White
};

// Variables

inline map<string, map<string, std::function<void()>>> map_test_functions;
inline map<string, vector<std::unique_ptr<Test>>>      fixtures_map;
inline vector<Test*>                                   failed_fixtures;
inline map<string, map<string, std::function<void()>>> failed_functions;

inline int error {0};

inline color current_color {color::White};

const inline std::map<color, string> color_code    // can't constexpr sadly
    {
        {color::Black, "30m"},  {color::Red, "31m"},   {color::Green, "32m"},
        {color::Yellow, "33m"}, {color::Blue, "34m"},  {color::Magenta, "35m"},
        {color::Cyan, "36m"},   {color::White, "37m"},
    };

// Some code are probably wrong here
const inline std::map<color, int> win_color_code {
    {color::Black, 1}, {color::Red, 4},     {color::Green, 2}, {color::Yellow, 14},
    {color::Blue, 5},  {color::Magenta, 6}, {color::Cyan, 3},  {color::White, 8},
};

/*!
 * @brief Just a shortcut so I don't have to write std::cout<< text << "\n" all the time
 */
inline void write_line(const string& str)
{
    std::cout << "\033[0;" << color_code.at(current_color) << str.c_str() << "\033[0m"
              << std::endl;
}

inline void write(const string& str)
{
    std::cout << "\033[0;" << color_code.at(current_color) << str.c_str() << "\033[0m"
              << std::flush;
}

inline void write(const string& str, color code_color)
{
    current_color = code_color;
    write(str);
}

/*!
 * @brief  Write a line of text and changes the console color
 * @param line         Contains the text to be displayed
 * @param console_color   Contains a code corresponding to a color
 */
inline void write_line(const string& line, color console_color)
{
    current_color = console_color;
    write_line(line);
}

template<class T>
void log_test_error(const T       val,
                    const string& value_name,
                    const string& expected,
                    const char*   file,
                    int           line)
{
    write_line("\n        ! Error : ", color::Red);
    write("            * file :     ", color::Cyan);
    write_line(file, color::Yellow);
    write("            * line :     ", color::Cyan);
    write_line(std::to_string(line), color::Magenta);
    write("            * Check if ", color::Cyan);
    write("\"" + value_name + "\" ", color::Magenta);
    write("== ");
    write_line("\"" + expected + "\"");
    write("                * Expected : ", color::Cyan);
    write_line(expected, color::Magenta);
    write("                * Value is : ", color::Cyan);

    std::stringstream ss;
    ss << val;
    write_line(ss.str(), color::Magenta);
    error += 1;
}

/*!
 * @brief 
 */
template<class T, class U>
void assert_that_(T             val,
                  U&&           checker,
                  const string& value,
                  const string& expected,
                  const char*   file,
                  int           line)
{
    if(checker->run(val) == false)
        log_test_error(val, value, expected, file, line);
}

/*!
 * @brief      Register a test function
 *  Called by the TEST macro.  The TEST macro declares a function 
 *  that will be named from the combination of the @ref group_name and @ref
 *  function_name parameters.
 *  Then, it will create a unique variable whose only goal is to call
 *  the register_function with a pointer to the previously declared function, along
 *  with the function name and the group (parameters of the TEST macro)
 *  Finally, it will start the function definition.
 *  @param func             Pointer to the test function created by the TEST macro
 *  @param group_name       First parameter of the TEST macro. Correspond to the group
 *                          in which the function belong
 *  @param function_name    Second parameter of the TEST macro. Correspond to the 
 *  function name.
 */
inline int
register_function(void (*func_ptr)(), const string& function, const string& group)
{
    map_test_functions[group][function] = func_ptr;
    return 0;    // We only return a value because of the affectation trick in the macro
}

/*!
 *   @brief Register a fixture object
 */
template<class T>
inline int register_fixture(const string& class_name, const string& test_name)
{
    try
    {
        T* t           = new T();
        t->_class_name = class_name;
        t->_test_name  = test_name;

        fixtures_map[class_name].emplace_back(t);
        return 0;    // We only return a value because of the affectation trick in the macro
    }
    catch(const std::exception& e)
    {
        std::cout << " ERROR " << e.what() << std::endl;
        return 0;
    }
}

/*!
 * @brief  Logs every failed function inside the console as a summary report
 */
inline void log_failed_functions()
{
    for(const auto& [group_name, group] : detail::failed_functions)
    {
        for(const auto& [function_name, function] : group)
        {
            write_line("      * Function " + group_name + "::" + function_name +
                           " failed",
                       color::Red);

            for(const auto& test_function : detail::failed_fixtures)
                write_line("      * " + test_function->_class_name +
                               "::" + test_function->_test_name + " failed",
                           color::Red);
        }
    }
}

/*!
 * @brief  Logs that we successfully pass every test
 */
inline void log_success()
{
    write_line("    * Every test passed", color::Green);
}

/*!
 * @brief  Logs that at least 1 test failed
 */
inline void log_failure()
{
    write_line("    Error : Some test failed to pass", color::Red);
    write_line("    Logging the Functions that failed", color::Cyan);
    log_failed_functions();
}

/*!
 *   @brief  Write an header in the console
 *   @detail Just write something in that way inside the console
 *   +---------+
 *   |   Text  |  
 *   +---------+
 */
inline void write_title(const string& text)
{
    const int max_column = 78;

    write("+", color::Green);
    write(string(max_column - 2, '-'));
    write("+\n");

    write_line("|    " + text + string(max_column - 1 - (5 + text.size()), ' ') + "|");

    write("+");
    write(string(max_column - 2, '-'));
    write("+\n");
}

/*!
 *   @brief  Logs that we're running tests that belongs to the @ref group_name group
 *   @param  group_name  Name of the tested group 
 *   @param  group_size  How many test are inside the group
 */
inline void log_start_group(const string& group_name, size_t group_size)
{
    write_title("Running " + std::to_string(group_size) + " tests grouped in " +
                group_name);
}

/*!
 * @brief Log that we're starting a test
 * @param test_name    Name of the test we're running
 * @param group_name   Name of the group the test belongs to
 * @param count        Counter used to know on which test from group we're (like
 * the first one, the second one ? etc
 */
inline void log_start_test(const string& test_name,
                           const string& group_name,
                           size_t        group_size,
                           size_t        count)
{
    write("  * Running ", color::Cyan);
    write(group_name + "." + test_name, color::Yellow);
    write(" (" + std::to_string(count) + "/" + std::to_string(group_size) + ")\n",
          color::Cyan);
}

/*!
 * @brief  Log that a test was successful
 */
inline void log_test_success(long long time)
{
    write_line("       Passed in " + std::to_string(static_cast<double>(time) / 1000.0) +
                   " ms",
               color::Green);
}
}    // namespace detail

// register the time it takes for a function to run
inline auto function_time(std::function<void()> fun) -> long long
{
    const auto start = std::chrono::high_resolution_clock::now();
    fun();
    const auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

inline void run_fixtures()
{
    for(auto& fixture : detail::fixtures_map)    // Loop through every fixture
    {
        auto total_test = detail::fixtures_map[fixture.first].size();
        int  test_index {1};

        detail::log_start_group(fixture.first, total_test);

        for(auto& test : fixture.second)    // loop through every fixture's test
        {
            detail::log_start_test(test->_test_name, test->_class_name, total_test,
                                   test_index++);

            // The assert function will increment the error count if something
            // went wrong. So we just registered how many error we had before
            // running the fixture, and compare afterward to know if the
            // fixture was a success or not
            int error_value = detail::error;

            test->set_up();
            auto time = function_time([&]() { test->run(); });
            test->tear_down();

            if(error_value == detail::error)
                detail::log_test_success(time);
            else
            {
                //detail::get_failed_fixtures().push_back(test_object);
            }
            std::cout << std::flush;
        }
    }
}

inline void run_functions()
{
    for(const auto& [group_name, group] : detail::map_test_functions)
    {
        auto total_test = group.size();
        detail::log_start_group(group_name, total_test);
        int test_index {1};

        for(const auto& [test_name, test] : group)
        {
            detail::log_start_test(test_name, group_name, total_test, test_index++);

            int  error_value = detail::error;
            auto time        = function_time(test);

            if(error_value == detail::error)
                detail::log_test_success(time);
            else
                detail::failed_functions[group_name].emplace(test_name, test);
        }
    }
}

struct benchmark
{
    std::function<void()> first_function;
    std::string           first_function_name;
    std::function<void()> second_function;
    std::string           second_function_name;
    int                   repetition;
    std::string           name;
};

static inline std::vector<benchmark> benchmarks;

inline void add_benchmark(std::string name,
                          int         repetition,
                          void (*first_function)(),
                          const std::string& first_function_name,
                          void (*second_function)(),
                          const std::string& second_function_name)
{
    benchmarks.push_back(benchmark(
        std::function<void()>(first_function), first_function_name,
        std::function<void()>(second_function), second_function_name, repetition, name));
}

inline void add_test(const std::string&    group_name,
                     const std::string&    test_name,
                     std::function<void()> lambda)
{
    corgi::test::detail::map_test_functions[group_name].emplace(test_name, lambda);
}

struct benchmark_function_result
{
    long long total_time = 0;
    long long max_time   = std::numeric_limits<long long>::min();
    long long min_time   = std::numeric_limits<long long>::max();
};

struct benchmark_result
{
    benchmark_function_result first_function_results;
    benchmark_function_result second_function_results;
};

inline benchmark_function_result run_benchmark_function(std::function<void()> function,
                                                        int                   repetition)
{
    benchmark_function_result result;

    for(int i = 0; i < repetition; i++)
    {
        auto time = function_time(function);
        result.total_time += time;

        result.max_time = std::max(result.max_time, time);
        result.min_time = std::min(result.min_time, time);
    }

    corgi::test::detail::write_line(
        "\t* Total Time : " +
            std::to_string(static_cast<double>(result.total_time) / 1000.0) + " ms",
        corgi::test::detail::color::Magenta);
    corgi::test::detail::write_line(
        "\t* Max Time : " +
            std::to_string(static_cast<double>(result.max_time) / 1000.0) + " ms",
        corgi::test::detail::color::Magenta);
    corgi::test::detail::write_line(
        "\t* Min Time : " +
            std::to_string(static_cast<double>(result.min_time) / 1000.0) + " ms",
        corgi::test::detail::color::Magenta);
    corgi::test::detail::write_line(
        "\t* Mean Time : " +
            std::to_string(static_cast<double>(result.total_time / repetition) / 1000.0) +
            " ms",
        corgi::test::detail::color::Magenta);
    return result;
}
inline void run_benchmark(benchmark& benchmark)
{
    benchmark_result result;
    corgi::test::detail::write("    * Benchmarking function " +
                                   benchmark.first_function_name + "\n",
                               corgi::test::detail::color::Green);
    result.first_function_results =
        run_benchmark_function(benchmark.first_function, benchmark.repetition);
    corgi::test::detail::write("    * Benchmarking function " +
                                   benchmark.second_function_name + "\n",
                               corgi::test::detail::color::Green);
    result.second_function_results =
        run_benchmark_function(benchmark.second_function, benchmark.repetition);

    if(result.first_function_results.total_time >=
       result.second_function_results.total_time)
        corgi::test::detail::write("    *" + benchmark.first_function_name +
                                       " was faster\n",
                                   corgi::test::detail::color::Cyan);
    else
        corgi::test::detail::write("    *" + benchmark.second_function_name +
                                       " was faster\n",
                                   corgi::test::detail::color::Cyan);
}

inline void run_benchmarks()
{
    if(benchmarks.empty())
        return;

    corgi::test::detail::write_title("Running benchmarks");
    for(auto benchmark : benchmarks)
    {
        corgi::test::detail::write("  * Running ", corgi::test::detail::color::Cyan);
        corgi::test::detail::write(benchmark.name + "\n",
                                   corgi::test::detail::color::Yellow);
        run_benchmark(benchmark);
    }
}

/*!
 * @brief      Run all the tests defined by the user 
 * @details    Must be called from main. Will fire all the test the user defined
 * with the TEST macro. Warning, this function returns a value that must be returned
 * inside the main function!
 */
inline int run_all()
{
    try
    {
        run_fixtures();
        run_functions();
        run_benchmarks();
        corgi::test::detail::write_title("Results");
        (detail::error == 0) ? corgi::test::detail::log_success() :
                               corgi::test::detail::log_failure();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return detail::error;    // Must return 0 to pass
}

/*!
 *   @brief  Define a new Fixture
 *
 *   A fixture is simply a way to organize tests around a class.
 *   To create a fixture, the user first needs to create a class
 *   that inherits from @ref corgi::test::Test.
 *   This class comes with 2 virtual methods the user needs to override :
 *   set_up and tear_down.
 *   In set_up, the user is expected to initialize the data that will be
 *   used by all the tests of the current feature.
 *   In tear_down, the user is expected to clean the data used by the test.
 *
 *   Once this is done, to add a test to the fixture, the user should use
 *   the TEST_F macro, and define the test. What the framework will do is
 *   call the set_up function, then the code defined inside the TEST_F macro,
 *   and then the tear_down function.
 *
 *   Under the hood, the TEST_F macro will create a new class that inherits from
 *   @ref class_name, declare and define the virtual run method, and register
 *   itself to the framework
 */
#define TEST_F(class_name, test_name)                                             \
    class class_name##test_name : public class_name                               \
    {                                                                             \
    public:                                                                       \
        void run() override;                                                      \
    };                                                                            \
    static int var##class_name##test_name =                                       \
        corgi::test::detail::register_fixture<class_name##test_name>(#class_name, \
                                                                     #test_name); \
    void class_name##test_name::run()

/*!
 * @brief      Replace the TEST macro with a function registered by the framework
 * @details    Ok so the trick is a little bit dirty, and the parameter should
 * actually not be a string for it to works. The idea is to create an useless variable
 * so I can call the register function. 
 */
#define TEST(group_name, function_name)                                                 \
    void       group_name##_##function_name();                                          \
    static int var##group_name##function_name = corgi::test::detail::register_function( \
        &group_name##_##function_name, #function_name, #group_name);                    \
    void group_name##_##function_name()

#define assert_that(value, expected)                                                \
    corgi::test::detail::assert_that_(value, expected, #value, #expected, __FILE__, \
                                      __LINE__)
}    // namespace test
}    // namespace corgi

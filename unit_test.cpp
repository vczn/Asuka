#include <iostream>
#include <typeinfo>

#include "src/util/any.hpp"
#include "src/util/block_queue.hpp"
#include "src/util/string_view.hpp"

using namespace Asuka;

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define UNIT_TEST_BASE(equality, expect, actual)                        \
    do                                                                  \
    {                                                                   \
        test_count++;                                                   \
        if (equality)                                                   \
            test_pass++;                                                \
        else                                                            \
        {                                                               \
            std::cerr << __FILE__ << ":" << __LINE__ << ":  except: "   \
                << expect << " actual: " << actual << std::endl;        \
            main_ret = 1;                                               \
        }                                                               \
    } while (0)

#define UNIT_TEST(expect, actual) UNIT_TEST_BASE((expect) == (actual), (expect), (actual))

void test_any()
{
    Any any1;
    UNIT_TEST(false, any1.has_value());
    UNIT_TEST(true, any1.type() == typeid(void));
    // any_cast<int>(any1);  // throw bad_any_cast
    
    Any any2{ 42 };
    UNIT_TEST(true, any2.has_value());
    UNIT_TEST(true, any2.type() == typeid(int));
    UNIT_TEST(42, any_cast<int>(any2));

    any1.swap(any2);
    UNIT_TEST(true, any1.has_value());
    UNIT_TEST(42, any_cast<int>(any1));
    UNIT_TEST(false, any2.has_value());

    any1.reset();
    UNIT_TEST(false, any1.has_value());
    
    const Any any3{ 42 };
    UNIT_TEST(42, any_cast<int>(any3));

    Any any4 = any3;
    UNIT_TEST(42, any_cast<int>(std::move(any4)));

    Any any5 = std::move(any4);
    UNIT_TEST(false, any4.has_value());
    UNIT_TEST(42, any_cast<int>(any5));
}

void test_block_queue()
{
    // TODO
}

void test_string_view()
{

}

int main()
{
    test_any();

    std::cout << test_pass << "/" << test_count
        << " (passed " << test_pass * 100.0 / test_count << "%)" << std::endl;
    return 0;
}
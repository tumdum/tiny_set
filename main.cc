#include <iostream>
#include "tiny.hpp"

struct Foo
{
    Foo() : a(0), b(0) {}
    Foo(uint16_t x, uint8_t y) : a(x), b(y) {}

    uint16_t a;
    uint8_t b;
};

bool operator == (const Foo& lhs, const Foo& rhs)
{
    return lhs.a == rhs.a;
}

bool operator < (const Foo& lhs, const Foo& rhs)
{
    return lhs.a < rhs.a;
}

static_assert(sizeof(std::set<Foo>) == 48, "");
static_assert(sizeof(std::unique_ptr<std::set<Foo>>) == sizeof(std::set<Foo>*), "");
static_assert(sizeof(tiny::set<Foo>)*2 == sizeof(std::set<Foo>), "");

std::string random_string(int modulo = 10)
{
    static const std::string prefix{"prefix                                                                                 "};
    return prefix + std::to_string(rand() % modulo);
}

void random_same_op(std::set<std::string>& a, tiny::set<std::string>& b)
{
    assert(a.size() == b.size());
    assert(a == b.to_std_set());
    switch (rand() % 3)
    {
        case 0:
            {
                const auto new_string = random_string();
                const auto aok = a.emplace(new_string).second; 
                const auto bok = b.emplace(new_string);
                assert(aok == bok);
            }
        case 1:
            {
                const auto needle = random_string();
                assert((a.find(needle) != a.end()) == b.contains(needle));
            }
        case 2:
            {
                const auto element = random_string();
                a.erase(element);
                b.erase(element);
            }
    }
    assert(a.size() == b.size());
    assert(a == b.to_std_set());
}

int main()
{
#if not defined(BENCHMARK_TINY) and not defined(BENCHMARK_STD)
    std::cerr << sizeof(tiny::array_set<Foo, 4>) << std::endl;
    std::cerr << "tiny size: " << tiny::set<Foo>::S << std::endl;
    std::cerr << sizeof(Foo) << std::endl;
    std::cerr << "tiny sizeof: " << sizeof(tiny::set<Foo>) << std::endl;

    tiny::array_set<Foo, 4> foos;
    assert(foos.emplace(uint16_t(1),uint8_t(2)));
    assert(!foos.emplace(uint16_t(1),uint8_t(2)));
    assert(foos.emplace(uint16_t(3),uint8_t(4)));
    assert(foos.emplace(uint16_t(5),uint8_t(6)));
    assert(foos.emplace(uint16_t(7),uint8_t(8)));
    assert(!foos.emplace(uint16_t(9),uint8_t(10)));

    assert(foos.contains(Foo{1,2}));
    assert(foos.contains(Foo{3,4}));
    assert(foos.contains(Foo{5,6}));
    assert(foos.contains(Foo{7,8}));
    assert(!foos.contains(Foo{9,10}));

    foos.erase(Foo{1, 2});

    assert(!foos.contains(Foo{1,2}));
    assert(foos.contains(Foo{3,4}));
    assert(foos.contains(Foo{5,6}));
    assert(foos.contains(Foo{7,8}));
    assert(!foos.contains(Foo{9,10}));

    foos.erase(Foo{7, 8});

    assert(!foos.contains(Foo{1,2}));
    assert(foos.contains(Foo{3,4}));
    assert(foos.contains(Foo{5,6}));
    assert(!foos.contains(Foo{7,8}));
    assert(!foos.contains(Foo{9,10}));

    assert(foos.emplace(uint16_t(9), uint8_t(10)));

    assert(!foos.contains(Foo{1,2}));
    assert(foos.contains(Foo{3,4}));
    assert(foos.contains(Foo{5,6}));
    assert(!foos.contains(Foo{7,8}));
    assert(foos.contains(Foo{9,10}));

    tiny::array_set<std::string, 4> strings;
    const std::string bar{"bar                                                                      "};
    strings.emplace("foo");
    strings.emplace(bar);
    strings.erase(bar);
    strings.emplace("baz");

    for (auto it = strings.begin(); it != strings.end(); ++it)
    {
        std::cerr << *it << std::endl;
    }

    const auto& sr = strings;
    for (const auto& s : sr)
    {
        std::cerr << s << std::endl;
    }

    tiny::set<Foo> tfoos;
    assert(0 == tfoos.size());
    assert(tfoos.emplace(uint16_t(1),uint8_t(2)));
    assert(1 == tfoos.size());
    assert(!tfoos.emplace(uint16_t(1),uint8_t(2)));
    assert(1 == tfoos.size());
    assert(tfoos.emplace(uint16_t(3),uint8_t(4)));
    assert(2 == tfoos.size());
    assert(tfoos.emplace(uint16_t(5),uint8_t(6)));
    assert(3 == tfoos.size());
    assert(tfoos.emplace(uint16_t(7),uint8_t(8)));
    assert(4 == tfoos.size());
    assert(tfoos.emplace(uint16_t(9),uint8_t(10)));
    assert(5 == tfoos.size());

    tiny::set<std::string> tstrings;
    assert(tstrings.emplace(bar));
    assert(!tstrings.emplace(bar));
    assert(tstrings.emplace("1"));
    assert(tstrings.emplace("2"));
    assert(tstrings.emplace("3"));
    assert(!tstrings.emplace("3"));

    assert(tstrings.contains(bar));
    assert(tstrings.contains("1"));
    assert(tstrings.contains("2"));
    assert(tstrings.contains("3"));
    assert(!tstrings.contains("4"));
    assert(4 == tstrings.size());

    tstrings.erase(bar);
    assert(3 == tstrings.size());
    tstrings.erase("1");
    tstrings.erase("2");
    tstrings.erase("3");
    tstrings.erase("4");

    assert(!tstrings.contains(bar));
    assert(!tstrings.contains("1"));
    assert(!tstrings.contains("2"));
    assert(!tstrings.contains("3"));
    assert(!tstrings.contains("4"));

    assert(tstrings.emplace(bar));
    assert(!tstrings.emplace(bar));
    assert(tstrings.emplace("1"));
    assert(tstrings.emplace("2"));
    assert(tstrings.emplace("3"));
    assert(!tstrings.emplace("3"));
    assert(tstrings.emplace("4"));
    assert(!tstrings.emplace("4"));

    assert(tstrings.contains(bar));
    assert(tstrings.contains("1"));
    assert(tstrings.contains("2"));
    assert(tstrings.contains("3"));
    assert(tstrings.contains("4"));

    tstrings.erase("3");
    assert(!tstrings.contains("3"));
    
    for (int a = 0; a != 1000; ++a)
    {
        for (int i = 0; i != 100; ++i)
        {
            std::set<std::string> a;
            tiny::set<std::string> b;
            random_same_op(a, b);
        }
    }

#else

    
#if defined(BENCHMARK_TINY)
    std::cout << "Using tiny::set" << std::endl;
    tiny::set<Foo> tfoos, tfoos2;
#elif defined(BENCHMARK_STD)
    std::cout << "Using std::set" << std::endl;
    std::set<Foo> tfoos, tfoos2;
#endif
    tfoos.emplace(uint16_t(1),uint8_t(2));
    tfoos.emplace(uint16_t(3),uint8_t(4));
    tfoos.emplace(uint16_t(5),uint8_t(6));
    tfoos.emplace(uint16_t(7),uint8_t(8));

    tfoos2.emplace(uint16_t(7),uint8_t(8));
    tfoos2.emplace(uint16_t(5),uint8_t(6));
    tfoos2.emplace(uint16_t(3),uint8_t(4));
    tfoos2.emplace(uint16_t(9),uint8_t(2));

    const Foo needle{uint16_t(BENCHMARK_WHICH), uint8_t(8)};

    volatile bool ret = false;

    for (uint64_t i = 0; i != BENCHMARK_MAX; ++i)
    {
#if defined(BENCHMARK_TINY)
ret =       tfoos.contains(needle);
ret =       tfoos2.contains(needle);
ret =       tfoos.contains(needle);
ret =       tfoos2.contains(needle);
ret =       tfoos.contains(needle);
ret =       tfoos2.contains(needle);
ret =       tfoos.contains(needle);
ret =       tfoos2.contains(needle);
ret =       tfoos.contains(needle);
ret =       tfoos2.contains(needle);
ret =       tfoos.contains(needle);
ret =       tfoos2.contains(needle);
ret =       tfoos.contains(needle);
ret =       tfoos2.contains(needle);
ret =       tfoos.contains(needle);
ret =       tfoos2.contains(needle);
ret =       tfoos.contains(needle);
ret =       tfoos2.contains(needle);
ret =       tfoos.contains(needle);
#elif defined(BENCHMARK_STD)
ret =       tfoos.find(needle) != tfoos.end();
ret =       tfoos2.find(needle) != tfoos2.end();
ret =       tfoos.find(needle) != tfoos.end();
ret =       tfoos2.find(needle) != tfoos2.end();
ret =       tfoos.find(needle) != tfoos.end();
ret =       tfoos2.find(needle) != tfoos2.end();
ret =       tfoos.find(needle) != tfoos.end();
ret =       tfoos2.find(needle) != tfoos2.end();
ret =       tfoos.find(needle) != tfoos.end();
ret =       tfoos2.find(needle) != tfoos2.end();
ret =       tfoos.find(needle) != tfoos.end();
ret =       tfoos2.find(needle) != tfoos2.end();
ret =       tfoos.find(needle) != tfoos.end();
ret =       tfoos2.find(needle) != tfoos2.end();
ret =       tfoos.find(needle) != tfoos.end();
ret =       tfoos2.find(needle) != tfoos2.end();
ret =       tfoos.find(needle) != tfoos.end();
ret =       tfoos2.find(needle) != tfoos2.end();
ret =       tfoos.find(needle) != tfoos.end();
#endif
    (void) ret;
    }

#endif
}

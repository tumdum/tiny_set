#include <iostream>
#include "tiny_set.hpp"

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
static_assert(sizeof(tiny_set<Foo>)*2 == sizeof(std::set<Foo>), "");

std::string random_string(int modulo = 10)
{
    static const std::string prefix{"prefix                                                                                 "};
    return prefix + std::to_string(rand() % modulo);
}

void random_same_op(std::set<std::string>& a, tiny_set<std::string>& b)
{
    assert(a.size() == b.size());
    switch (rand() % 3)
    {
        case 0:
            {
                const auto new_string = random_string();
                a.emplace(new_string); b.emplace(new_string);
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
}

int main()
{
    std::cerr << sizeof(array_set<Foo, 4>) << std::endl;
    std::cerr << "tiny size: " << tiny_set<Foo>::S << std::endl;
    std::cerr << sizeof(Foo) << std::endl;
    std::cerr << "tiny sizeof: " << sizeof(tiny_set<Foo>) << std::endl;

    array_set<Foo, 4> foos;
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

    array_set<std::string, 4> strings;
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

    tiny_set<Foo> tfoos;
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

    tiny_set<std::string> tstrings;
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
    
    for (int a = 0; a != 100; ++a)
    {
        for (int i = 0; i != 50; ++i)
        {
            std::set<std::string> a;
            tiny_set<std::string> b;
            random_same_op(a, b);
        }
    }
}

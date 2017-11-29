#include <set>
#include <memory>
#include <cassert>

namespace tiny
{

template <typename T, int S, typename TSize, typename... Args>
bool emplace(std::array<T, S>& data, TSize& size, Args&&... args)
{
    assert(S > size);
    new (&data[size]) T{std::forward<Args>(args)...};
    for (int i = 0; i != size; ++i)
    {
        if (data[i] == data[size])
        {
            data[size].~T();
            return false;
        }
    }
    size++;
    return true;
}

template <typename T, int S>
bool contains(const std::array<T, S>& data, uint8_t size, const T& needle)
{
    for (int i = 0; i != size; ++i)
    {
        if (data[i] == needle) { return true; }
    }
    return false;
}

template <typename T, int S, typename TSize>
void erase(std::array<T, S>& data, TSize& size, const T& element)
{
    for (int i = 0; i != size; ++i)
    {
        if (data[i] == element)
        {
            data[i].~T();
            std::swap(data[i], data[size-1]); // TODO: what if last element is removed?
            size--;
            return;
        }
    }
}

template <typename T, int S>
struct array_set
{
    using iterator = T*;
    using const_iterator = T const *;

    template <typename... Args>
    bool emplace(Args&&... args)
    {
        if (m_size == S) { return false; }
        return tiny::emplace<T, S, decltype(m_size), Args...>(m_data, m_size, std::forward<Args>(args)...);
    }

    bool contains(const T& needle) const
    {
        return tiny::contains<T, S>(m_data, m_size, needle);
    }

    void erase(const T& element)
    {
        tiny::erase<T, S, decltype(m_size)>(m_data, m_size, element);
    }

    iterator begin() { return &m_data[0]; }

    const_iterator begin() const { return &m_data[0]; }

    iterator end() { return &m_data[m_size]; }

    const_iterator end() const { return &m_data[m_size]; }

private:
    std::array<T, S> m_data;
    uint8_t m_size = 0;
};

template <typename T>
struct set
{
    static constexpr const int S = 4; // (sizeof(std::set<T>)/sizeof(T))/2;
    set()
        : m_data{std::array<T, S>{}}
        , m_size(0)
    {
    }

    ~set()
    {
        if (isTiny())
        {
            for (int i = 0; i != m_size; ++i)
            {
                m_data.tiny[i].~T();
            }
            return;
        }
        m_data.full.reset();
    }

    size_t size() const
    {
        if (isTiny()) { return m_size; }
        return m_data.full->size();
    }

    template <typename... Args>
    bool emplace(Args&&... args)
    {
        if (isTiny())
        {
            if (m_size == S)
            {
                T tmp{std::forward<Args>(args)...};
                if (contains(tmp)) { return false; }

                auto set = std::unique_ptr<std::set<T>>(new std::set<T>);
                set->insert(m_data.tiny.begin(), m_data.tiny.end());
                const auto ret = set->emplace(std::move(tmp)).second;

                m_data.tiny.std::array<T, S>::~array();
                new (&m_data.full) std::unique_ptr<std::set<T>>();
                m_data.full = std::move(set);

                m_size = -1;
                return ret;
            }
            return tiny::emplace<T, S, decltype(m_size), Args...>(m_data.tiny, m_size, std::forward<Args>(args)...);
        }
        return m_data.full->emplace(std::forward<Args>(args)...).second;
    }

    bool contains(const T& needle) const
    {
        if (isTiny()) { return tiny::contains<T, S>(m_data.tiny, m_size, needle); }
        return m_data.full->find(needle) != m_data.full->end();
    }

    void erase(const T& element)
    {
        if (isTiny())
        {
            tiny::erase<T, S, decltype(m_size)>(m_data.tiny, m_size, element);
            return;
        }
        m_data.full->erase(element);
    }

    std::set<T> to_std_set() const
    {
        if (isTiny())
        {
            std::set<T> ret;
            for (int i = 0; i != m_size; ++i)
            {
                ret.insert(m_data.tiny[i]);
            }
            return ret;
        }
        return *m_data.full;
    }
private:
    bool isTiny() const
    {
        return m_size != -1;
    }

    union Data 
    {
        std::array<T, S> tiny;
        std::unique_ptr<std::set<T>> full;
        ~Data() {}
    };
    Data m_data;
    int8_t m_size; // >= 0 when tiny, -1 when full 
};

static_assert(sizeof(set<uint32_t>) < sizeof(std::set<uint32_t>), "");

}

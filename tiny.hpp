#include <set>
#include <memory>
#include <cassert>

#include <boost/iterator/iterator_facade.hpp>

namespace tiny
{

template <typename T, int S, typename TSize, typename... Args>
bool emplace(T (&data)[S], TSize& size, Args&&... args)
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
bool contains(const T (&data)[S], uint8_t size, const T& needle)
{
    for (int i = 0; i != size; ++i)
    {
        if (data[i] == needle) { return true; }
    }
    return false;
}

template <typename T, int S, typename TSize>
void erase(T (&data)[S], TSize& size, const T& element)
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

template <
    typename T,
    typename C = std::less<T>,
    typename A = std::allocator<T>>
struct const_iterator : boost::iterator_facade<
                  const_iterator<T,C,A>, const T, boost::forward_traversal_tag>
{
    using setIt = typename std::set<T,C,A>::const_iterator;

    const_iterator(const T* it) : pointer(it) {}
    const_iterator(setIt it) : iterator(it) {}

    template <typename U>
    bool equal(const const_iterator<U,C,A>& other) const
    {
        if (pointer != nullptr)
        {
            assert(other.pointer != nullptr);
            return pointer == other.pointer;
        }
        assert(other.pointer == nullptr);
        return iterator == other.iterator;
    }

    void increment()
    {
        if (pointer != nullptr)
        {
            pointer++;
        }
        else
        {
            iterator++;
        }
    }

    const T& dereference() const
    {
        if (pointer != nullptr)
        {
            return *pointer;
        }
        return *iterator;
    }

private:
    const T* pointer = nullptr;
    setIt iterator;
};

template <
    typename T,
    typename C = std::less<T>,
    typename A = std::allocator<T>,
    int S = 4>
struct set
{
    set()
        : m_data{T{}}
        , m_size(0)
    {
    }

    ~set()
    {
        if (is_tiny())
        {
            for (int i = 0; i != m_size; ++i)
            {
                m_data.tiny[i].~T();
            }
        }
        else
        {
            m_data.full.reset();
        }
    }

    set(const set<T,C,A>& other)
        : m_data{T{}}
        , m_size{0}
    {
        *this = other;
    }

    set(set<T,C,A>&& other)
        : m_data{T{}}
        , m_size{0}
    {
        *this = std::move(other);
    }

    set<T,C,A>& operator=(set<T,C,A>&& other)
    {
        m_size = other.m_size;
        if (other.is_tiny())
        {
            for (int i = 0; i != other.m_size; ++i)
            {
                m_data.tiny[i] = std::move(other.m_data.tiny[i]);
            }
        }
        else
        {
            new (&m_data.full) std::unique_ptr<std::set<T>>(
                std::move(other.m_data.full));
        }
        return *this;
    }

    set<T,C,A>& operator=(const set<T,C,A>& other)
    {
        if (other.is_tiny())
        {
            if (!is_tiny())
            {
                m_data.full.reset();
            }
            for (int i = 0; i != other.m_size; ++i)
            {
                new (&m_data.tiny[i]) std::string(other.m_data.tiny[i]);
            }
        }
        else
        {
            if (is_tiny())
            {
                for (int i = 0; i != m_size; ++i)
                {
                    m_data.tiny[i].~T();
                }
            }
            new (&m_data.full) std::unique_ptr<std::set<T>>(
                new std::set<T>(*other.m_data.full.get()));
        }
        m_size = other.m_size;
        return *this;
    }

    const_iterator<T,C,A> begin() const
    {
        if (is_tiny())
        {
            return const_iterator<T,C,A>(&m_data.tiny[0]);
        }
        return const_iterator<T,C,A>(m_data.full->cbegin());
    }

    const_iterator<T,C,A> end() const
    {
        if (is_tiny())
        {
            return const_iterator<T,C,A>(&m_data.tiny[m_size]);
        }
        return const_iterator<T,C,A>(m_data.full->cend());
    }

    size_t size() const
    {
        if (is_tiny()) { return m_size; }
        return m_data.full->size();
    }

    template <typename... Args>
    bool emplace(Args&&... args)
    {
        if (is_tiny())
        {
            if (m_size == S)
            {
                T tmp{std::forward<Args>(args)...};
                if (contains(tmp)) { return false; }

                auto set = std::unique_ptr<std::set<T>>(new std::set<T>);
                set->insert(&m_data.tiny[0], &m_data.tiny[S]);
                const auto ret = set->emplace(std::move(tmp)).second;

                for (int i = 0; i != S; ++i)
                {
                    m_data.tiny[i].~T();
                }
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
        if (is_tiny()) { return tiny::contains<T, S>(m_data.tiny, m_size, needle); }
        return m_data.full->find(needle) != m_data.full->end();
    }

    void erase(const T& element)
    {
        if (is_tiny())
        {
            tiny::erase<T, S, decltype(m_size)>(m_data.tiny, m_size, element);
            return;
        }
        m_data.full->erase(element);
    }

    void clear()
    {
        if (is_tiny())
        {
            for (int i = 0; i != m_size; ++i)
            {
                m_data.tiny[i].~T();
            }
            m_size = 0;
            return;
        }
        m_data.full->clear();
    }

    std::set<T,C,A> to_std_set() const
    {
        if (is_tiny())
        {
            std::set<T,C,A> ret{begin(), end()};
            return ret;
        }
        return *m_data.full;
    }

    bool is_tiny() const
    {
        return m_size != -1;
    }
private:

    union Data 
    {
        T tiny[S];
        std::unique_ptr<std::set<T,C,A>> full;
        ~Data() {}
    };
    Data m_data;
    int8_t m_size; // >= 0 when tiny, -1 when full 
};

static_assert(sizeof(set<uint32_t,std::less<uint32_t>, std::allocator<uint32_t>, 4>) < sizeof(std::set<uint32_t>), "");

}

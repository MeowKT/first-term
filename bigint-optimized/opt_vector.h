#pragma once
#include <cstddef>
#include <cstdint>
#include <algorithm>

class opt_vector {
public:
    opt_vector(): _size(0) {}

    opt_vector(opt_vector const& other): _size(other._size)
    {
        if (other.is_small())
        {
            std::copy_n(other.val, other.size(), val);
        } else
        {
            other.data[0]++;
            data = other.data;
        }
    }

    opt_vector& operator=(opt_vector const& other)
    {
        if (&other != this)
        {
            opt_vector safe(other);
            swap(safe);
        }
        return *this;
    }

    ~opt_vector()
    {
        if (!is_small())
        {
            data[0]--;
            if (data[0] == 0)
            {
                operator delete(data);
            }
        }
    }

    size_t size() const
    {
        return _size & (UINT32_MAX - BIG_FLAG);
    }

    bool empty() const
    {
        return size() == 0;
    }

    void push_back(uint32_t x)
    {
        become_unique();
        if (size() == SMALL_SZ)
        {
            become_big();
        }
        if (is_small())
        {
            val[_size] = x;
        }
        else
        {
            if (size() == capacity())
            {
                expand(capacity() * 2);
            }
            data[size() + 2] = x;
        }
        _size++;
    }

    void pop_back()
    {
        become_unique();
        _size--;
    }

    void resize(size_t n)
    {
        become_unique();
        if (n > SMALL_SZ)
        {
            become_big();
        }
        if (is_small())
        {
            while (size() < n)
            {
                val[_size++] = 0;
            }
            _size = n;
        }
        else
        {
            if (n > capacity())
            {
                expand(n);
            }
            while (size() < n)
            {
                data[size() + 2] = 0;
                _size++;
            }
            _size = n | BIG_FLAG;
        }
    }

    const uint32_t* begin() const
    {
        return is_small() ? val : (data + 2);
    }

    uint32_t* begin()
    {
        become_unique();
        return is_small() ? val : (data + 2);
    }

    const uint32_t* end() const
    {
        return (is_small() ? val : (data + 2)) + size();
    }

    uint32_t* end()
    {
        become_unique();
        return (is_small() ? val : (data + 2)) + size();
    }

    uint32_t const& operator[](size_t i) const
    {
        return is_small() ? val[i] : data[i + 2];
    }

    uint32_t& operator[](size_t i)
    {
        become_unique();
        return is_small() ? val[i] : data[i + 2];
    }

    uint32_t const& back() const
    {
        return is_small() ? val[size() - 1] : data[size() + 1];
    }

    uint32_t& back()
    {
        become_unique();
        return is_small() ? val[size() - 1] : data[size() + 1];
    }
private:
    static constexpr size_t SMALL_SZ = 2;
    static constexpr uint32_t BIG_FLAG = (static_cast<uint32_t>(1) << 31);
    // last bit of _size is an "is big?" flag
    size_t _size;
    union
    {
        uint32_t val[SMALL_SZ];
        // data[0] is a reference counter in COW
        // data[1] is capacity
        uint32_t* data;
    };

    size_t capacity() const
    {
        return is_small() ? SMALL_SZ : data[1];
    }

    void swap(opt_vector& other)
    {
        if (is_small())
        {
            size_t buf_sz = size();
            uint32_t buf[SMALL_SZ];
            std::copy_n(val, buf_sz, buf);
            if (other.is_small())
            {
                std::copy_n(other.val, other.size(), val);
            }
            else
            {
                data = other.data;
            }
            std::copy_n(buf, buf_sz, other.val);
        }
        else
        {
            if (other.is_small()) {
                uint32_t* buf_data = data;
                std::copy_n(other.val, other.size(), val);
                other.data = buf_data;
            }
            else
            {
                std::swap(data, other.data);
            }
        }
        std::swap(_size, other._size);
    }

    bool is_small() const
    {
        return (_size & BIG_FLAG) == 0;
    }

    static uint32_t* get_big_data(uint32_t* old_data, size_t old_size, size_t capacity)
    {
        auto* new_data = static_cast<uint32_t*>(operator new((capacity + 2) * sizeof(uint32_t)));
        new_data[0] = 1;
        new_data[1] = capacity;
        std::copy_n(old_data, old_size, new_data + 2);
        return new_data;
    }

    void expand(size_t new_capacity)
    {
        uint32_t* new_data = get_big_data(begin(), size(), new_capacity);
        operator delete(data);
        data = new_data;
    }

    void become_big()
    {
        if (is_small())
        {
            _size |= BIG_FLAG;
            uint32_t buf[SMALL_SZ];
            std::copy_n(val, size(), buf);
            data = get_big_data(buf, size(), size());
        }
    }

    void become_unique()
    {
        if (!is_small() && data[0] > 1)
        {
            data[0]--;
            data = get_big_data(data + 2, size(), capacity());
        }
    }
};

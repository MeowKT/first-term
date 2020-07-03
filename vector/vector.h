#pragma once

#include <cassert>
#include <algorithm>

template <typename T>
struct vector
{
    typedef T* iterator;
    typedef T const* const_iterator;

    vector() : data_(nullptr), size_(0), capacity_(0) {}

    vector(vector const& other) : vector(other.data_, other.size_, other.size_) {}

    vector& operator=(vector const& other)
    {
        if (this != &other)
        {
            vector safe(other);
            swap(safe);
        }
        return *this;
    }

    ~vector()
    {
        clear();
        operator delete(data_);
    }

    T& operator[](size_t i)
    {
        assert(i >= 0 && i < size_);
        return data_[i];
    }

    T const& operator[](size_t i) const
    {
        assert(i >= 0 && i < size_);
        return data_[i];
    }

    T* data()
    {
        return data_;
    }

    T const* data() const
    {
        return data_;
    }

    size_t size() const
    {
        return size_;
    }

    T& front()
    {
        assert(size_ > 0);
        return *begin();
    }

    T const& front() const
    {
        assert(size_ > 0);
        return *begin();
    }

    T& back()
    {
        assert(size_ > 0);
        return *(end() - 1);
    }

    T const& back() const
    {
        assert(size_ > 0);
        return *(end() - 1);
    }

    void push_back(T const& x)
    {
        if (size_ == capacity_)
        {
            T safe = x;
            increase_capacity();
            new (data_ + size_) T(safe);
        }
        else
        {
            new (data_ + size_) T(x);
        }
        size_++;
    }

    void pop_back()
    {
        assert(size_ > 0);
        size_--;
        data_[size_].~T();
    }

    bool empty() const
    {
        return size_ == 0;
    }

    size_t capacity() const
    {
        return capacity_;
    }

    void reserve(size_t new_capacity)
    {
        if (new_capacity <= capacity_)
        {
            return;
        }
        vector safe(data_, size_, new_capacity);
        swap(safe);
    }

    void shrink_to_fit()
    {
        if (size_ == capacity_)
        {
            return;
        }
        vector safe(data_, size_, size_);
        swap(safe);
    }

    void clear()
    {
        while (size_ > 0)
        {
            pop_back();
        }
    }

    void swap(vector& other)
    {
        std::swap(other.data_, data_);
        std::swap(other.capacity_, capacity_);
        std::swap(other.size_, size_);
    }

    iterator begin()
    {
        return data_;
    }

    iterator end()
    {
        return data_ + size_;
    }

    const_iterator begin() const
    {
        return data_;
    }

    const_iterator end() const
    {
        return data_ + size_;
    }

    iterator insert(const_iterator it, T const& x)
    {
        ptrdiff_t pos = it - begin();
        push_back(x);
        std::rotate(begin() + pos, end() - 1, end());
        return begin() + pos;
    }

    iterator erase(const_iterator it)
    {
        return erase(it, it + 1);
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        ptrdiff_t l = first - begin();
        ptrdiff_t len = last - first;
        if (len != 0)
        {
            std::rotate(begin() + l, begin() + l + len, end());
            for (size_t i = 0; i < len; i++)
            {
                pop_back();
            }
        }
        return begin() + l;
    }

private:

    vector( T const* data, size_t size, size_t capacity) : size_(0), capacity_(capacity)
    {
        data_ = capacity ? static_cast<T*>(operator new(capacity * sizeof(T))) : nullptr;

        try
        {
            for (; size_ < size; size_++)
            {
                new(data_ + size_) T(data[size_]);
            }
        }
        catch (...)
        {
            clear();
            operator delete(data_);
            throw;
        }
    }

    void increase_capacity()
    {
        size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
        vector<T> other(data_, size_, new_capacity);
        swap(other);
    }

private:
    T* data_;
    size_t size_;
    size_t capacity_;
};

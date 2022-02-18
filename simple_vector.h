#pragma once

#include "array_ptr.h"
#include <cassert>
#include <stdexcept>
#include <initializer_list>
#include <utility>

struct ReserveProxyObj
{
    ReserveProxyObj() noexcept = default;

    ReserveProxyObj(size_t capacity_to_reserve)
        : capacity_to_reserve_(capacity_to_reserve)
    {
    }
    size_t capacity_to_reserve_ = 0;
};

template <typename Type>
class SimpleVector
{
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size)
        : size_(size), capacity_(size), items_(size)
    {
        std::move(begin(), end(), Type());
    }

    SimpleVector(size_t size, const Type& value)
        : size_(size), capacity_(size), items_(size)
    {
        std::move(begin(), end(), value);
    }

    SimpleVector(std::initializer_list<Type> init)
        : size_(init.size()), capacity_(init.size()), items_(init.size())
    {
        std::copy(init.begin(), init.end(), begin());
    }

    SimpleVector(const SimpleVector& other)
        : size_(other.size_), capacity_(other.capacity_), items_(other.size_)
    {
        std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector(SimpleVector&& other)
        : size_(std::exchange(other.size_, 0)), capacity_(std::exchange(other.capacity_, 0)), items_(std::move(other.items_))
    {
    }

    SimpleVector(ReserveProxyObj reserve_proxy_obj)
        : size_(0), capacity_(reserve_proxy_obj.capacity_to_reserve_), items_(size_)
    {
    }

    SimpleVector& operator=(const SimpleVector& rhs)
    {
        if (this != &rhs)
        {
            SimpleVector tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs)
    {
        if (this != &rhs)
        {
            SimpleVector tmp = std::move(rhs);
            swap(tmp);
        }
        return *this;
    }

    size_t GetSize() const noexcept
    {
        return size_;
    }

    size_t GetCapacity() const noexcept
    {
        return capacity_;
    }

    bool IsEmpty() const noexcept
    {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept
    {
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept
    {
        return items_[index];
    }

    Type& At(size_t index)
    {
        if (index >= size_)
        {
            throw std::out_of_range("Invalid index");
        }
        else
        {
            return items_[index];
        }
    }

    const Type& At(size_t index) const
    {
        if (index >= size_)
        {
            throw std::out_of_range("Invalid index");
        }
        else
        {
            return items_[index];
        }
    }

    void Clear() noexcept
    {
        size_ = 0;
    }

    void Resize(size_t new_size)
    {
        if (new_size < size_)
        {
            size_ = new_size;
        }
        else if (new_size <= capacity_)
        {
            std::move(end(), &items_[capacity_], Type());
            size_ = new_size;
        }
        else
        {
            Reserve(std::max(new_size, 2 * capacity_));
            std::move(&items_[size_], &items_[new_size], Type());
            size_ = new_size;
        }
    }

    Iterator begin() noexcept
    {
        return items_.Get();
    }

    Iterator end() noexcept
    {
        return &items_[size_];
    }

    ConstIterator begin() const noexcept
    {
        return items_.Get();
    }

    ConstIterator end() const noexcept
    {
        return &items_[size_];
    }

    ConstIterator cbegin() const noexcept
    {
        return items_.Get();
    }

    ConstIterator cend() const noexcept
    {
        return &items_[size_];
    }

    void PushBack(Type&& item)
    {
        if (size_ < capacity_)
        {
            items_[size_] = std::move(item);
        }
        else
        {
            Reserve(std::max(1ul, 2 * capacity_));
            items_[size_] = std::move(item);
        }
        ++size_;
    }

    void PushBack(const Type& item)
    {
        if (size_ < capacity_)
        {
            items_[size_] = item;
        }
        else
        {
            Reserve(std::max(1ul, 2 * capacity_));
            items_[size_] = item;
        }
        ++size_;
    }

    Iterator Insert(ConstIterator pos, Type&& value)
    {
        size_t number_pos = VectorMoveRigth(pos);
        items_[number_pos] = std::move(value);
        return &items_[number_pos];
    }

    Iterator Insert(ConstIterator pos, const Type& value)
    {
        size_t number_pos = VectorMoveRigth(pos);
        items_[number_pos] = value;
        return &items_[number_pos];
    }

    void PopBack() noexcept
    {
        assert(!IsEmpty());
        if (!IsEmpty())
        {
            --size_;
        }
    }

    Iterator Erase(ConstIterator pos)
    {
        assert(pos != end());
        Iterator it = &items_[std::distance<ConstIterator>(cbegin(), pos)];
        std::move(it + 1, end(), it);
        --size_;
        return it;
    }

    void swap(SimpleVector& other) noexcept
    {
        std::swap(this->size_, other.size_);
        std::swap(this->capacity_, other.capacity_);
        items_.swap(other.items_);
    }

    void Reserve(size_t new_capacity)
    {
        if (new_capacity >= capacity_)
        {
            ArrayPtr<Type> new_items(new_capacity);
            std::move(begin(), end(), new_items.Get());
            items_.swap(new_items);
            capacity_ = new_capacity;
        }
    }

private:
    size_t size_ = 0;
    size_t capacity_ = 0;
    ArrayPtr<Type> items_;

    size_t VectorMoveRigth(ConstIterator pos)
    {
        size_t number_pos = std::distance<ConstIterator>(cbegin(), pos);
        if (size_ < capacity_)
        {
            if (pos == end())
            {
                assert(pos == end());
            }
            std::move_backward(&items_[number_pos], end(), &items_[size_ + 1]);
        }
        else
        {
            if (capacity_ == 0)
            {
                Reserve(1);
            }
            else
            {
                Reserve(2 * capacity_);
                std::move_backward(&items_[number_pos], end(), &items_[size_ + 1]);
            }
        }
        ++size_;
        return number_pos;
    }
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return !operator==(lhs, rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return !operator<(rhs, lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return operator<(rhs, lhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs)
{
    return !operator>(rhs, lhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve)
{
    return ReserveProxyObj(capacity_to_reserve);
}

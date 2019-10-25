#pragma once

#include <clean-core/always_false.hh>
#include <clean-core/assert.hh>
#include <clean-core/fwd.hh>
#include <clean-core/typedefs.hh>

#include <utility>

namespace cc
{
/**
 * - single-owner heap-allocated object
 * - move-only type
 * - supports polymorphic behavior
 *
 * changes to std::unique_ptr<T>:
 * - no custom deleter
 * - no allocators
 * - no operator<
 * - no operator bool
 * - no T[]
 */
template <class T>
struct poly_unique_ptr
{
    poly_unique_ptr() = default;

    poly_unique_ptr(poly_unique_ptr const&) = delete;
    poly_unique_ptr& operator=(poly_unique_ptr const&) = delete;

    poly_unique_ptr(poly_unique_ptr&& rhs) noexcept
    {
        _ptr = rhs._ptr;
        rhs._ptr = nullptr;
    }
    poly_unique_ptr& operator=(poly_unique_ptr&& rhs) noexcept
    {
        if (this != &rhs)
        {
            delete _ptr;
            _ptr = rhs._ptr;
            rhs._ptr = nullptr;
        }
        return *this;
    }

    ~poly_unique_ptr()
    {
        static_assert(sizeof(T) > 0, "cannot delete incomplete class");
        delete _ptr;
    }

    void reset(T* p = nullptr)
    {
        CC_CONTRACT(p == nullptr || p != _ptr); // no self-reset
        delete _ptr;
        _ptr = p;
    }

    [[nodiscard]] T* get() const { return _ptr; }

    T* release()
    {
        auto p = _ptr;
        _ptr = nullptr;
        return p;
    }

    [[nodiscard]] T* operator->() const
    {
        CC_ASSERT_NOT_NULL(_ptr);
        return _ptr;
    }
    [[nodiscard]] T& operator*() const
    {
        CC_ASSERT_NOT_NULL(_ptr);
        return *_ptr;
    }

    [[nodiscard]] bool operator==(poly_unique_ptr const& rhs) const { return _ptr == rhs._ptr; }
    [[nodiscard]] bool operator!=(poly_unique_ptr const& rhs) const { return _ptr != rhs._ptr; }
    [[nodiscard]] bool operator==(T const* rhs) const { return _ptr == rhs; }
    [[nodiscard]] bool operator!=(T const* rhs) const { return _ptr != rhs; }

private:
    T* _ptr = nullptr;
};

template <class T>
struct poly_unique_ptr<T[]>
{
    static_assert(always_false<T>, "poly_unique_ptr does not support arrays, use cc::vector or cc::array instead");
};

template <class T>
[[nodiscard]] bool operator==(T const* lhs, poly_unique_ptr<T> const& rhs)
{
    return lhs == rhs.get();
}
template <class T>
[[nodiscard]] bool operator==(nullptr_t, poly_unique_ptr<T> const& rhs)
{
    return rhs.get() == nullptr;
}

template <typename T, typename... Args>
[[nodiscard]] poly_unique_ptr<T> make_unique(Args&&... args)
{
    poly_unique_ptr<T> p;
    p.reset(new T(std::forward<Args>(args)...));
    return p;
}

template <class T>
struct hash<poly_unique_ptr<T>>
{
    [[nodiscard]] hash_t operator()(poly_unique_ptr<T> const& v) const noexcept { return hash_t(v.get()); }
};

template <class T>
struct less<poly_unique_ptr<T>>
{
    [[nodiscard]] bool operator()(poly_unique_ptr<T> const& a, poly_unique_ptr<T> const& b) const noexcept { return a.get() < b.get(); }
};
}
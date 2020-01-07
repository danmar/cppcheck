#ifndef GUARD_VALUEPTR_HPP
#define GUARD_VALUEPTR_HPP

#include "config.h"
#include <memory>
#include <functional>

template <class T>
class CPPCHECKLIB ValuePtr {
    template<class U>
    struct cloner {
        static T* apply(T* x) {
            return new U(*x);
        }
    };

    public:
    using pointer = T*;
    using element_type = T;
    using cloner_type = decltype(&cloner<T>::apply);


    ValuePtr()
    : ptr(nullptr), clone()
    {}

    template<class U>
    ValuePtr(const U &value)
    : ptr(ValuePtr<U>::cloner(&value)), clone(&cloner<U>::apply) 
    {}

    ValuePtr(const ValuePtr &rhs) : ptr(nullptr), clone(rhs.clone) {
        if (rhs) {
            ptr.reset(clone(rhs.get()));
        }
    }
    ValuePtr(ValuePtr && rhs)
    : ptr(std::move(rhs.ptr)), clone(std::move(rhs.clone))
    {}

    pointer release() {
        return ptr.release();
    }

    T *get() noexcept { return ptr.get(); }
    const T *get() const noexcept { return ptr.get(); }

    T &operator*() { return *get(); }
    const T &operator*() const { return *get(); }

    T *operator->() noexcept { return get(); }
    const T *operator->() const noexcept { return get(); }

    void swap(ValuePtr& rhs) {
        using std::swap;
        swap(ptr, rhs.ptr);
        swap(clone, rhs.clone);
    }

    ValuePtr<T> &operator=(ValuePtr rhs) {
        swap(rhs);
        return *this;
    }

    operator bool() const noexcept { return !!ptr; }
    ~ValuePtr() {}
    private:

    std::shared_ptr<T> ptr;
    cloner_type clone;
};

#endif

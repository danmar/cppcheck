/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2019 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GUARD_VALUEPTR_HPP
#define GUARD_VALUEPTR_HPP

#include "config.h"
#include <functional>
#include <memory>

template <class T>
class CPPCHECKLIB ValuePtr {
    template <class U>
    struct cloner {
        static T* apply(const T* x) { return new U(*static_cast<const U*>(x)); }
    };

  public:
    using pointer = T*;
    using element_type = T;
    using cloner_type = decltype(&cloner<T>::apply);

    ValuePtr() : ptr(nullptr), clone() {}

    template <class U>
    ValuePtr(const U& value) : ptr(cloner<U>::apply(&value)), clone(&cloner<U>::apply)
    {}

    ValuePtr(const ValuePtr& rhs) : ptr(nullptr), clone(rhs.clone)
    {
        if (rhs) {
            ptr.reset(clone(rhs.get()));
        }
    }
    ValuePtr(ValuePtr&& rhs) : ptr(std::move(rhs.ptr)), clone(std::move(rhs.clone)) {}

    pointer release() { return ptr.release(); }

    T* get() noexcept { return ptr.get(); }
    const T* get() const noexcept { return ptr.get(); }

    T& operator*() { return *get(); }
    const T& operator*() const { return *get(); }

    T* operator->() noexcept { return get(); }
    const T* operator->() const noexcept { return get(); }

    void swap(ValuePtr& rhs)
    {
        using std::swap;
        swap(ptr, rhs.ptr);
        swap(clone, rhs.clone);
    }

    ValuePtr<T>& operator=(ValuePtr rhs)
    {
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

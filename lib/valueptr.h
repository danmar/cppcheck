/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

//---------------------------------------------------------------------------
#ifndef valueptrH
#define valueptrH
//---------------------------------------------------------------------------

#include "config.h"

#include <memory>

template<class T>
class CPPCHECKLIB ValuePtr {
    template<class U>
    struct cloner {
        static T* apply(const T* x) {
            return new U(*static_cast<const U*>(x));
        }
    };

public:
    using pointer = T*;
    using element_type = T;
    using cloner_type = decltype(&cloner<T>::apply);

    ValuePtr() : mPtr(nullptr), mClone() {}

    template<class U>
    // cppcheck-suppress noExplicitConstructor
    // NOLINTNEXTLINE(google-explicit-constructor)
    ValuePtr(const U& value) : mPtr(cloner<U>::apply(&value)), mClone(&cloner<U>::apply)
    {}

    ValuePtr(const ValuePtr& rhs) : mPtr(nullptr), mClone(rhs.mClone) {
        if (rhs) {
            mPtr.reset(mClone(rhs.get()));
        }
    }
    ValuePtr(ValuePtr&& rhs) NOEXCEPT : mPtr(std::move(rhs.mPtr)), mClone(std::move(rhs.mClone)) {}

    /**
     * Releases the shared_ptr's ownership of the managed object using the .reset() function
     */
    void release() {
        mPtr.reset();
    }

    T* get() NOEXCEPT {
        return mPtr.get();
    }
    const T* get() const NOEXCEPT {
        return mPtr.get();
    }

    T& operator*() {
        return *get();
    }
    const T& operator*() const {
        return *get();
    }

    T* operator->() NOEXCEPT {
        return get();
    }
    const T* operator->() const NOEXCEPT {
        return get();
    }

    void swap(ValuePtr& rhs) {
        using std::swap;
        swap(mPtr, rhs.mPtr);
        swap(mClone, rhs.mClone);
    }

    ValuePtr<T>& operator=(ValuePtr rhs) {
        swap(rhs);
        return *this;
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    operator bool() const NOEXCEPT {
        return !!mPtr;
    }

private:
    std::shared_ptr<T> mPtr;
    cloner_type mClone;
};

#endif

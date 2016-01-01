/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
#ifndef utilsH
#define utilsH
//---------------------------------------------------------------------------

#include <algorithm>

/*! Helper class to aid in the initializing global const data */
template < typename Cont >
class make_container {
public:
    typedef make_container< Cont > my_type;
    typedef typename Cont::value_type T;

    my_type& operator<< (const T& val) {
        data_.insert(data_.end(), val);
        return *this;
    }
    my_type& operator<< (const Cont& other_container) {
        for (typename Cont::const_iterator it=other_container.begin(); it!=other_container.end(); ++it) {
            data_.insert(data_.end(), *it);
        }
        return *this;
    }
    my_type& operator<< (T&& val) {
        data_.insert(data_.end(), val);
        return *this;
    }
    my_type& operator<< (const char* val) {
        data_.insert(data_.end(), val);
        return *this;
    }
    operator Cont() const {
        return data_;
    }
private:
    Cont data_;
};

#endif

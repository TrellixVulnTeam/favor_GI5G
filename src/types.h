/*
Copyright (C) 2015  Joshua Tanner (mindful.jt@gmail.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#ifndef favor_types_include
#define favor_types_include

namespace favor {
    namespace worker{
        class AccountManager;
    }
    template <typename T> class DataLock;

    typedef std::string string;
    typedef worker::AccountManager AccountManager;
    template<typename T> using shared_ptr = std::shared_ptr<T>;
    template<typename T> using vector = std::vector<T>;
    template<typename T> using list = std::list<T>;
}

#endif
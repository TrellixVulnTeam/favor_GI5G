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



#ifndef favor_address_include
#define favor_address_include

#include "favor.h"
#include "reader.h"
#include "logger.h"

namespace favor {
    class Address {
    public:
        const string addr;
        const int count;
        const long contactId;
        const MessageType type;

        Address(string addr, int c, int contact, MessageType t) : addr(addr), count(c), contactId(contact), type(t){}

        bool belongsToContact(){
            return contactId > -1;
        }

        bool operator==(const Address& other) const{
            //logger::info(as_string(*this)+" compared to "+as_string(other));
            return (type == other.type && contactId == other.contactId &&
                count == other.count && addr == other.addr);
        }

        bool operator!=(const Address& other) const{
            return !(*this==other);
        }

//        //TODO: This won't compile for reasons unclear, but makes sense as a method here
//        bool inDatabase(){
//            return reader::addressExists(addr, type);
//        }
    };
}

#endif
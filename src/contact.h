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



#ifndef favor_contact_include
#define favor_contact_include

#include "favor.h"
#include "address.h"

namespace favor {
    class Contact {
    public:
        const long id;
        const string displayName;

        Contact(long ident, string name);
        Contact(long ident, string name, const Address& addr);
        Contact(long ident, string name, const vector <Address> &addrs);

        bool operator==(const Contact& rhs) const;

        const vector<Address>& getAddresses() const;

        bool hasType(MessageType type) const ;
        MessageTypeFlag typeFlags() const; //Largely exists just for testing purposes



    private:
        vector<Address> addresses;
        MessageTypeFlag types;

    };
}

#endif
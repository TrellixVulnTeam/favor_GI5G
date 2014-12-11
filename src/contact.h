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
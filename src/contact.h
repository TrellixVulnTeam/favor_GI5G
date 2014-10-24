#ifndef favor_contact_include
#define favor_contact_include

#include "favor.h"
#include "address.h"

namespace favor {
    class Contact {
    public:
        const long id;
        const string displayName;
        const MessageType type;

        Contact(long ident, string name, MessageType t);
        Contact(long ident, string name, MessageType t, const Address& addr);
        Contact(long ident, string name, MessageType t, const vector <Address> &addrs);

        void addAddress(const Address& addr);
        const vector<Address>& getAddresses() const;


    private:
        vector<Address> addresses;

    };
}

#endif
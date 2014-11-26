#ifndef favor_contact_include
#define favor_contact_include

#include "favor.h"
#include "address.h"

namespace favor {
    class Contact {
    public:
        const long id;
        const string displayName;

        Contact(long ident, string name, MessageTypeFlag ts);
        Contact(long ident, string name, MessageTypeFlag ts, const Address& addr);
        Contact(long ident, string name, MessageTypeFlag ts, const vector <Address> &addrs);

        bool operator==(const Contact& rhs) const{
            return id == rhs.id && displayName == rhs.displayName && types == rhs.types;
        }

        void addAddress(const Address& addr);
        const vector<Address>& getAddresses() const;

        bool hasType(MessageType type);



    private:
        vector<Address> addresses;
        MessageTypeFlag types;

    };
}

#endif
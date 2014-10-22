#ifndef favor_contact_include
#define favor_contact_include

#include "favor.h"
#include "address.h"

namespace favor {
    class Contact {
    public:
        const int id;
        const string displayName;

        Contact(int ident, string name);
        Contact(int ident, string name, const vector <Address> &addrs);

        void addAddress(const Address& addr);
        bool generated();
        vector<Address>& getAddresses();


    private:
        vector<Address> addresses;

    };
}

#endif
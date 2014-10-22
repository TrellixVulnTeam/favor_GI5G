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
        Contact(int ident, string name, const Address& addr);
        Contact(int ident, string name, const vector <Address> &addrs);

        Contact(const Contact& c) = delete; //Like account manager, we don't really want these copied

        void addAddress(const Address& addr);
        bool generated() const;
        const vector<Address>& getAddresses() const;


    private:
        vector<Address> addresses;

    };
}

#endif
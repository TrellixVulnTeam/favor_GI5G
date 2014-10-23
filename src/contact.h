#ifndef favor_contact_include
#define favor_contact_include

#include "favor.h"
#include "address.h"

namespace favor {
    class Contact {
    public:
        //TODO: do contacts really need to know their ID?
        const int id;
        const string displayName;
        const MessageType type;

        Contact(int ident, string name, MessageType t);
        Contact(int ident, string name, MessageType t, const Address& addr);
        Contact(int ident, string name, MessageType t, const vector <Address> &addrs);

        Contact(const Contact& c) = delete; //Like account manager, we don't really want these copied

        void addAddress(const Address& addr);
        const vector<Address>& getAddresses() const;


    private:
        vector<Address> addresses;

    };
}

#endif
#include "contact.h"

namespace favor{
    Contact::Contact(int ident, string name, MessageType t) : id(ident), displayName(name), type(t){}
    Contact::Contact(int ident, string name, MessageType t,  const Address &addr) : id(ident), displayName(name), type(t) {
        addresses.push_back(addr);
    }
    Contact::Contact(int ident, string name, MessageType t, const vector <Address> &addrs) : id(ident), displayName(name),
                                                                                             type(t), addresses(addrs) {}

    const vector<Address>& Contact::getAddresses() const {
        return addresses;
    }

    void Contact::addAddress(const Address &addr) {
        addresses.push_back(addr);
    }
}
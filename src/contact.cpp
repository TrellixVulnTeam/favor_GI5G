#include "contact.h"

namespace favor{
    Contact::Contact(int ident, string name) : id(ident), displayName(name){}
    Contact::Contact(int ident, string name, const vector <Address> &addrs) : id(ident), displayName(name), addresses(addrs) {}

    vector<Address>& Contact::getAddresses() {
        return addresses;
    }

    void Contact::addAddress(const Address &addr) {
        addresses.push_back(addr);
    }

    bool Contact::generated() {
        return id == -1;
    }
}
#include "contact.h"

namespace favor{
    Contact::Contact(int ident, string name) : id(ident), displayName(name){}
    Contact::Contact(int ident, string name, const Address &addr) : id(ident), displayName(name){
        addresses.push_back(addr);
    }
    Contact::Contact(int ident, string name, const vector <Address> &addrs) : id(ident), displayName(name), addresses(addrs) {}

    const vector<Address>& Contact::getAddresses() const {
        return addresses;
    }

    void Contact::addAddress(const Address &addr) {
        addresses.push_back(addr);
    }

    bool Contact::generated() const {
        return id == -1;
    }
}
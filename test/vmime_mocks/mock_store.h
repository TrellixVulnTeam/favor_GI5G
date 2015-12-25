//
// Created by josh on 12/24/15.
//

#ifndef FAVOR_MOCK_STORE_H
#define FAVOR_MOCK_STORE_H

#include "favor.h"
#include "vmime/include/vmime/net/imap/IMAPStore.hpp"
#include "vmime/include/vmime/security/defaultAuthenticator.hpp"
#include "mock_folder.h"

class MockImapStore: public vmime::net::imap::IMAPStore {
public:
    MockImapStore() : IMAPStore(std::make_shared<vmime::net::session>(), std::make_shared<vmime::security::defaultAuthenticator>(), false){};

    void connect(){}

    std::shared_ptr <vmime::net::folder> getRootFolder()
    {

        return std::make_shared <MockImapFolder>(std::dynamic_pointer_cast<IMAPStore>(shared_from_this()));
    }

};


#endif //FAVOR_MOCK_STORE_H

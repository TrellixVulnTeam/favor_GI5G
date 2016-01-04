//
// Created by josh on 1/3/16.
//

#ifndef FAVOR_MOCK_MESSAGE_H
#define FAVOR_MOCK_MESSAGE_H

#include "vmime/include/vmime/net/imap/IMAPMessage.hpp"
#include "vmime/include/vmime/net/imap/IMAPFolder.hpp"


class MockImapMessage : public vmime::net::imap::IMAPMessage {
public:
    MockImapMessage(std::shared_ptr <vmime::net::imap::IMAPFolder> folder, const int num, const uid& uid) :
            IMAPMessage(folder, num, uid) {}

};


#endif //FAVOR_MOCK_MESSAGE_H
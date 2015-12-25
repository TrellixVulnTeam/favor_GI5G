//
// Created by josh on 12/25/15.
//

#ifndef FAVOR_MOCK_STATUS_H
#define FAVOR_MOCK_STATUS_H


//I know this is awful, but the getUidValidity() and getUidNext() methods aren't virtual and hence can't be overriden
#define private protected
#include "vmime/include/vmime/net/imap/IMAPFolderStatus.hpp"
#define private private


class MockImapFolderStatus : public vmime::net::imap::IMAPFolderStatus {

public:
    MockImapFolderStatus(){
        m_uidValidity = 1;
        m_uidNext = 2;
    }
    MockImapFolderStatus(long uidValidity, long uidNext){
        m_uidValidity = uidValidity;
        m_uidNext = uidNext;
    }


protected:

};

#endif //FAVOR_MOCK_STATUS_H

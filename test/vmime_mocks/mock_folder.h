//
// Created by josh on 12/25/15.
//

#ifndef FAVOR_MOCK_FOLDER_H
#define FAVOR_MOCK_FOLDER_H

#include "vmime/include/vmime/utility/path.hpp"
#include "vmime/include/vmime/net/imap/IMAPStore.hpp"
#include "vmime/include/vmime/net/imap/IMAPFolder.hpp"
#include "vmime/include/vmime/net/folderAttributes.hpp"
#include "mock_status.h"
#include "mock_message.h"
#include "../testing_definitions.h"


class MockImapFolder : public vmime::net::imap::IMAPFolder {
public:
    MockImapFolder(std::shared_ptr<vmime::net::imap::IMAPStore> store) : IMAPFolder(vmime::net::folder::path(), store, std::make_shared<vmime::net::folderAttributes>()),
    mockName("USELESS_FOLDER"), mockOpen(false){};

    std::vector <std::shared_ptr <vmime::net::folder> > getFolders(const bool recursive)
    {

        std::vector <std::shared_ptr <vmime::net::folder> > v;
        v.push_back(std::make_shared<MockImapFolder>(std::dynamic_pointer_cast<vmime::net::imap::IMAPStore>(getStore())));
        v.push_back(std::make_shared<MockImapFolder>(std::dynamic_pointer_cast<vmime::net::imap::IMAPStore>(getStore())));
        std::shared_ptr<MockImapFolder> sent = std::make_shared<MockImapFolder>(std::dynamic_pointer_cast<vmime::net::imap::IMAPStore>(getStore()));
        sent->mockName = "SENT_FOLDER";
        std::shared_ptr<MockImapFolder> rec = std::make_shared<MockImapFolder>(std::dynamic_pointer_cast<vmime::net::imap::IMAPStore>(getStore()));
        rec->mockName = "INBOX";
        v.push_back(sent);
        v.push_back(rec);
        v.push_back(std::make_shared<MockImapFolder>(std::dynamic_pointer_cast<vmime::net::imap::IMAPStore>(getStore())));


        return (v);
    }

    const folder::path::component getName() const
    {
        return vmime::net::folder::path::component(mockName);
    }

    std::shared_ptr <vmime::net::folderStatus> getStatus()
    {
        return std::make_shared<MockImapFolderStatus>(mockStatus);
    }

    void setStatus(MockImapFolderStatus input)
    {
        mockStatus = input;
    }

    bool isOpen() const override
    {
        return mockOpen;
    }

    void open(const int mode, bool failIfModeIsNotAvailable) override
    {
        mockOpen = true;
    }


    void close(const bool expunge) override
    {
        mockOpen = false;
    }

    vmime::net::messageSet UIDSearch(const std::string& input) override
    {
        std::vector <vmime::net::message::uid> uids;
        return vmime::net::messageSet::byUID(uids);
    }

    std::vector<std::shared_ptr<vmime::net::message>> getAndFetchMessages
            (const vmime::net::messageSet& msgs, const vmime::net::fetchAttributes& attribs) override
    {
        std::vector <std::shared_ptr<vmime::net::message> > messages;
        for (int i = 0; i < EMAILMANAGER_MAIL_COUNT; i++){
            messages.push_back(std::make_shared<MockImapMessage>(std::dynamic_pointer_cast<vmime::net::imap::IMAPFolder>(shared_from_this()), i, i*2));
        }
        return messages;
    }



protected:
    std::string mockName;
    MockImapFolderStatus mockStatus;
    bool mockOpen;
};


#endif //FAVOR_MOCK_FOLDER_H

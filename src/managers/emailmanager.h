/*
Copyright (C) 2015  Joshua Tanner (mindful.jt@gmail.com)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#ifndef favor_emailmanager_include
#define favor_emailmanager_include

#include "favor.h"
#include "accountmanager.h"
#include "vmime/include/vmime/vmime.hpp"
#include "vmime/include/vmime/net/imap/IMAPFolderStatus.hpp"
#include "tidy-html5/tidy.h"
#include "tidy-html5/buffio.h"

namespace favor {
    class EmailManager : public AccountManager {
    public:
        EmailManager(string accNm, string detailsJson);

    protected:
        void fetchMessages() override;

        void fetchAddresses() override;

        void updateJson() override;
        void consultJson(bool initial = false) override;

    private:
        long lastSentUid;
        long lastReceivedUid;
        long lastSentUidValidity;
        long lastReceivedUidValidity;
        string password;
        vmime::utility::url serverURL;
        static const char* addrListName;
        std::set<string> managedAddresses;

        shared_ptr<vmime::net::store> login();

        string folderList(vector<shared_ptr<vmime::net::folder>> folders);

        void parseMessage(bool sent, favor::shared_ptr<vmime::net::message> m);

        void handleHTML(vmime::utility::outputStream *out, std::stringstream &ss, shared_ptr<const vmime::htmlTextPart> part);

        bool hasMedia(shared_ptr<vmime::net::messageStructure> structure);

        std::time_t toTime(const vmime::datetime input);

        string searchCommand(bool sent, shared_ptr<const vector<Address>> addresses, long startUid, long endUid);

        std::pair<std::shared_ptr<vmime::net::folder>, std::shared_ptr<vmime::net::folder>> findSentRecFolder(favor::shared_ptr<vmime::net::store> st);

        void fetchFromFolder(favor::shared_ptr<vmime::net::folder> folder, shared_ptr<const vector<Address>> addresses, bool catchUp);
    };
}

#endif
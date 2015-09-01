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



#ifndef favor_skypemanager_include
#define favor_skypemanager_include

#include "favor.h"
#include "accountmanager.h"

namespace favor {
    class SkypeManager : public AccountManager {
    public:
        SkypeManager(string accNm, string detailsJson);


    protected:
        void fetchMessages() override;

        void fetchAddresses() override;

        void updateJson() override;
        void consultJson(bool initial = false) override;

    private:
        string skypeDatabaseLocation;
        long lastFetchTime;
        std::set<string> managedAddresses;
        static const char* addrListName;

    };
}

#endif
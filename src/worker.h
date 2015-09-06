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



#ifndef favor_worker_include
#define favor_worker_include

#include "favor.h"

namespace favor {
    namespace worker {
        //Basic
        void initialize();

        void cleanup();

        //Database
        void buildDatabase();

        void dropDatabase();

        void truncateDatabase();

        void indexDatabase();

        void deindexDatabase();


        void beginTransaction();

        void commitTransaction();

        void rollbackTransaction();

        void backupDatabase();

        //Writing methods
        void exec(string sql);

        void createContactWithAddress(const string &address, MessageType type, const string &displayName);

        void createContactFromAddress(const Address& addr, const string& displayName);

        void recomputeAddressTable(std::unordered_map<std::string, int>& countedAddresses, std::unordered_map<std::string, std::string>& addressNames, MessageType type);
    }
}

#endif
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



#ifndef favor_processor_include
#define favor_processor_include

#include "favor.h"
#include "reader.h"
#include "contact.h"
#include "address.h"

namespace favor {
    namespace processor {
        //TODO: right now these only support single accounts and contacts, but the underlying query framework is set up to support much more

        //AccountManager determines relevant account (limited to one.. for now). NULL means a query on everything.
        //The response time methods take pointers for consistency but a null contact there doesn't actually mean anything

        double averageCharcount(AccountManager* account, const Contact* c, time_t fromDate, time_t untilDate, bool sent);
        double averageConversationalResponsetime(AccountManager* account, const Contact* c, time_t fromDate, time_t untilDate, bool sent);
        long responseTimeNintiethPercentile(AccountManager* account, const Contact* c, time_t fromDate, time_t untilDate, bool sent);

        long totalCharcount(AccountManager* account, const Contact* c, time_t fromDate, time_t untilDate, bool sent);
        long totalMessagecount(AccountManager* account, const Contact* c, time_t fromDate, time_t untilDate, bool sent);


        template<typename T>
        void cacheResult(ResultType t, AccountManager* account, const Contact* const contact, long fromD, long untilD, bool sent,  T input);
        template<typename T>
        T getResult(ResultType t, AccountManager* account, const Contact* const contact, long fromD, long untilD, bool sent);
        bool countResult(ResultType t, AccountManager* account, const Contact* const contact, long fromD, long untilD, bool sent);


    }
}

#endif
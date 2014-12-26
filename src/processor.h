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

        double averageCharcount(AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent);
        double averageConversationalResponsetime(AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent);
        long responseTimeNintiethPercentile(AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent);

        long totalCharcount(AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent);
        long totalMessagecount(AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent);


        template<typename T>
        void cacheResult(ResultType t, AccountManager* account, const Contact* const contact, long fromD, long untilD, bool sent,  T input);
        template<typename T>
        T getResult(ResultType t, AccountManager* account, const Contact* const contact, long fromD, long untilD, bool sent);
        bool countResult(ResultType t, AccountManager* account, const Contact* const contact, long fromD, long untilD, bool sent);


    }
}

#endif
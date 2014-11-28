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

        double averageCharcount(const AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent);
        double averageResponsetime(const AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent);

        long totalCharcount(const AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent);
        long totalResponsetime(const AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent);
        long totalMessagecount(const AccountManager* account, const Contact& c, time_t fromDate, time_t untilDate, bool sent);


    }
}

#endif
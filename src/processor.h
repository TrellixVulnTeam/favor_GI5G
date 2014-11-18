#ifndef favor_processor_include
#define favor_processor_include

#include "favor.h"
#include "reader.h"
#include "contact.h"
#include "address.h"

namespace favor {
    namespace processor {
        //TODO: is it worth it to rig up some crazyness where queries can be passed to the reader/query object and it computes the stuff we need without ever constructing
        //message objects? This might not be terribly dirty (then again it might) and might be a lot fasterh, but it's also undeniably less elegant

        //TODO: think about how exactly we want to implement the cache, and also think about whether we should return pairs or a specific "result"
        //type.


        std::pair<long, long> averageCharcount(const Contact& c, time_t fromDate, time_t untilDate);
        std::pair<long, long> averageResponsetime(const Contact& c, time_t fromDate, time_t untilDate);

        std::pair<long, long> totalCharcount(const Contact& c, time_t fromDate, time_t untilDate);
        std::pair<long, long> totalResponsetime(const Contact& c, time_t fromDate, time_t untilDate);
        std::pair<long, long> totalMessagecount(const Contact& c, time_t fromDate, time_t untilDate);


    }
}

#endif
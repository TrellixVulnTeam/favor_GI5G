#ifndef favor_processor_include
#define favor_processor_include

#include "favor.h"
#include "reader.h"
#include "contact.h"
#include "address.h"

namespace favor {
    namespace processor {
        //TODO: the internal methods in a perfect world would be standardized as some kind of... operation on the data, so that we could do them all in one iteration
        //but choose how many we wanted to do on any given pass through a list. This is almost certainly best done by treating the actual operations as an object with state
        //that know how to operate on each message they're fed, and can be told when they've hit the end of a list and will then spit out a result.
        //...not actually sure this will be any faster with cache locality. may be worth testing, or just trying normally first since the multiple computes will only happen on
        //fetches which are slow anyway.

        //TODO: think about how exactly we want to implement the cache, and also think about whether we should return pairs or a specific "result"
        //type.


        std::pair<long, long> averageCharcount(const Contact& c, time_t fromDate, time_t untilDate);
        std::pair<long, long> averageResponsetime(const Contact& c, time_t fromDate, time_t untilDate);

        std::pair<long, long> totalCharcount(const Contact& c, time_t fromDate, time_t untilDate);
        std::pair<long, long> totalResponsetime(const Contact& c, time_t fromDate, time_t untilDate);
        std::pair<long, long> totalMessagecount(const Contact& c, time_t fromDate, time_t untilDate);

        long charcountRatio(const Contact& c, time_t fromDate, time_t untilDate);
        long responsetimeRatio(const Contact& c, time_t fromDate, time_t untilDate); //TODO: does this make sense?
        long messagecountRatio(const Contact& c, time_t fromDate, time_t untilDate);

    }
}

#endif
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

        void updateFetchData() override;

    private:

    };
}

#endif
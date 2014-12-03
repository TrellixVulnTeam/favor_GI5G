#ifndef favor_linemanager_include
#define favor_linemanager_include

#include "favor.h"
#include "accountmanager.h"

namespace favor {
    class LineManager : public AccountManager {
    public:
        LineManager(string accNm, string detailsJson);


    protected:
        void fetchMessages() override;

        void fetchAddresses() override;

        void updateFetchData() override;

    private:

    };
}

#endif
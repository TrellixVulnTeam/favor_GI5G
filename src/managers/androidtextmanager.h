#ifndef favor_androidtextmanager_include
#define favor_androidtextmanager_include

#include <jni.h>
#include "favor.h"
#include "accountmanager.h"

namespace favor {
    class AndroidTextManager : public AccountManager {
    public:
        AndroidTextManager(string accNm, string detailsJson);

        static void setVM(JavaVM* inputVm);

    protected:
        void fetchMessages() override;

        void fetchAddresses() override;

        void updateFetchData() override;

    private:
        static JavaVM* vm;

    };
}

#endif
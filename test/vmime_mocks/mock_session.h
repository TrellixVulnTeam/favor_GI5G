//
// Created by josh on 12/24/15.
//

#ifndef FAVOR_MOCK_SESSON_H
#define FAVOR_MOCK_SESSON_H

#include "favor.h"
#include "mock_store.h"

#include "vmime/config.hpp"


#include "vmime/security/authenticator.hpp"

#if VMIME_HAVE_TLS_SUPPORT
#	include "vmime/net/tls/TLSProperties.hpp"
#endif // VMIME_HAVE_TLS_SUPPORT

#include "vmime/utility/url.hpp"

#include "vmime/propertySet.hpp"

using namespace vmime::net;
using namespace vmime;
using namespace std;

class store;
class transport;

class MockSession
{
    public:

        /** Return a store service instance for the specified URL.
          *
          * @param url full URL with at least the protocol to use (eg: "imap://username:password@myserver.com/")
          * @param auth authenticator object to use for the new store service. If
          * NULL, a default one is used. The default authenticator simply return user
          * credentials by reading the session properties "auth.username" and "auth.password".
          * @return a new store service, or NULL if no service is registered for this
          * protocol or is not a store protocol
          */
        std::shared_ptr <vmime::net::store> getStore
                (const utility::url& url,
                 std::shared_ptr <security::authenticator> auth = null){
            return std::make_shared<MockImapStore>();
        }

        std::unordered_map<string, string>& getProperties(){
            return properties;
        };

    protected:
        std::unordered_map<string, string> properties;
};


#endif //FAVOR_MOCK_SESSON_H

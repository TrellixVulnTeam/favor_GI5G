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



#ifndef favor_emailmanager_include
#define favor_emailmanager_include

#include "favor.h"
#include "accountmanager.h"
#include "vmime/include/vmime/vmime.hpp"
#include "vmime/include/vmime/net/imap/IMAPFolderStatus.hpp"
#include "tidy-html5/tidy.h"
#include "tidy-html5/tidybuffio.h"

namespace favor {
    class EmailManager : public AccountManager {
        friend class EmailManagerTest;
        friend class AccountManager;

    public:
        bool addressValid(const string &address) override;

    protected:
        EmailManager(string accNm, string detailsJson);
        void fetchMessages() override;

        void fetchAddresses() override;

        void updateJson() override;
        void consultJson(bool initial = false) override;

    private:
        long lastSentUid;
        long lastReceivedUid;
        long lastSentUidValidity;
        long lastReceivedUidValidity;
        string password;
        vmime::utility::url serverURL;
        static std::regex utf8regex;
        static std::regex emailRegex;
        static const std::unordered_map<string, string> imapServers;

        shared_ptr<vmime::net::store> login();

        string folderList(vector<shared_ptr<vmime::net::folder>> folders);

        bool toXML(std::stringstream &ss);

        void parseMessage(bool sent, favor::shared_ptr<vmime::net::message> m);

        bool parseMessageParts(long uid, const vmime::messageParser& mp, string& bodyFinal);

        void holdParsedMessage(bool failure, const vmime::messageParser &mp, bool sent, long uid, time_t date,
                               bool media, string bodyFinal);

        void handleHTMLandEncoding(vmime::utility::outputStream *out, std::stringstream &ss,
                                   shared_ptr<const vmime::htmlTextPart> part, string &output);

        bool hasMedia(shared_ptr<vmime::net::messageStructure> structure);

        void processFetchedAddresses(vector<shared_ptr<vmime::net::message>> fetchedAddresses);

        static std::time_t toTime(const vmime::datetime input);

        string searchCommand(bool sent, shared_ptr<const vector<Address>> addresses, long startUid, long endUid);

        SentRec<std::shared_ptr<vmime::net::folder>> findSentRecFolder(favor::shared_ptr<vmime::net::store> st);

        void fetchFromFolder(favor::shared_ptr<vmime::net::folder> folder, shared_ptr<const vector<Address>> addresses, bool catchUp);

        class InfoTracer : public vmime::net::tracer {
        public:
            void traceSend(const string &line) override {
                DLOG("[" + service->getProtocolName() + "] Sent: " + line);
            }

            void traceReceive(const string &line) override {
                DLOG("[" + service->getProtocolName() + "] Received: " + line);
            }

            InfoTracer(vmime::shared_ptr<vmime::net::service> serv, const int id) : service(serv), connectionId(id) {
            }

        private:
            vmime::shared_ptr<vmime::net::service> service;
            const int connectionId;
        };

        class InfoTracerFactory : public vmime::net::tracerFactory {
        public:
            shared_ptr<vmime::net::tracer> create(shared_ptr<vmime::net::service> serv, const int connectionId) override {
                return std::make_shared<InfoTracer>(serv, connectionId);
            }

        };


        /*
         * According to VMIME:
         * If you need to do more complex verifications on certificates, you will
         * have to write your own verifier. Your verifier should inherit from the
         * vmime::security::cert::certificateVerifier class and implement the method
         * verify(). Then, if the specified certificate chain is trusted, simply return from the function,
         * or else throw a certificate verification exception.
         *
         * ...but this is not something Favor currently implements. A lot of potential maintenance work/overhead in
         * maintaining certificates that would just be for one manager.
        */
        // Certificate verifier (TLS/SSL)


        class TrustingCertificateVerifier : public vmime::security::cert::certificateVerifier {
        public:
            void verify(vmime::shared_ptr<vmime::security::cert::certificateChain> certs, const string &hostname) override {
                return;
            }
        };
    };
}

#endif
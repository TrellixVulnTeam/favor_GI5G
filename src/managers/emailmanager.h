#ifndef favor_emailmanager_include
#define favor_emailmanager_include

#include "favor.h"
#include "accountmanager.h"
#include "vmime/include/vmime/vmime.hpp"
#include "tidy-html5/tidy.h"
#include "tidy-html5/buffio.h"

namespace favor{
  class EmailManager : public AccountManager {
  public:
    EmailManager(string accNm, string detailsJson);
  protected:
    void fetchMessages() override;
    void fetchContacts() override; 
  private:
    long lastSentUid;
    long lastReceivedUid;
    long lastSentUidValidity;
    long lastReceivedUidValidity;
    string password;
    vmime::utility::url serverURL;
    
    void saveFetchMetadata();
    shared_ptr<vmime::net::store> login();
    string folderList(vector<shared_ptr<vmime::net::folder>> folders);
    void parseMessage(bool sent, favor::shared_ptr<vmime::net::message> m);
    bool hasMedia(shared_ptr<vmime::net::messageStructure> structure);
    std::time_t toTime(const vmime::datetime input);
    string searchCommand(bool sent, const favor::vector<favor::string>& addresses, long int uid);
    std::pair< std::shared_ptr<vmime::net::folder>, std::shared_ptr<vmime::net::folder>> findSentRecFolder(favor::shared_ptr<vmime::net::store> st);
    void fetchFromFolder(favor::shared_ptr< vmime::net::folder > folder, const favor::vector< favor::string >& addresses);
  };
}

#endif
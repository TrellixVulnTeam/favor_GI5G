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



#include "skypemanager.h"

namespace favor{

    SkypeManager::SkypeManager(string accNm, string detailsJson) : AccountManager(accNm, TYPE_SKYPE, detailsJson) {}


    void SkypeManager::updateJson() {
        setJsonLong(lastFetchTime);
        rapidjson::Value addrsVal;
        addrsVal.SetArray();
        for (auto it = managedAddresses.begin(); it != managedAddresses.end(); ++it){
            addrsVal.PushBack(rapidjson::Value(it->c_str(), json.GetAllocator()).Move(), json.GetAllocator());
        }
        json[addrListName] = addrsVal;
    }


    const char* SkypeManager::addrListName  = "managedAddresses";

    void SkypeManager::consultJson(bool initial) {
        if (initial) {
            if (json.HasMember("skypeDatabaseLocation"))
                skypeDatabaseLocation = json["skypeDatabaseLocation"].GetString();
            else throw badUserDataException("Skype manager missing database location");
        }

        if (json.HasMember(addrListName)){
            rapidjson::Value& addrsVal = json[addrListName];
            if (!addrsVal.IsArray()) throw badUserDataException("Managed addresses list improperly formatted in "+accountName +" json");
            else {
                for (auto it = addrsVal.Begin(); it!= addrsVal.End(); ++it){
                    //TODO: do skype addresses need to be validated somehow?
                    managedAddresses.insert(it->GetString());
                }
            }
        }
        else {
            rapidjson::Value addrsVal;
            addrsVal.SetArray();
            json.AddMember(rapidjson::Value(addrListName, json.GetAllocator()).Move(), addrsVal, json.GetAllocator());
        }

        getJsonLong(lastFetchTime, 0);
    }


    void SkypeManager::fetchAddresses() {
        //TODO
    }

    void SkypeManager::fetchMessages() {
        //TODO
    }


}
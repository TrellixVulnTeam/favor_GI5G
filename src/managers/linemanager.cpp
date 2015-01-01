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



#include "linemanager.h"

namespace favor{

    LineManager::LineManager(string accNm, string detailsJson) : AccountManager(accNm, TYPE_LINE, detailsJson) {}


    void LineManager::updateFetchData(){
        //TODO:
    }


    void LineManager::fetchAddresses() {
        //TODO
    }

    void LineManager::fetchMessages() {
        //TODO
    }


}
//
//  Row.cpp
//  Assignment4
//
//  Created by rick gessner on 4/19/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#include "Row.hpp"
#include "Database.hpp"
#include <string>

namespace ECE141 {
    StatusResult Row::encode(std::ostream &aWriter) {
        aWriter << this->rowNum << " " << this->blockNumber <<
                " " << this->refID << " attrs ";
        // loop through the map

        for (auto it = this->data.begin(); it != this->data.end(); it++) {
            aWriter << "attr " << it->first <<
                    " " << it->second.index() << " ";

            if(it->second.index() == 3) {
                std::string tempString = std::get<std::string>(it->second);
                std::replace(tempString.begin(), tempString.end(), ' ', '~');
                aWriter << tempString;
            }
            else {
                std::visit([&](const auto &x) {
                    aWriter << x;
                }, it->second);
            }

            aWriter << " end ";
        }

        aWriter << "end";

        return StatusResult();
    }

    StatusResult Row::decode(std::istream &aReader) {
        std::string tempCheck;
        aReader >> this->rowNum >> this->blockNumber
                >> this->refID >> tempCheck;
        StatusResult theResult = StatusResult();

        if (tempCheck == "attrs") {
            aReader >> tempCheck;

            while (tempCheck == "attr") {
                std::string theKey;
                size_t index;
                Value theValue;
                aReader >> theKey >> index;
                theResult = decodeValue(aReader, index, theValue);

                if(theResult != StatusResult())
                    return theResult;

                this->data.insert({theKey, theValue});
                aReader >> tempCheck;

                if(tempCheck == "end")
                    aReader >> tempCheck;
                else
                    return StatusResult{Errors::readError};
            }

            aReader >> tempCheck;

            if(tempCheck != "end")
                return StatusResult{Errors::readError};
        }

        return StatusResult();
    }

    StatusResult Row::decodeValue(std::istream &aReader, size_t &anIndex, Value &aValue) {
        if(anIndex == 0) {
            bool theBool;
            aReader >> theBool;
            aValue = theBool;
        }
        else if(anIndex == 1) {
            int theInt;
            aReader >> theInt;
            aValue = theInt;
        }
        else if(anIndex == 2) {
            double theDoub;
            aReader >> theDoub;
            aValue = theDoub;
        }
        else if(anIndex == 3) {
            std::string theStr;
            aReader >> theStr;
            std::replace(theStr.begin(), theStr.end(), '~', ' ');
            aValue = theStr;
        }
        else
            return StatusResult{Errors::unknownError};

        return StatusResult();
    }

    void Row::setName(std::string &aName) {
        this->refID = Entity::hashString(aName);
    }

    void Row::setBlockNum(uint32_t aBlockNumber) {
        this->blockNumber = aBlockNumber;
    }

    void Row::setRowNum(uint32_t aRowNumber) {
        this->rowNum = aRowNumber;
    }

    std::string Row::getDecodedValue(std::string &aKey) {
        std::stringstream theStream;

        std::visit([&](const auto &x) {
            theStream << x;
        }, this->data[aKey]);

        return theStream.str();
    }

    Value Row::getValue(std::string &aKey) {
        return this->data[aKey];
    }

    StatusResult Row::updateValue(std::string &aKey, Value &aValue) {
        this->data[aKey] = aValue;

        return StatusResult();
    }

    uint32_t Row::getRefID() {
        return this->refID;
    }

    uint32_t Row::getBlockNum() {
        return this->blockNumber;
    }

    KeyValues& Row::getData() {
        return this->data;
    }

    size_t Row::numAttributes() {
        return this->data.size();
    }

    void Row::addAttribute(std::pair<std::string, Value> &aPair) {
        this->data.insert({aPair.first, aPair.second});
    }

    StatusResult Row::addDefault(Entity* anEntity, Attribute &anAttribute) {
        if(anEntity) {
            std::pair<std::string, Value> thePair;

            if (anAttribute.isAuto()) {
                thePair = {anAttribute.getName(), anEntity->useAuto(anAttribute.getName())};
            } else if (!anAttribute.isNullable()) {
                thePair = {anAttribute.getName(), anAttribute.defaultValueVal()};
            } else
                return StatusResult{Errors::unknownError};

            addAttribute(thePair);

            return StatusResult();
        }

        return StatusResult{Errors::badPointer};
    }

    StatusResult Row::addNull(Entity *anEntity, Attribute &anAttribute) {
        if(anEntity) {
            std::pair<std::string, Value> thePair;

            if(anAttribute.isNullable()) {
                thePair = {anAttribute.getName(), anAttribute.nullValueVal()};
            }

            addAttribute(thePair);

            return StatusResult();
        }

        return StatusResult{Errors::badPointer};
    }

    bool Row::loadValue(std::string &aName, Value &aValue) {
        if (this->data.find(aName) != this->data.end()) {
            aValue = this->data[aName];
            return true;
        }

        return false;
    }
}

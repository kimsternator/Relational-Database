//
//  Entity.cpp
//
//  Created by rick gessner on 4/03/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#include <sstream>
#include "Entity.hpp"


namespace ECE141 {

    //STUDENT: Implement the Entity class here...

    Entity &Entity::operator=(const Entity &anEntity) {
        this->name = anEntity.name;
        this->blockNum = anEntity.blockNum;
        this->refID = anEntity.refID;

        for (auto attribute: anEntity.attributes)
            this->attributes.push_back(attribute);

        for (auto theAuto: anEntity.autoValues)
            this->autoValues.insert({theAuto.first, theAuto.second});

        return *this;
    }

    const int gMultiplier = 37;

    //hash given string to numeric quantity...
    uint32_t Entity::hashString(const char *str) {
        uint32_t h{0};
        unsigned char *p;
        for (p = (unsigned char *) str; *p != '\0'; p++)
            h = gMultiplier * h + *p;
        return h;
    }

    uint32_t Entity::hashString(const std::string &aStr) {
        return hashString(aStr.c_str());
    }

    //OCF...

    //other entity methods...
    StatusResult Entity::addBlockNum(uint32_t aBlockNum) {
        this->blockNum = aBlockNum;

        return StatusResult();
    }

    StatusResult Entity::encode(std::ostream &aWriter) {
        aWriter << this->name << " " << this->blockNum <<
                " " << this->refID << " " << this->rowNum << " attrs ";

        for (int i = 0; i < this->attributes.size(); i++) {
            aWriter << "attr ";
            this->attributes[i].encode(aWriter);
            aWriter << " end ";
        }

        aWriter << "end vals ";

        for (auto it = this->autoValues.begin(); it != this->autoValues.end(); it++) {
            aWriter << "val " << it->first << " " << it->second << " end ";
        }

        aWriter << "end";

        return StatusResult{noError};
    }

    StatusResult Entity::decode(std::istream &aReader) {
        StatusResult theResult = StatusResult{Errors::readError};
        std::string tempCheck;
        aReader >> this->name >> this->blockNum >>
                this->refID >> this->rowNum;
        aReader >> tempCheck;

        if (tempCheck == "attrs") {
            theResult = decodeAttributes(aReader);

            if(theResult == StatusResult())
                aReader >> tempCheck;

                if(tempCheck == "vals")
                    theResult = decodeAutos(aReader);
        }

        return theResult;
    }

    StatusResult Entity::decodeAttributes(std::istream &aReader) {
        std::string tempCheck;
        aReader >> tempCheck;

        while (tempCheck != "end") {
            if (tempCheck == "attr") {
                Attribute theAttr;
                theAttr.decode(aReader);
                this->attributes.emplace_back(theAttr);
            }

            aReader >> tempCheck;

            if (tempCheck != "end")
                return StatusResult{Errors::readError};

            aReader >> tempCheck;
        }

        return StatusResult();
    }

    StatusResult Entity::decodeAutos(std::istream &aReader) {
        std::string tempCheck;
        aReader >> tempCheck;

        while (tempCheck != "end") {
            if (tempCheck == "val") {
                std::string theName;
                int theVal;
                aReader >> theName >> theVal;
                this->autoValues.insert({theName, theVal});
            }

            aReader >> tempCheck;

            if (tempCheck != "end")
                return StatusResult{Errors::readError};

            aReader >> tempCheck;
        }

        return StatusResult();
    }

    StatusResult Entity::addAttribute(Attribute &anAttribute) {
        this->attributes.emplace_back(anAttribute);

        return StatusResult();
    }

    AttributeList Entity::getAttributes() {
        return this->attributes;
    }

    Attribute *Entity::getAttribute(const std::string &aName) {
        for (int i = 0; i < this->attributes.size(); i++)
            if (this->attributes[i].getName() == aName)
                return &this->attributes[i];

        return nullptr;
    }

    DataTypes Entity::getAttributeDatatype(const std::string &aName) {
        for (int i = 0; i < this->attributes.size(); i++)
            if (this->attributes[i].getName() == aName)
                return this->attributes[i].getType();

        return DataTypes::no_type;
    }

    StatusResult Entity::setRefID() {
        this->refID = hashString(this->name);

        return StatusResult();
    }

    StatusResult Entity::setAutos() {
        for(auto attr: this->attributes) {
            if(attr.isAuto()) {
                this->autoValues.insert({attr.getName(), 1});
            }
        }

        return StatusResult();
    }

    std::string Entity::getName() {
        return this->name;
    }

    std::string Entity::getPrimaryKeyName() {
        for(auto attr: this->attributes) {
            if(attr.isPrimary()) {
                return attr.getName();
            }
        }

        return "";
    }

    uint32_t Entity::getRefID() {
        return this->refID;
    }

    uint32_t Entity::getBlockNum() {
        return this->blockNum;
    }

    uint32_t Entity::useRowNum() {
        return this->rowNum++;
    }

    int Entity::useAuto(std::string &aName) {
        auto it = this->autoValues.find(aName);

        return it->second++;
    }

    size_t Entity::describeDisplay(std::ostream &anOutput) {
        for (auto attribute: this->attributes)
            attribute.describeDisplay(anOutput);

        return this->attributes.size();
    }
}

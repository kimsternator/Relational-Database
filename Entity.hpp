//
//  Entity.hpp
//
//  Created by rick gessner on 4/03/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//


#ifndef Entity_hpp
#define Entity_hpp

#include <stdio.h>
#include <vector>
#include <optional>
#include <memory>
#include <string>

#include "Attribute.hpp"
#include "BasicTypes.hpp"
#include "Errors.hpp"
#include "Storage.hpp"

namespace ECE141 {

    using AttributeOpt = std::optional<Attribute>;
    using AttributeList = std::vector<Attribute>;

    //------------------------------------------------

    class Entity : public Storable {
    public:

        static uint32_t hashString(const char *str);
        static uint32_t hashString(const std::string &aStr);

        //declare ocf methods...
        Entity() {
            name = "";
            blockNum = 0;
            refID = 0;
            rowNum = 1;
        }

        Entity(const std::string &aName) {
            name = aName;
            blockNum = 0;
            refID = 0;
            rowNum = 1;
        }

        ~Entity() = default;

        Entity &operator=(const Entity &anEntity);

        //declare methods you think are useful for entity...
        StatusResult addBlockNum(uint32_t aBlockNum);

        //this is the storable interface...
        StatusResult encode(std::ostream &aWriter) override;
        StatusResult decode(std::istream &aReader) override;

        StatusResult decodeAttributes(std::istream &aReader);
        StatusResult decodeAutos(std::istream &aReader);

        StatusResult addAttribute(Attribute &anAttribute);
        AttributeList getAttributes();
        Attribute *getAttribute(const std::string &aName);
        DataTypes getAttributeDatatype(const std::string &aName);

        StatusResult setRefID();
        StatusResult setAutos();
        std::string getName();
        std::string getPrimaryKeyName();
        uint32_t getRefID();
        uint32_t getBlockNum();
        uint32_t useRowNum();
        int useAuto(std::string &aName);

        size_t describeDisplay(std::ostream &anOutput);

    protected:

        AttributeList attributes;
        std::string name;
        uint32_t blockNum;
        uint32_t refID;
        //only supporting integer auto increments for now
        std::map<std::string, int> autoValues;

        uint32_t rowNum;
        //surely there must be other data members?
    };

}
#endif /* Entity_hpp */

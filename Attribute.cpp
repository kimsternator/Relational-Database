//
//  Attribute.hpp
//
//  Created by rick gessner on 4/02/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#include <iostream>
#include "Attribute.hpp"

namespace ECE141 {

    std::string Attribute::typeToString(DataTypes aType) {
        static std::map<DataTypes, std::string> typesDS = {
                {DataTypes::int_type,      "integer"},
                {DataTypes::varchar_type,  "varchar"},
                {DataTypes::float_type,    "float"},
                {DataTypes::datetime_type, "date"},
                {DataTypes::bool_type,     "boolean"},
        };

        return typesDS[aType];
    }

    DataTypes Attribute::stringToType(std::string &aType) {
        static std::map<std::string, DataTypes> typesSD = {
                {"integer", DataTypes::int_type},
                {"varchar", DataTypes::varchar_type},
                {"float",   DataTypes::float_type},
                {"date",    DataTypes::datetime_type},
                {"boolean", DataTypes::bool_type},
        };

        return typesSD[aType];
    }

    DataTypes Attribute::indexToType(size_t anIndex) {
        static std::map<size_t, DataTypes> typesID = {
                {0, DataTypes::bool_type},
                {1, DataTypes::int_type},
                {2, DataTypes::float_type},
                {3, DataTypes::varchar_type},
        };

        return typesID[anIndex];
    }

    std::string Attribute::defaultVal() {
        static std::map<DataTypes, std::string> defaultTypes = {
                {DataTypes::int_type,      "0"},
                {DataTypes::varchar_type,  ""},
                {DataTypes::float_type,    "0.0"},
                {DataTypes::datetime_type, ""},
                {DataTypes::bool_type,     ""},
        };

        return defaultTypes[this->type];
    }

    Value Attribute::defaultValueVal() {
        static std::map<DataTypes, Value> defaultValueTypes = {
                {DataTypes::int_type,      0},
                {DataTypes::varchar_type,  ""},
                {DataTypes::float_type,    0.0},
                {DataTypes::datetime_type, ""},
                {DataTypes::bool_type,     false},
        };

        return defaultValueTypes[this->type];
    }

    Value Attribute::nullValueVal() {
        static std::map<DataTypes, Value> nullValues = {
                {DataTypes::int_type,      -1},
                {DataTypes::varchar_type,  "NULL"},
                {DataTypes::float_type,    -1.0},
                {DataTypes::datetime_type, ""},
                {DataTypes::bool_type,     false},
        };

        return nullValues[this->type];
    }

    //set to primary key
    StatusResult Attribute::setPrimary() {
        this->primary_key = true;

        return StatusResult();
    }

    //STUDENT: implement Attribute class...
    StatusResult Attribute::encode(std::ostream &aWriter) {
        aWriter << this->name << " " << typeToString(this->type) << " ";

        if(this->type == DataTypes::varchar_type)
            aWriter << *(this->fieldLength) << " ";

        aWriter << this->auto_increment << " " << this->primary_key <<
                " " << this->nullable;

        return StatusResult();
    }

    StatusResult Attribute::decode(std::istream &aReader) {
        std::string dataType;
        std::string len;
        aReader >> this->name >> dataType;
        this->type = stringToType(dataType);

        if(this->type == DataTypes::varchar_type) {
            aReader >> len;
            this->fieldLength = std::stoi(len);
        }

        aReader >> this->auto_increment >> this->primary_key >> this->nullable;

        return StatusResult();
    }

    StatusResult Attribute::initialize(std::string &aName, DataTypes aType,
                                       std::optional<size_t> aLen,
                                       bool aAuto_increment,
                                       bool aPrimary_key, bool aNullable) {
        this->name = aName;
        this->type = aType;
        this->fieldLength = aLen;
        this->auto_increment = aAuto_increment;
        this->primary_key = aPrimary_key;
        this->nullable = aNullable;

        return StatusResult();
    }

    void Attribute::describeDisplay(std::ostream &anOutput) {
        anOutput << "| " << std::left << std::setw(13) << this->name;
        std::string theType;

        if (this->type == DataTypes::varchar_type) {
            theType = typeToString(this->type) + "(" + std::to_string(*(this->fieldLength)) + ")";
        } else
            theType = typeToString(this->type);

        anOutput << "| " << std::left << std::setw(12) << theType;
        anOutput << "| " << std::left << std::setw(5) << (this->nullable ? "YES" : "NO");
        anOutput << "| " << std::left << std::setw(4) << (this->primary_key ? "YES" : "");
        anOutput << "| " << std::left << std::setw(8) << (this->nullable ? "NULL" : defaultVal());

        std::string theExtra;

        if (this->auto_increment)
            theExtra += "auto_increment ";
        if (this->primary_key)
            theExtra += "primary key";

        anOutput << "| " << std::left << std::setw(28) << theExtra;

        anOutput << "|\n";
    }

    bool Attribute::isNullable() {
        return this->nullable;
    }

    bool Attribute::isAuto() {
        return this->auto_increment;
    }

    bool Attribute::isPrimary() {
        return this->primary_key;
    }

    StatusResult Attribute::typeMatch(size_t anIndex) {
        return (this->type == indexToType(anIndex)) ? StatusResult() : StatusResult{Errors::invalidArguments};
    }

    std::string &Attribute::getName() {
        return this->name;
    }

    DataTypes Attribute::getType() {
        return this->type;
    }
}

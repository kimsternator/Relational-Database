//
//  Row.hpp
//  Assignment4
//
//  Created by rick gessner on 4/19/20.
//  Copyright © 2020 rick gessner. All rights reserved.
//

#ifndef Row_hpp
#define Row_hpp

#include <stdio.h>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <memory>

#include "Entity.hpp"

namespace ECE141 {
        using RowVisitor = std::function<bool(const std::string &, Value)>;

    class Database;

    class Row : public Storable {
    public:

        //STUDENT declare OCF methods...
        Row() {
            blockNumber = 0;
            refID = 0;
            rowNum = 1;
        }

        ~Row() = default;

        //storable api just in case...
        StatusResult encode(std::ostream &aWriter) override;
        StatusResult decode(std::istream &aReader) override;
        StatusResult decodeValue(std::istream &aReader, size_t &anIndex, Value &aValue);

        bool each(RowVisitor aCall) {
            for (auto thePair : this->data) {
                if (!aCall(thePair.first, thePair.second)) {
                    return false;
                }
            }

            return true;
        }

        void setName(std::string &aName);
        void setBlockNum(uint32_t aBlockNumber);
        void setRowNum(uint32_t aRowNumber);
        std::string getDecodedValue(std::string &aKey);
        Value getValue(std::string &aKey);
        StatusResult updateValue(std::string &aKey, Value &aValue);

        uint32_t getRefID();
        uint32_t getBlockNum();
        size_t numAttributes();
        KeyValues& getData();

        void addAttribute(std::pair<std::string, Value> &aPair);
        StatusResult addDefault(Entity* anEntity, Attribute &anAttribute);
        StatusResult addNull(Entity* anEntity, Attribute &anAttribute);

        bool loadValue(std::string &aName, Value &aValue);

    protected:
        KeyValues data;
        uint32_t blockNumber;
        uint32_t refID;
        uint32_t rowNum;
        //do you need any other data members?

    };

    //-------------------------------------------

    using RowCollection = std::vector<std::unique_ptr<Row>>;
    using RowPtr = std::unique_ptr<Row>;

}

#endif /* Row_hpp */

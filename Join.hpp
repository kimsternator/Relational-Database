//
//  Join.hpp
//  RGAssignment8
//
//  Created by rick gessner on 5/05/21.
//

#ifndef Join_h
#define Join_h

#include <string>
#include <vector>
#include "BasicTypes.hpp"
#include "Errors.hpp"
#include "keywords.hpp"


namespace ECE141 {
    struct TableField {
    public:
        TableField() {}

        TableField(const TableField &aCopy) {
            this->table = aCopy.table;
            this->attribute = aCopy.attribute;
        }

        std::string table;
        std::string attribute;
    };

    class Join {
    public:

        Join() {
            joinType = Keywords::unknown_kw;
        }

        Join(const std::string &aTable, Keywords aType)
                : table(aTable), joinType(aType), lhs(), rhs() {}

        StatusResult validateJoin(std::string &aName) {
            StatusResult theResult = StatusResult();

            if(this->table != "") {
                if(this->table == this->lhs.table && aName == this->rhs.table ||
                        this->table == this->rhs.table && aName == this->lhs.table) {
                    theResult = this->joinType != Keywords::unknown_kw ?
                            StatusResult() : StatusResult{Errors::invalidAttribute};
                }
                else {
                    theResult = StatusResult{Errors::invalidAttribute};
                }
            }

            return theResult;
        }

        StatusResult leftJoinFactory(RowCollection &aResult, RowCollection &aLeft, RowCollection &aRight) {
            for(auto& theLeftRow: aLeft) {
                size_t theCount = 0;

                for(auto& theRightRow: aRight) {
                    if(matchOn(theLeftRow, theRightRow)) {
                        StatusResult theResult = combineRows(aResult, theLeftRow, theRightRow);
                        theCount++;

                        if(theResult != StatusResult()) {
                            return theResult;
                        }
                    }
                }

                if(theCount == 0) {
                    StatusResult theResult = addNull(aResult, theLeftRow, aRight[0]);

                    if(theResult != StatusResult()) {
                        return theResult;
                    }
                }
            }

            return StatusResult();
        }

        StatusResult rightJoinFactory(RowCollection &aResult, RowCollection &aLeft, RowCollection &aRight) {
            TableField temp = this->lhs;
            this->lhs = this->rhs;
            this->rhs = temp;

            return leftJoinFactory(aResult, aRight, aLeft);
        }

//        using JoinFactory = StatusResult (Join::*)(RowCollection &, RowCollection &, RowCollection &);

        StatusResult joinRows(RowCollection &aRows, RowCollection &aLeftRows, RowCollection &aRightRows) {
//            static std::unordered_map<Keywords, JoinFactory> joinFactories = {
//                    {Keywords::left_kw, leftJoinFactory},
//                    {Keywords::right_kw, rightJoinFactory},
//            };

            if(this->joinType == Keywords::left_kw) {
                return leftJoinFactory(aRows, aLeftRows, aRightRows);
            }
            else if(this->joinType == Keywords::right_kw) {
                return rightJoinFactory(aRows, aLeftRows, aRightRows);
            }

            return StatusResult{Errors::unknownError};

//            return joinFactories[this->joinType](aRows, aLeftRows, aRightRows);
        }

        //combine the two rows and append to the collection
        StatusResult combineRows(RowCollection &aRows, RowPtr &aLeftRow, RowPtr &aRightRow) {
            RowPtr theRow{new Row()};

            aLeftRow->each([&](const std::string &anAttr, const Value &aValue) {
                std::pair<std::string, Value> thePair = {anAttr, aValue};
                theRow->addAttribute(thePair);

                return true;
            });

            aRightRow->each([&](const std::string &anAttr, const Value &aValue) {
                std::pair<std::string, Value> thePair = {anAttr, aValue};
                theRow->addAttribute(thePair);

                return true;
            });

            aRows.emplace_back(std::move(theRow));

            return StatusResult();
        }

        bool matchOn(RowPtr &aLeft, RowPtr &aRight) {
            return aLeft->getValue(this->lhs.attribute) == aRight->getValue(this->rhs.attribute);
        }

        StatusResult addNull(RowCollection &aRows, RowPtr &aLeftRow, RowPtr &aRightRow) {
            RowPtr theRow{new Row()};
            Value theNullValue = "NULL";

            aLeftRow->each([&](const std::string &anAttr, const Value &aValue) {
                std::pair<std::string, Value> thePair = {anAttr, aValue};
                theRow->addAttribute(thePair);

                return true;
            });

            aRightRow->each([&](const std::string &anAttr, const Value &aValue) {
                std::pair<std::string, Value> thePair = {anAttr, theNullValue};
                theRow->addAttribute(thePair);

                return true;
            });

            aRows.emplace_back(std::move(theRow));

            return StatusResult();
        }

        std::string table;
        Keywords joinType;
        TableField lhs;
        TableField rhs;
    };

    using JoinList = std::vector<Join>;

}

#endif /* Join_h */

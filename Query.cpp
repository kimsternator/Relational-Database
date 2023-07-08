//
//  Query.cpp
//  Assignment5
//
//  Created by rick gessner on 4/26/21.
//

#include "Query.hpp"
#include "Compare.hpp"

namespace ECE141 {

    Query::Query() {
        _from = nullptr;
        all = false;
        offset = 0;
        limit = 0;
    }
    //implement your query class here...
    bool Query::selectAll() const {
        return this->all;
    }

    Query & Query::setSelectAll(bool aState) {
        this->all = aState;

        return *this;
    }

    Query & Query::setSelect(const StringList &aFields) {
        for(auto field: aFields)
            this->selects.push_back(field);

        return *this;
    }

    void Query::setAll() {
        for(auto attr: this->_from->getAttributes())
            this->selects.push_back(attr.getName());
    }
    bool Query::getAll() {
        return this->all;
    }

    StringList Query::getSelects() const {
        return this->selects;
    }

    Query & Query::setFrom(Entity *anEntity) {
        this->_from = anEntity;

        return *this;
    }

    StatusResult Query::validateQuery() {
        // validate selects
        auto attrs = this->_from->getAttributes();
        if (!getAll()) {
            for(auto it = this->selects.begin(); it != this->selects.end(); it++) {
                auto j = attrs.begin();

                while(j != attrs.end()) {
                    if(j->getName() == *it) {
                        break;
                    }

                    j++;
                }

                if(j == attrs.end()) {
                    return StatusResult{Errors::invalidArguments};
                }
            }
        }

        //validate orders
        for(auto it = this->orders.begin(); it != this->orders.end(); it++) {
            auto j = attrs.begin();

            while(j != attrs.end()) {
                if(j->getName() == *it)
                    break;

                j++;
            }

            if(j == attrs.end()) {
                std::cout << "query here1\n";
                return StatusResult{Errors::invalidArguments};
            }
        }

        //validate updates
        for(int i = 0; i < this->updates.size(); i++) {
            if(this->updates[i].dtype != this->_from->getAttributeDatatype(this->updates[i].name)) {
                std::cout << "query here2\n";
                return StatusResult{Errors::invalidArguments};
            }
        }

        return StatusResult();
    }

    //validate if attributes are present
    StatusResult Query::validateJoinQuery(Entity *aLeft, Entity *aRight) {
        auto attrsLeft = aLeft->getAttributes();
        auto attrsRight = aRight->getAttributes();

        if (!getAll()) {
            //validate table attributes present in left or right
            for(auto it = this->selects.begin(); it != this->selects.end(); it++) {
                auto j = attrsLeft.begin();
                auto k = attrsRight.begin();

                while(j != attrsLeft.end() && k != attrsRight.end()) {
                    if(j->getName() == *it || k->getName() == *it) {
                        break;
                    }

                    if(j != attrsLeft.end()) {
                        j++;
                    }

                    if(k != attrsRight.end()) {
                        k++;
                    }
                }

                if(j == attrsLeft.end() && k == attrsRight.end()) {
                    return StatusResult{Errors::invalidArguments};
                }
            }
        }

        return StatusResult();
    }

    StatusResult Query::sortRows(RowCollection &aRows){
        if(!this->orders.empty()) {
            std::sort(aRows.begin(), aRows.end(), [&](RowPtr &a, RowPtr &b) {
                bool done, theResult = false;

                std::visit([&](const auto &x) {
                    std::visit([&](const auto &y) {
                        if (!done) {
                            if (isEqual(x, y) && this->orders.size() > 1)
                                theResult = compareNext(1, a, b);
                            else
                                theResult = isGreaterThan(x, y);

                            done = true;
                        }

                        return true;
                    }, a->getValue(this->orders[0]));

                    return true;
                }, b->getValue(this->orders[0]));

                return theResult;
            });
        }

        return StatusResult();
    }

    StatusResult Query::updateRows(RowCollection &aRows) {
        for(int i = 0; i < aRows.size(); i++) {
            for(auto it = this->updates.begin(); it != this->updates.end(); it++) {
                if(aRows[i]->updateValue(it->name, it->value) != StatusResult())
                    return StatusResult{Errors::writeError};
            }
        }

        return StatusResult();
    }

    StatusResult Query::addOrder(std::string &anOrder) {
        this->orders.push_back(anOrder);

        return StatusResult();
    }

    StatusResult Query::addUpdate(Operand &anOperand) {
        this->updates.emplace_back(anOperand);

        return StatusResult();
    }

    StatusResult Query::setLimit(const int aLimit) {
        this->limit = aLimit;

        return StatusResult();
    }

    int Query::getLimit() {
        return this->limit;
    }

    int Query::getOffset() {
        return this->offset;
    }

    bool Query::compareNext(int orderIndex, RowPtr &aLeft, RowPtr &aRight) {
        bool done, theResult = false;

        std::visit([&](const auto &x) {
            std::visit([&](const auto &y) {
                if(!done) {
                    if(isEqual(x, y) && this->orders.size() > orderIndex)
                        theResult = compareNext(++orderIndex, aLeft, aRight);
                    else
                        theResult = isGreaterThan(x, y);

                    done = true;
                }

                return true;
            }, aLeft->getValue(this->orders[orderIndex]));

            return true;
        }, aRight->getValue(this->orders[orderIndex]));

        return theResult;
    }
}

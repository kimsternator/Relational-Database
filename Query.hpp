//
//  Query.hpp
//  Assignment5
//
//  Created by rick gessner on 4/26/21.
//

#ifndef Query_hpp
#define Query_hpp

#include <stdio.h>
#include <string>

#include "Row.hpp"
#include "Tokenizer.hpp"
#include "Filters.hpp"

namespace ECE141 {

    class Query {
    public:

        Query();

        //all handlers
        bool selectAll() const;
        Query &setSelectAll(bool aState);
        void setAll();
        bool getAll();

        Query &setSelect(const StringList &aFields);

        StringList getSelects() const;

        Query &setOffset(int anOffset);

        StatusResult sortRows(RowCollection &aRows);
        StatusResult updateRows(RowCollection &aRows);

        StatusResult validateQuery();
        StatusResult validateJoinQuery(Entity* aLeft, Entity* aRight);

        //getters and setters
        StatusResult addOrder(std::string &anOrder);
        StatusResult addUpdate(Operand &anOperand);
        StatusResult setLimit(const int aLimit);
        Query &setFrom(Entity *anEntity);
        int getLimit();
        int getOffset();


        bool compareNext(int orderIndex, RowPtr &aLeft, RowPtr &aRight);

    protected:
        Entity *_from;
        StringList selects;
        StringList orders;
        std::vector<Operand> updates;
        bool all;
        int offset;
        int limit;
    };
}

#endif /* Query_hpp */

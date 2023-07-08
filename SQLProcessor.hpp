//
//  SQLProcessor.hpp
//  RGAssignment3
//
//  Created by rick gessner on 4/1/21.
//

#ifndef SQLProcessor_hpp
#define SQLProcessor_hpp

#include <stdio.h>

#include "CmdProcessor.hpp"

#include "Entity.hpp"
#include "Row.hpp"

#include "Query.hpp"
#include "Filters.hpp"
#include "Join.hpp"

namespace ECE141 {
    class SQLProcessor : public CmdProcessor {
    public:

        //STUDENT: Declare OCF methods...
        SQLProcessor(CmdProcessor *theDB, std::ostream &anOutput);

        virtual ~SQLProcessor();

        StatusResult createTable(Entity *anEntity);
        StatusResult dropTable(const std::string &aName);
        StatusResult describeTable(const std::string &aName);
        StatusResult showTables();
        StatusResult insert(std::string &aName, std::vector<std::vector<Value>> &aValues,
                            StringList &anAttributes, RowCollection &aRows);
        StatusResult select(std::string &aName, RowCollection &aRows, Query* aQuery,
                                                        Filters* aFilter, Join* aJoin);
        StatusResult update(std::string &aName, RowCollection &aRows, Query* aQuery, Filters* aFilter);
        StatusResult deleteFunc(std::string &aName, RowCollection &aRows, Filters* aFilter);
        StatusResult showIndex(std::string &aTableName, StringList &anAttrs);
        StatusResult showIndexes();

        //create helpers
        StatusResult primaryCheck(Entity* anEntity);

        //insert helpers
        StatusResult validateInput(std::vector<std::string> &anAttribute,
                                   std::vector<std::vector<Value>> aValues);
        StatusResult createRows(RowCollection &aRows, StringList &anAttribute,
                                ValuesList &aValues);
        StatusResult saveRows(std::string &aTableName, RowCollection &aRows);
        StatusResult addAutoDefault(std::string &aName, RowCollection &aRows);

        //select helpers
        StatusResult joinSelect(std::string &aName, RowCollection &aRows,
                                Query *aQuery, Filters *aFilter, Join* aJoin);
        StatusResult selectRows(std::string &aName, RowCollection &aRows);
        StatusResult validateJoinTables(Join* aJoin);
        StatusResult validateTableAttr(std::string &aTableName, std::string &anAttrName);

        //delete helpers
        StatusResult validateTable(std::string &aTableName);

        //retrieve a table entity
        Entity *getEntity(std::string &aTableName);
        StatusResult updateRows(std::string &aTableName, RowCollection &aRows);

        //standard statement commands
        CmdProcessor *recognizes(Tokenizer &aTokenizer) override;
        Statement *makeStatement(Tokenizer &aTokenizer) override;
        StatusResult run(Statement *aStmt, const Timer &aTimer) override;
        Database *getDatabase() override;

    protected:
        CmdProcessor *dbproc;
    };

}

#endif /* SQLProcessor_hpp */

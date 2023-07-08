//
//  Database.hpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#ifndef Database_hpp
#define Database_hpp

#include <stdio.h>
#include <fstream>
#include "Storage.hpp"
#include "Row.hpp"
#include "Index.hpp"
#include "Helpers.hpp"

namespace ECE141 {

    class Database : public Storable {
    public:

        Database(const std::string aPath, CreateDB);
        Database(const std::string aPath, OpenDB);

        virtual ~Database();

        //High Level Commands
        StatusResult createTable(Entity *anEntity);
        StatusResult dumpDatabase(std::ostream &anOutput);
        size_t showTables(std::ostream &anOutput);
        size_t describeTable(const std::string &aName, std::ostream &anOutput);
        StatusResult dropTable(const std::string &aName, std::ostream &anOutput);
        size_t showTableIndexes(std::ostream &anOutput);
        size_t showTableIndex(std::string &aTableName, StringList &anAttrs, std::ostream &anOutput);

        //Storable Interface
        StatusResult encode(std::ostream &anOutput) override;
        StatusResult decode(std::istream &anInput) override;

        //header helpers
        StatusResult saveDBHeader(uint32_t aPos);
        StatusResult loadDBHeader();

        //entity helpers
        StatusResult saveEntity(Entity* anEntity);
        StatusResult updateEntity(Entity *anEntity);
        Entity* getEntity(std::string aTableName);
        Entity* getEntity(uint32_t aTableName);

        //block helpers
        StatusResult loadBlock(uint32_t aBlockNum, Block &aBlock);
        StatusResult deleteBlock(uint32_t aBlockNum);
        StatusResult loadObject(std::iostream &aStream, uint32_t aBlockNum);
        StatusResult deleteObject(uint32_t aBlockNum);

        //row helpers
        StatusResult saveRow(RowPtr &aRow);
        StatusResult saveRows(std::string &aTableName, RowCollection &aRows);
        StatusResult updateRow(RowPtr &aRow);
        StatusResult updateRows(std::string &aTableName, RowCollection &aRows);
        StatusResult deleteRows(std::string &aTableName, RowCollection &aRows);
        StatusResult deleteRow(RowPtr &aRow);
        StatusResult selectRows(std::string &aName, RowCollection &aRows);
        StatusResult validateRows(RowCollection &aRows);
        StatusResult compareAttributes(Entity &anEntity, RowPtr &aRow);
        StatusResult saveRowToIndex(Index* aTableIndex, RowPtr &aRow);
        StatusResult updateRowToIndex(Index* aTableIndex, RowPtr &aRow);
        StatusResult deleteRowToIndex(Index* aTableIndex, RowPtr &aRow);

        //index helpers
        StatusResult saveIndex(Index* anIndex);
        Index* loadIndex(uint32_t aPos);
        StatusResult updateIndex(Index* anIndex);
        Index* getTableIndex(std::string aTableName);
        Index* loadAttributeIndex(Index* aTableIndex, std::string anAttribute);
        StatusResult saveTableIndex(std::string aTableName, uint32_t aPos);
        StatusResult addIndex(std::string aTableName, std::string anAttributeName);
        StatusResult addIndex(Index* aTable, std::string anAttributeName);
        Index* getPrimaryKeyIndex(std::string aTableName, Index* aTable);

        Index* getTablesIndex();
        StatusResult removeTable(std::string aTableName);
        StatusResult saveTablesIndex();
        StatusResult loadTablesIndex();

        StatusResult findAttr(Index* theTableIndex, std::string &anAttr);
        StatusResult validateShowIndex(std::string &aTableName, StringList &anAttrs);

        //database getters
        uint32_t getRows();
        uint32_t getDataRows();
        uint32_t getTableCount();
        std::string getDBName();
        uint32_t getFreeBlock();

    protected:

        std::string name;
        Storage storage;
        bool changed;
        std::fstream stream; //stream storage uses for IO
        Index* tablesIndex; //big index for database
    };

}
#endif /* Database_hpp */

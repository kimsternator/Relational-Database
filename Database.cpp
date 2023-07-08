//
//  Database.cpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#include <string>
#include <iostream>
#include <map>
#include "Storage.hpp"
#include "Database.hpp"
#include "Config.hpp"

namespace ECE141 {

    static std::string typeToString(BlockType aType) {
        static std::map<BlockType, std::string> types = {
                {BlockType::meta_block,    "Meta"},
                {BlockType::data_block,    "Data"},
                {BlockType::entity_block,  "Entity"},
                {BlockType::free_block,    "Free"},
                {BlockType::index_block,   "Index"},
                {BlockType::unknown_block, "Unknown"},
        };

        return types[aType];
    }

    // Create Database
    Database::Database(const std::string aName, CreateDB)
            : name(aName), storage(stream), changed(true) {
        std::string thePath = Config::getDBPath(name);
        stream.clear(); // Clear Flag, then create file...
        stream.open(thePath.c_str(), std::fstream::binary | std::fstream::in | std::fstream::out | std::fstream::trunc);
        stream.close();
        stream.open(thePath.c_str(), std::fstream::binary |
                                     std::fstream::in | std::fstream::out);

        //STUDENT: Write meta info to block 0...
        //write here
        //save the header block
        StatusResult theResult = saveDBHeader(0);
        stream.close();
        //save the database index
        if(theResult == StatusResult()) {
            theResult = saveTablesIndex();
        }

        if(theResult != StatusResult()) {
            std::cerr << "Database creation error" << std::endl;
        }
    }

    // Open Database
    Database::Database(const std::string aName, OpenDB)
            : name(aName), changed(false), storage(stream) {
        std::string thePath = Config::getDBPath(name);
        stream.open(thePath.c_str(), std::fstream::binary
                                     | std::fstream::in | std::fstream::out);
        //STUDENT: Load meta info from block 0
        loadDBHeader();
        stream.close();
        //load entity of entity index into a data member for database.cpp
        loadTablesIndex();
    }

    // Saving Entity into file (here because making a new entity for a new table)
    StatusResult Database::createTable(Entity *anEntity) {
        if(anEntity) {
            StatusResult theResult = saveEntity(anEntity);

            //save the table index
            if(theResult == StatusResult()) {
                theResult = saveTableIndex(anEntity->getName(), anEntity->getBlockNum());
            }
            //save the primary key index
            if(theResult == StatusResult()) {
                theResult = addIndex(anEntity->getName(), anEntity->getPrimaryKeyName());
            }

            return theResult;
        }

        return StatusResult{Errors::unknownEntity};
    }

    // Dump Database into a View for Debug
    StatusResult Database::dumpDatabase(std::ostream &anOutput) {
        std::string thePath = Config::getDBPath(this->name);
        this->stream.open(thePath.c_str(), std::fstream::binary
                                     | std::fstream::in | std::fstream::out);
        std::string theBar = "+----+----------+------------+------+\n";
        anOutput << theBar;
        anOutput << "| N  | Type     | refID      | next |\n";
        anOutput << theBar;
        size_t count = 0;

        this->storage.each([&](const Block &aBlock, uint32_t aBlockNumber) {
            anOutput << "| " << std::left << std::setw(3) << aBlockNumber <<
            "| " << std::left << std::setw(9) << typeToString((BlockType) aBlock.header.type) <<
            "| " << std::left << std::setw(11) << aBlock.header.refId << "| " <<
            std::left << std::setw(5) << aBlock.header.next << "|\n";
            anOutput << theBar;
            count++;

            return StatusResult();
        });

        anOutput << count << " rows in set ";
        this->stream.close();

        return StatusResult();
    }

    // Show tables in a view
    size_t Database::showTables(std::ostream &anOutput) {
        std::string theBar = "+----------------------+\n";
        anOutput << theBar;
        anOutput << "| Tables               |\n";
        anOutput << theBar;
        size_t count = 0;

        if(this->tablesIndex) {
            std::string thePath = Config::getDBPath(this->name);
            this->stream.open(thePath.c_str(), std::fstream::binary
                                               | std::fstream::in | std::fstream::out);
            this->tablesIndex->eachKV([&](const IndexKey &aKey, const uint32_t aBlockNum) {
                if(aKey.index() == 1 && std::get<std::string>(aKey) != "Header") {
                    std::visit([&](const auto &aValue) {
                        anOutput << "| " << std::left << std::setw(21) << aValue << "|\n";
                        count++;
                    }, aKey);
                }

               return true;
            });

            if (count > 0) {
                anOutput << theBar;
            }

            this->stream.close();

            return count;
        }

        return 0;
    }

    //execute describe table command
    size_t Database::describeTable(const std::string &aName, std::ostream &anOutput) {
        std::string theBar = "+--------------+-------------+------+-----+---------+-----------------------------+\n";
        anOutput << theBar;
        anOutput << "| Field        | Type        | Null | Key | Default | Extra                       |\n";
        anOutput << theBar;
        size_t count;
        Entity *theTableEntity = getEntity(aName);

        if(theTableEntity) {
            count = theTableEntity->describeDisplay(anOutput);
            anOutput << theBar;

            return count;
        }

        return 0;
    }

    //execute drop table in the blocks
    StatusResult Database::dropTable(const std::string &aName, std::ostream &anOutput) {
        Index* theTable = getTableIndex(aName);

        if(theTable) {
            theTable->eachKV([&](const IndexKey &aKey, const uint32_t aBlockNum) {
                //mark the entity as free
                if(std::get<std::string>(aKey) == theTable->getName()) {
                    //removes the table from the TablesIndex
                    StatusResult theResult = removeTable(aName);

                    if (theResult == StatusResult()) {
                        //removes the entity block
                        theResult = deleteObject(aBlockNum);
                    }

                    return theResult == StatusResult();
                }
                else {
                    //iterate through each attribute index and delete the rows
                    Index* theAttributeIndex = loadIndex(aBlockNum);

                    if(theAttributeIndex) {
                        std::vector<uint32_t> theRemoved;
                        StatusResult theResult = StatusResult();

                        theAttributeIndex->eachKV([&](const IndexKey &anAttributeBlock,
                                                      const uint32_t &aRowNum) {
                            // don't want to remove the table quite yet
                            if(aRowNum != aBlockNum) {
                                if (theRemoved.empty() || find(theRemoved.begin(), theRemoved.end(), aRowNum)
                                                          == theRemoved.end()) {
                                    theResult = deleteObject(aRowNum);
                                    theRemoved.push_back(aRowNum);
                                }
                            }

                            return theResult == StatusResult();
                        });
                        //remove the attribute Index
                        theResult = deleteObject(aBlockNum);

                        delete theAttributeIndex;

                        return theResult == StatusResult();
                    }

                    delete theAttributeIndex;

                    return false;
                }
            });

            //delete theTableIndex
            if(deleteObject(theTable->getBlockNum()) == StatusResult()) {
                //update the Tables index
                if(updateIndex(this->tablesIndex) == StatusResult()) {
                    delete theTable;

                    return StatusResult();
                }
            }

            delete theTable;

            return StatusResult{writeError};
        }

        return StatusResult{Errors::unknownTable};
    }

    size_t Database::showTableIndexes(std::ostream &anOutput) {
        std::string theBar = "+----------------+------------------------+\n";
        anOutput << theBar;
        anOutput << "| Tables         | field(s)               |\n";
        anOutput << theBar;
        size_t count = 0;

        if(this->tablesIndex) {
            this->tablesIndex->eachKV([&](const IndexKey &aKey, const uint32_t aBlockNum) {
                //skip over the header
                if(std::get<std::string>(aKey) != "Header") {
                    Index *theTableIndex = loadIndex(aBlockNum);

                    if(theTableIndex) {
                        StringList theFields;
                        count++;

                        theTableIndex->eachKV([&](const IndexKey &aKey, const uint32_t aBlockNum) {
                            if (std::get<std::string>(aKey) != theTableIndex->getName()) {
                                theFields.push_back(std::get<std::string>(aKey));
                            }

                            return true;
                        });

                        anOutput << "| " << std::left << std::setw(15) << theTableIndex->getName() <<
                                 "| " << std::left << std::setw(23) << Helpers::join(theFields) <<
                                 "|\n" << theBar;

                        delete theTableIndex;

                        return true;
                    }

                    return false;
                }

                return true;
            });

            return count;
        }

        return 0;
    }

    size_t Database::showTableIndex(std::string &aTableName, StringList &anAttrs,
                                                        std::ostream &anOutput) {
        Index* theTable = getTableIndex(aTableName);
        std::string theBar = "+----------------+----------------+--------+\n";
        anOutput << theBar;
        anOutput << "| Table          | Key            | Block  |\n";
        anOutput << theBar;
        size_t theRows = 0;

        if(theTable) {
            for(auto attr: anAttrs) {
                IndexKey theKey = attr;
                IntList theBlockNum = theTable->valueAt(theKey);

                if(theBlockNum.size() == 1) {
                    Index *theAttrIndex = loadIndex(theBlockNum[0]);

                    if (theAttrIndex) {
                        theAttrIndex->eachKV([&](const IndexKey &aKey, const uint32_t aBlockNum) {
                            theRows++;
                            anOutput << "| " << std::left << std::setw(15) << theTable->getName() <<
                                     "| " << std::left << std::setw(15) << std::get<std::string>(aKey) <<
                                     "| " << std::left << std::setw(7) << aBlockNum << "|\n" << theBar;

                            return true;
                        });
                    }
                }
            }
        }

        return theRows;
    }

    Database::~Database() {
        if (this->changed) {
            //save db meta information to block 0?
            //save anything else in memory that changed...
        }

        if(this->stream.is_open()) {
            this->stream.close();
        }
    }

    // encoding data into files
    StatusResult Database::encode(std::ostream &anOutput) {
        anOutput << this->name << " " << this->changed;

        return StatusResult{Errors::noError};
    }

    // decoding files into data
    StatusResult Database::decode(std::istream &anInput) {
        anInput >> this->name >> this->changed;

        return StatusResult{Errors::noError};
    }

    // saving Database header for use
    StatusResult Database::saveDBHeader(uint32_t aPos) {
        std::stringstream theHeader;
        encode(theHeader);
        std::string theStr = theHeader.str();
        StorageInfo theInfo(0, theStr.size(), aPos, BlockType::meta_block);
        std::stringstream theOutput(theStr);

        return this->storage.save(theOutput, theInfo);
    }

    // loading Database header
    StatusResult Database::loadDBHeader() {
        std::stringstream theInput;
        StatusResult theResult = this->storage.load(theInput, 0);
        decode(theInput);

        return theResult;
    }

    StatusResult Database::saveEntity(Entity *anEntity) {
        uint32_t thePos = getFreeBlock();
        anEntity->addBlockNum(thePos);
        anEntity->setRefID();

        return updateEntity(anEntity);
    }

    //update an entity given a changed entity
    StatusResult Database::updateEntity(Entity *anEntity) {
        if(anEntity) {
            std::string thePath = Config::getDBPath(this->name);
            this->stream.open(thePath.c_str(), std::fstream::binary |
                                         std::fstream::in | std::fstream::out);
            //encoding
            std::stringstream theData;
            anEntity->encode(theData);
            std::string theStr = theData.str();
            StorageInfo theInfo(anEntity->getRefID(), theStr.size(), anEntity->getBlockNum(),
                                                                BlockType::entity_block);
            std::stringstream theOutput(theStr);
            //saving
            StatusResult theResult = this->storage.save(theOutput, theInfo);
            this->stream.close();

            return theResult;
        }

        return StatusResult{Errors::badPointer};
    }

    //load a block into memory
    StatusResult Database::loadBlock(uint32_t aBlockNum, Block &aBlock) {
        return this->storage.readBlock(aBlockNum, aBlock);
    }

    StatusResult Database::deleteBlock(uint32_t aBlockNum) {
        return this->storage.markBlockAsFree(aBlockNum);
    }

    StatusResult Database::loadObject(std::iostream &aStream, uint32_t aBlockNum) {
        std::string thePath = Config::getDBPath(this->name);
        this->stream.open(thePath.c_str(), std::fstream::binary |
                                           std::fstream::in | std::fstream::out);
        StatusResult theResult = this->storage.load(aStream, aBlockNum);
        this->stream.close();

        return theResult;
    }

    StatusResult Database::deleteObject(uint32_t aBlockNum) {
        std::string thePath = Config::getDBPath(this->name);
        this->stream.open(thePath.c_str(), std::fstream::binary |
                                           std::fstream::in | std::fstream::out);
        Block theBlock;
        StatusResult theResult = loadBlock(aBlockNum, theBlock);

        if(theResult == StatusResult()) {
            theResult = deleteBlock(aBlockNum);

            if(theResult == StatusResult()) {
                uint32_t theNext = theBlock.header.next;

                while (theResult == StatusResult() && theNext != 0) {
                    Block theNextBlock;
                    theResult = loadBlock(aBlockNum, theNextBlock);

                    if(theResult == StatusResult()) {
                        theResult = deleteBlock(theNext);
                        theNext = theNextBlock.header.next;
                    }
                }
            }
        }

        this->stream.close();

        return theResult;
    }

    // Saving row into block
    StatusResult Database::saveRow(RowPtr &aRow) {
        if(aRow) {
            uint32_t thePos = getFreeBlock();
            aRow->setBlockNum(thePos);

            return updateRow(aRow);
        }

        return StatusResult{Errors::badPointer};
    }

    // Saving the row collection
    StatusResult Database::saveRows(std::string &aTableName, RowCollection &aRows) {
        StatusResult theResult = validateRows(aRows);
        Index* theTableIndex = getTableIndex(aTableName);

        if(theTableIndex) {
            if (theResult == StatusResult()) {
                for (auto & aRow : aRows) {
                    theResult = saveRow(aRow);

                    if (theResult == StatusResult()) {
                        theResult = saveRowToIndex(theTableIndex, aRow);
                    }

                    if (theResult != StatusResult()) {
                        return theResult;
                    }
                }
            }

            theResult = updateIndex(theTableIndex);
        }
        else {
            return StatusResult{Errors::unknownTable};
        }

        return theResult;
    }

    //updating the row in storage
    StatusResult Database::updateRow(RowPtr &aRow) {
        if(aRow) {
            std::string thePath = Config::getDBPath(this->name);
            this->stream.open(thePath.c_str(), std::fstream::binary |
                                         std::fstream::in | std::fstream::out);
            std::stringstream theData;
            aRow->encode(theData);
            std::string theStr = theData.str();
            StorageInfo theInfo(aRow->getRefID(), theStr.size(), aRow->getBlockNum(), BlockType::data_block);
            std::stringstream theOutput(theStr);

            StatusResult theResult = this->storage.save(theOutput, theInfo);
            this->stream.close();

            return theResult;
        }

        return StatusResult{Errors::badPointer};
    }

    // updating row collection
    StatusResult Database::updateRows(std::string &aTableName, RowCollection &aRows) {
        StatusResult theResult = StatusResult();
        Index* theTableIndex = getTableIndex(aTableName);

        if(theTableIndex) {
            for(auto & aRow : aRows) {
                theResult = updateRow(aRow);

                if(theResult == StatusResult()) {
                    theResult = updateRowToIndex(theTableIndex, aRow);
                }

                if(theResult != StatusResult()) {
                    break;
                }
            }
        }

        return theResult;
    }

    //delete a row in memory
    StatusResult Database::deleteRow(RowPtr &aRow) {
        if(aRow) {
            return deleteObject(aRow->getBlockNum());
        }

        return StatusResult{Errors::badPointer};
    }

    //delete the rows
    StatusResult Database::deleteRows(std::string &aTableName, RowCollection &aRows){
        StatusResult theResult = StatusResult();
        Index* theTableIndex = getTableIndex(aTableName);

        if(theTableIndex) {
            for (auto &aRow : aRows) {
                theResult = deleteRow(aRow);

                if (theResult == StatusResult()) {
                    theResult = deleteRowToIndex(theTableIndex, aRow);
                }

                if (theResult != StatusResult()) {
                    break;
                }
            }
        }

        return theResult;
    }

    // Grabbing rows from file system
    StatusResult Database::selectRows(std::string &aName, RowCollection &aRows) {
        Index* theTableIndex = getTableIndex(aName);

        if(theTableIndex) {
            Index* thePrimaryIndex = getPrimaryKeyIndex(aName, theTableIndex);
            StatusResult theResult = StatusResult();

            if(thePrimaryIndex) {
                thePrimaryIndex->eachKV([&](const IndexKey &aKey, const uint32_t aBlockNum) {
                    RowPtr theRow{new Row()};
                    std::stringstream theStream;
                    theResult = loadObject(theStream, aBlockNum);
                    theRow->decode(theStream);
                    aRows.emplace_back(std::move(theRow));

                    return theResult == StatusResult();
                });

                return theResult;
            }
            else {
                return StatusResult{Errors::cantLoadIndex};
            }
        }

        return StatusResult{Errors::unknownTable};
    }

    //validate rows against it's table entity
    StatusResult Database::validateRows(RowCollection &aRows) {
        StatusResult theResult;
        Entity* theEntity = getEntity(aRows[0]->getRefID());

        if (theEntity) {
            for (int i = 0; i < aRows.size(); i++) {
                theResult = compareAttributes(*theEntity, aRows[i]);

                if (theResult != StatusResult())
                    return theResult;
                //now that a row is validated set it's dedicated row num
                aRows[0]->setRowNum(theEntity->useRowNum());
            }
        }

        updateEntity(theEntity);

        return theResult;
    }

    //compare the attributes in number and if any of the required attributes are there
    StatusResult Database::compareAttributes(Entity &anEntity, RowPtr &aRow) {
        if(aRow) {
            StatusResult theResult;
            AttributeList theAttributes = anEntity.getAttributes();

            // see if there are extra values
            theResult = (aRow->numAttributes() > theAttributes.size()) ?
                        StatusResult{Errors::invalidArguments} : StatusResult();

            // see if all the attributes are present
            if (theResult == StatusResult()) {
                for (auto attribute: theAttributes) {
                    Value theValue;

                    if (!aRow->loadValue(attribute.getName(), theValue)) {
                        if (!attribute.isNullable() && !attribute.isAuto()) {
                            theResult = StatusResult{Errors::invalidArguments};
                        }
                    } else {
                        theResult = attribute.typeMatch(theValue.index());
                    }

                    if (theResult != StatusResult()) {
                        return theResult;
                    }
                }
            }

            return theResult;
        }

        return StatusResult{Errors::unknownError};
    }

    //save row into index(s)
    StatusResult Database::saveRowToIndex(Index* aTableIndex, RowPtr &aRow) {
        if(aTableIndex) {
            aRow->each([&](const std::string &anAttr, const Value &aValue) {
                IndexKey theKey = anAttr;
                IntList theBlockNum = aTableIndex->valueAt(theKey);

                if(theBlockNum.size() == 0) {
                    StatusResult theResult = addIndex(aTableIndex, anAttr);

                    if(theResult != StatusResult()) {
                        return false;
                    }
                }

                Index* theAttrIndex = loadAttributeIndex(aTableIndex, anAttr);

                if(theAttrIndex) {
                    IndexKey theValue = Helpers::valueToString(aValue);
                    theAttrIndex->setKeyValue(theValue, aRow->getBlockNum());
                    StatusResult theResult = updateIndex(theAttrIndex);

                    delete theAttrIndex;

                    return theResult == StatusResult();
                }

                return false;
            });

            return StatusResult();
        }

        return StatusResult{Errors::badPointer};
    }

    //update the row in the index
    StatusResult Database::updateRowToIndex(Index *aTableIndex, RowPtr &aRow) {
        if(aTableIndex) {
            if(aRow) {
                aRow->each([&](const std::string &anAttr, const Value &aValue) {
                    Index *theAttrIndex = loadAttributeIndex(aTableIndex, anAttr);
                    //update the values in the attr index
                    if (theAttrIndex) {
                        //erase old value
                        IntOpt theBlockNum = aRow->getBlockNum();
                        IndexKey theOldKey = theAttrIndex->keyAt(theBlockNum);
                        theAttrIndex->erase(std::get<std::string>(theOldKey));
                        //add new value
                        IndexKey theValue = Helpers::valueToString(aValue);
                        theAttrIndex->setKeyValue(theValue, aRow->getBlockNum());
                        StatusResult theResult = updateIndex(theAttrIndex);

                        delete theAttrIndex;

                        return theResult == StatusResult();
                    }

                    return false;
                });

                return StatusResult();
            }
        }

        return StatusResult{Errors::unknownTable};
    }

    //delete a row from the index
    StatusResult Database::deleteRowToIndex(Index *aTableIndex, RowPtr &aRow) {
        if(aTableIndex) {
            if(aRow) {
                aRow->each([&](const std::string &anAttr, const Value &aValue) {
                    Index *theAttrIndex = loadAttributeIndex(aTableIndex, anAttr);
                    //erase the values in the attr index
                    if (theAttrIndex) {
                        IntOpt theBlockNum = aRow->getBlockNum();
                        std::string theRowKey = anAttr;
                        IndexKey theKey = Helpers::valueToString(aRow->getValue(theRowKey));
                        theAttrIndex->erase(std::get<std::string>(theKey));
                        StatusResult theResult = updateIndex(theAttrIndex);

                        delete theAttrIndex;

                        return theResult == StatusResult();
                    }

                    return false;
                });

                return StatusResult();
            }
        }

        return StatusResult{Errors::badPointer};
    }

    StatusResult Database::saveIndex(Index *anIndex) {
        std::string thePath = Config::getDBPath(this->name);
        this->stream.open(thePath.c_str(), std::fstream::binary
                                           | std::fstream::in | std::fstream::out);
        //encode the index
        std::stringstream theIndexStream;
        anIndex->encode(theIndexStream);
        std::string theStr = theIndexStream.str();
        std::stringstream theOutput(theStr);
        //save the index
        StorageInfo theInfo = anIndex->getStorageInfo(theStr.size());
        StatusResult theResult = this->storage.save(theOutput, theInfo);
        this->stream.close();

        return theResult;
    }

    Index * Database::loadIndex(uint32_t aPos) {
        std::stringstream theIndexObject;
        StatusResult theResult = loadObject(theIndexObject, aPos);

        if(theResult == StatusResult()) {
            Index *theIndexTable = new Index(this->name, this->storage);
            std::string theIndexStr(theIndexObject.str());
            std::stringstream theStream(theIndexStr);
            theIndexTable->decode(theStream);

            return theIndexTable;
        }

        return nullptr;
    }

    StatusResult Database::updateIndex(Index *anIndex) {
        if(anIndex) {
            std::string thePath = Config::getDBPath(this->name);
            this->stream.open(thePath.c_str(), std::fstream::binary
                                               | std::fstream::in | std::fstream::out);
            //encode the index
            std::stringstream theIndexStream;
            anIndex->encode(theIndexStream);
            std::string theStr = theIndexStream.str();
            std::stringstream theOutput(theStr);
            //save the index
            StorageInfo theInfo = anIndex->getStorageInfo(theStr.size());
            StatusResult theResult = this->storage.save(theOutput, theInfo);
            this->stream.close();

            return theResult;
        }

        return StatusResult{Errors::badPointer};
    }

    Index * Database::getTableIndex(std::string aTableName) {
        if(!this->tablesIndex) {
            loadTablesIndex();
        }

        IndexKey theKey = aTableName;
        IntList theVal = this->tablesIndex->valueAt(theKey);

        if(theVal.size() == 1) {
            return loadIndex(theVal[0]);
        }

        return nullptr;
    }

    Index* Database::loadAttributeIndex(Index* aTableIndex, std::string anAttribute) {
        if(aTableIndex) {
            IndexKey theKey = anAttribute;
            IntList theBlockNum = aTableIndex->valueAt(theKey);

            if(theBlockNum.size() == 1) {
                return loadIndex(theBlockNum[0]);
            }
        }

        return nullptr;
    }

    StatusResult Database::saveTableIndex(std::string aTableName, uint32_t aPos) {
        uint32_t thePos = getFreeBlock();
        Index* theTableIndex = new Index(aTableName, this->storage, thePos, IndexType::strKey);
        theTableIndex->setTableId(aTableName);
        IndexKey theEntity = aTableName;
        theTableIndex->setKeyValue(theEntity, aPos);
        StatusResult theResult = saveIndex(theTableIndex);
        //save the index into the TablesIndex
        if(theResult == StatusResult()) {
            IndexKey theTableName = aTableName;

            if(this->tablesIndex) {
                theResult = this->tablesIndex->setKeyValue(theTableName, thePos) ?
                            StatusResult() : StatusResult{Errors::unknownError};
            }
            else {
                theResult = StatusResult{Errors::cantLoadIndex};
            }
        }
        //update the tablesIndex
        if(theResult == StatusResult()) {
            theResult = saveIndex(this->tablesIndex);
        }

        delete theTableIndex;

        return theResult;
    }

    StatusResult Database::addIndex(std::string aTableName, std::string anAttributeName) {
        uint32_t thePos = getFreeBlock();
        Index* theIndex = new Index(anAttributeName, this->storage, thePos, IndexType::strKey);
        theIndex->setTableId(aTableName);
        StatusResult theResult = saveIndex(theIndex);
        // save the index into the specified table index
        if(theResult == StatusResult()) {
            Index* theTableIndex = getTableIndex(aTableName);

            if(theTableIndex) {
                IndexKey theAttributeName = anAttributeName;
                theTableIndex->setKeyValue(theAttributeName, thePos);
                theResult = updateIndex(theTableIndex);
            }
            else {
                theResult = StatusResult{Errors::unknownTable};
            }

            delete theTableIndex;
        }

        delete theIndex;

        return theResult;
    }

    StatusResult Database::addIndex(Index *aTable, std::string anAttributeName) {
        uint32_t thePos = getFreeBlock();
        Index* theIndex = new Index(anAttributeName, this->storage, thePos, IndexType::strKey);
        theIndex->setTableId(aTable->getName());
        StatusResult theResult = saveIndex(theIndex);
        // save the index into the specified table index
        if(theResult == StatusResult()) {
            if(aTable) {
                IndexKey theAttributeName = anAttributeName;
                aTable->setKeyValue(theAttributeName, thePos);
                theResult = updateIndex(aTable);
            }
            else {
                theResult = StatusResult{Errors::unknownTable};
            }
        }

        delete theIndex;

        return theResult;
    }

    Index * Database::getPrimaryKeyIndex(std::string aTableName, Index* aTable) {
        if(aTable) {
            Entity* theTableEntity = getEntity(aTableName);

            if(theTableEntity) {
                IndexKey theKey = theTableEntity->getPrimaryKeyName();
                IntList theBlockNum = aTable->valueAt(theKey);

                if(theBlockNum.size() == 1) {
                    return loadIndex(theBlockNum[0]);
                }
            }
        }

        return nullptr;
    }

    Entity * Database::getEntity(std::string aTableName) {
        Index* theTableIndex = getTableIndex(aTableName);

        if(theTableIndex) {
            IndexKey theKey = aTableName;
            IntList theBlockNum = theTableIndex->valueAt(theKey);

            if(theBlockNum.size() == 1) {
                std::stringstream theEntityStream;

                if(loadObject(theEntityStream, theBlockNum[0]) == StatusResult()) {
                    Entity *theEntity = new Entity(aTableName);
                    std::string theEntityStr(theEntityStream.str());
                    std::stringstream theStream(theEntityStr);
                    theEntity->decode(theStream);

                    return theEntity;
                }
            }
        }

        return nullptr;
    }

    Entity * Database::getEntity(uint32_t aTableName) {
        std::string theName;

        this->tablesIndex->eachKV([&](const IndexKey &aKey, const uint32_t &aBlockNum) {
            if(aKey.index() == 1) {
                if(Entity::hashString(std::get<std::string>(aKey)) == aTableName) {
                    theName = std::get<std::string>(aKey);

                    return false;
                }
            }

            return true;
        });

        return !theName.empty() ? getEntity(theName) : nullptr;
    }

    Index * Database::getTablesIndex() {
        return loadIndex(1);
    }

    StatusResult Database::removeTable(std::string aTableName) {
        if(!this->tablesIndex) {
            loadTablesIndex();
        }

        IndexKey theKey = aTableName;

        if(this->tablesIndex->exists(theKey)) {
            return this->tablesIndex->erase(aTableName);
        }

        return StatusResult{Errors::unknownTable};
    }

    // save Big index per database (storage, blocknum, strkey)
    // big index is the index of indexes (ie the index of tables)
    StatusResult Database::saveTablesIndex() {
        //create the index
        uint32_t thePos = getFreeBlock();
        std::string theIndexName = this->name;
        Index* theBigIndex = new Index(theIndexName, this->storage, thePos, IndexType::strKey);
        IndexKey theHeader = "Header";
        theBigIndex->setKeyValue(theHeader, 0);
        StatusResult theResult = saveIndex(theBigIndex);

        delete theBigIndex;

        return theResult;
    }

    //load the tables index
    StatusResult Database::loadTablesIndex() {
        if ((this->tablesIndex = getTablesIndex())) {
            return StatusResult();
        }

        return StatusResult{Errors::cantLoadIndex};
    }

    StatusResult Database::findAttr(Index *theTableIndex, std::string &anAttr) {
        if(theTableIndex) {
            IndexKey theAttr = anAttr;

            if(!theTableIndex->valueAt(theAttr).empty()) {
                return StatusResult();
            }

            return StatusResult{Errors::invalidArguments};
        }

        return StatusResult{Errors::badPointer};
    }

    StatusResult Database::validateShowIndex(std::string &aTableName, StringList &anAttrs) {
        Index* theTableIndex = getTableIndex(aTableName);

        if(theTableIndex) {
            StatusResult theResult = StatusResult();

            for(auto attr: anAttrs) {
                theResult = findAttr(theTableIndex, attr);

                if(theResult != StatusResult()) {
                    break;
                }
            }

            return theResult;
        }

        return StatusResult{Errors::unknownTable};
    }

    //get the number of blocks of this database
    uint32_t Database::getRows() {
        std::string thePath = Config::getDBPath(this->name);
        stream.open(thePath.c_str(), std::fstream::binary |
                                     std::fstream::in | std::fstream::out);
        uint32_t rows = this->storage.getBlockCount();
        stream.close();

        return rows;
    }

    //get the number of datablocks of this database
    uint32_t Database::getDataRows() {
        std::string thePath = Config::getDBPath(this->name);
        stream.open(thePath.c_str(), std::fstream::binary |
                                     std::fstream::in | std::fstream::out);
        uint32_t theRows = 0;
        // visitor pattern to check for data blocks and return the row collections
        this->storage.each([&](const Block &aBlock, uint32_t aBlockNumber) {
            if ((BlockType) aBlock.header.type == BlockType::data_block) {
                theRows++;
            }

            return StatusResult();
        });

        stream.close();

        return theRows;
    }

    uint32_t Database::getTableCount() {
        uint32_t theTables = 0;

        if(!this->tablesIndex) {
            loadTablesIndex();
        }

        this->tablesIndex->eachKV([&](const IndexKey &aKey, uint32_t aBlockNum) {
            theTables++;

            return true;
        });

        return theTables - 1; //have a pointer to the header block so subtract 1
    }

    std::string Database::getDBName() {
        return this->name;
    }

    uint32_t Database::getFreeBlock() {
        std::string thePath = Config::getDBPath(this->name);
        this->stream.open(thePath.c_str(), std::fstream::binary |
                                     std::fstream::in | std::fstream::out);
        uint32_t thePos = this->storage.getFreeBlock();
        this->stream.close();

        return thePos;
    }
}

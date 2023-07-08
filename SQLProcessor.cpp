//
//  SQLProcessor.cpp
//  RGAssignment3
//
//  Created by rick gessner on 4/1/21.
//

#include "SQLProcessor.hpp"
#include "EntityView.hpp"
#include "TableDescriptionView.hpp"
#include "IndexView.hpp"
#include "ChangeStatement.hpp"

namespace ECE141 {
    //***********************************Factories**********************************

    using StatementFactory = Statement* (*)(SQLProcessor *aProc, Tokenizer &aTokenizer);

    //********************Statement Factories************************

    Statement* createStatementFactory(SQLProcessor *aProc, Tokenizer &aTokenizer) {
        return new CreateTableStatement(aProc);
    }

    Statement* dropStatementFactory(SQLProcessor *aProc, Tokenizer &aTokenizer) {
        return new DropTableStatement(aProc);
    }

    Statement* describeStatementFactory(SQLProcessor *aProc, Tokenizer &aTokenizer) {
        return new DescribeTableStatement(aProc);
    }

    Statement* showStatementFactory(SQLProcessor *aProc, Tokenizer &aTokenizer) {
        return new ShowTableStatement(aProc);
    }

    Statement* insertStatementFactory(SQLProcessor *aProc, Tokenizer &aTokenizer) {
        return new InsertTableStatement(aProc);
    }

    Statement* selectStatementFactory(SQLProcessor *aProc, Tokenizer &aTokenizer) {
        return new SelectStatement(aProc);
    }

    Statement* updateStatementFactory(SQLProcessor *aProc, Tokenizer &aTokenizer) {
        return new UpdateStatement(aProc);
    }

    Statement* deleteStatementFactory(SQLProcessor *aProc, Tokenizer &aTokenizer) {
        return new DeleteStatement(aProc);
    }

    Statement* indexStatementFactory(SQLProcessor *aProc, Tokenizer &aTokenizer) {
        return new IndexStatement(aProc);
    }

    Statement* indexesStatementFactory(SQLProcessor *aProc, Tokenizer &aTokenizer) {
        return new IndexesStatement(aProc);
    }

    //***********************************Processor**********************************

    SQLProcessor::SQLProcessor(CmdProcessor* theDB, std::ostream &anOutput) : CmdProcessor(anOutput) {
        dbproc = theDB;
    }

    SQLProcessor::~SQLProcessor() {
    }

    //create a table with a given name
    StatusResult SQLProcessor::createTable(Entity* anEntity) {
        if(anEntity) {
            //check if primary key is valid
            StatusResult theResult = primaryCheck(anEntity);

            if(theResult == StatusResult()) {
                auto theDb = getDatabase();

                if (theDb) {
                    if (theResult == StatusResult()) {
                        theResult = theDb->createTable(anEntity);

                        if (theResult == StatusResult()) {
                            this->clock.stop();
                            this->output << "Query OK, 0 rows affected (" << clock.elapsed() << " sec)\n";
                        }
                    }

                    return theResult;
                } else {
                    return StatusResult{Errors::noDatabaseSpecified};
                }
            }

            return theResult;
        }

        return StatusResult{Errors::badPointer};
    }

    //Drop a table with a given name
    StatusResult SQLProcessor::dropTable(const std::string &aName) {
        auto theDb = getDatabase();

        if(theDb) {
            StatusResult theResult = theDb->dropTable(aName, this->output);
            this->clock.stop();
            this->output << "Query OK, 0 rows affected (" <<
            clock.elapsed() << " sec)\n";

            return theResult;
        }
        else {
            return StatusResult{Errors::noDatabaseSpecified};
        }
    }

    //show tables commmand with TableDescriptionView theView(aName, theDb);
    StatusResult SQLProcessor::describeTable(const std::string &aName) {
        auto theDb = getDatabase();

        if(theDb) {
            //load table index
            //navigate to entity
            TableDescriptionView theView(aName, theDb);
            StatusResult theResult = theView.show(this->output) ?
                    StatusResult() : StatusResult{Errors::notImplemented};
            this->clock.stop();
            this->output << "(" << this->clock.elapsed() << " sec)\n";

            return theResult;
        }
        else {
            return StatusResult{Errors::noDatabaseSpecified};
        }
    }

    //show tables commmand with EntityView
    StatusResult SQLProcessor::showTables() {
        auto theDb = getDatabase();

        if(theDb) {
            EntityView theView(theDb);
            StatusResult theResult = theView.show(this->output) ?
                    StatusResult() : StatusResult{Errors::notImplemented};
            this->clock.stop();
            this->output << " (" << this->clock.elapsed() << " sec)\n";

            return theResult;
        }

        return StatusResult{Errors::noDatabaseSpecified};
    }

    //execute the insert command
    StatusResult SQLProcessor::insert(std::string &aName, std::vector<std::vector<Value>> &aValues,
                                      StringList &anAttributes, RowCollection &aRows) {
        //add values to primary key index and any other index that is there

        //validate the input
        StatusResult theResult = validateInput(anAttributes, aValues);

        //create rows
        if(theResult == StatusResult()) {
            theResult = createRows(aRows, anAttributes, aValues);
        }

        //set row name (REFID)
        if(theResult == StatusResult()) {
            for (int i = 0; i < aRows.size(); i++) {
                aRows[i]->setName(aName);
            }
        }

        //add auto_increments & defaults
        if(theResult == StatusResult()) {
            theResult = addAutoDefault(aName, aRows);
        }

        //save rows
        if(theResult == StatusResult()) {//third save row error here
            theResult = saveRows(aName, aRows);
        }

        // clock output
        if(theResult == StatusResult()) {
            this->clock.stop();
            this->output << "Query OK, " << aRows.size() << " rows affected ("
            << this->clock.elapsed() << " sec)\n";
        }

        return theResult;
    }

    //execute the select command
    StatusResult SQLProcessor::select(std::string &aName, RowCollection &aRows,
                                      Query *aQuery, Filters *aFilter, Join* aJoin) {
        if(aQuery && aFilter && aJoin) {
            //if there is a join present
            if(!aJoin->table.empty()) {
                return joinSelect(aName, aRows, aQuery, aFilter, aJoin);
            }
            //validate query
            StatusResult theResult = aQuery->validateQuery();
            //build a row collection
            if (theResult == StatusResult()) {
                theResult = selectRows(aName, aRows);
            }
            //filter the rows
            if (theResult == StatusResult()) {
                theResult = aFilter->filterRows(aRows);
            }
            //sort the rows
            if (theResult == StatusResult()) {
                theResult = aQuery->sortRows(aRows);
            }
            //display rows
            if (theResult == StatusResult()) {
                TabularView theView(aRows, aQuery);
                theResult = theView.show(this->output) ? StatusResult() : StatusResult{Errors::writeError};
                this->clock.stop();
                this->output << "(" << this->clock.elapsed() << " sec)\n";
            }

            return theResult;
        }

        return StatusResult{Errors::badPointer};
    }

    StatusResult SQLProcessor::update(std::string &aName, RowCollection &aRows, Query *aQuery, Filters* aFilter) {
        if(aQuery && aFilter) {
            //validate query
            StatusResult theResult = aQuery->validateQuery();
            //build a row collection
            if (theResult == StatusResult()) {
                theResult = selectRows(aName, aRows);
            }
            //filter the rows
            if (theResult == StatusResult()) {
                theResult = aFilter->filterRows(aRows);
            }
            //update the rows using query
            if (theResult == StatusResult()) {
                theResult = aQuery->updateRows(aRows);
            }
            //update the rows in the database
            if (theResult == StatusResult()) {
                theResult = updateRows(aName, aRows);
            }

            if (theResult == StatusResult()) {
                this->clock.stop();
                this->output << "Query Ok, " << aRows.size() << " rows affected ";
                this->output << "(" << clock.elapsed() << " sec)\n";
            }

            return theResult;
        }

        return StatusResult{Errors::badPointer};
    }

    StatusResult SQLProcessor::deleteFunc(std::string &aName, RowCollection &aRows, Filters *aFilter) {
        StatusResult theResult = validateTable(aName);
        //delete the values in the index from the table
        //mark blocks as free
        //build a row collection
        if(theResult == StatusResult()) {
            theResult = selectRows(aName, aRows);
        }
        //filter the rows (this will contain the rows that need to be deleted)
        if(theResult == StatusResult()) {
            theResult = aFilter->filterRows(aRows);
        }

        if(theResult == StatusResult()) {
            auto theDb = getDatabase();

            if(theDb) {
                theResult = theDb->deleteRows(aName, aRows);
            }
            else {
                theResult = StatusResult{Errors::noDatabaseSpecified};
            }
        }

        if(theResult == StatusResult()) {
            this->clock.stop();
            this->output << "Query Ok, " << aRows.size() << " rows affected ";
            this->output << "(" << clock.elapsed() << " sec)\n";
        }

        return theResult;
    }

    StatusResult SQLProcessor::showIndex(std::string &aTableName, StringList &anAttrs) {
        auto theDb = getDatabase();

        if(theDb) {
            StatusResult theResult = theDb->validateShowIndex(aTableName, anAttrs);

            if(theResult == StatusResult()) {
                IndexView theView(aTableName, anAttrs, theDb);

                theResult = theView.show(this->output) ?
                            StatusResult() : StatusResult{Errors::readError};
            }

            if(theResult == StatusResult()) {
                this->clock.stop();
                this->output << "(" << clock.elapsed() << " sec)\n";
            }

            return theResult;
        }

        return StatusResult{Errors::noDatabaseSpecified};
    }

    StatusResult SQLProcessor::showIndexes() {
        auto theDb = getDatabase();

        if(theDb) {
            IndexesView theView(theDb);
            StatusResult theResult = theView.show(this->output) ?
                                     StatusResult() : StatusResult{Errors::notImplemented};
            this->clock.stop();
            this->output << " (" << this->clock.elapsed() << " sec)\n";

            return theResult;
        }

        return StatusResult{Errors::noDatabaseSpecified};
    }

    //check the primary keys of an entity are valid + add it if none are specified
    StatusResult SQLProcessor::primaryCheck(Entity *anEntity) {
        if(anEntity) {
            bool thePrimaryCheck = false;
            auto theAttributes = anEntity->getAttributes();

            for(int i = 0; i < theAttributes.size(); i++) {
                if(theAttributes[i].isPrimary()) {
                    if(thePrimaryCheck) {
                        return StatusResult{Errors::invalidArguments};
                    }
                    else {
                        thePrimaryCheck = true;
                    }
                }
            }

            if (!thePrimaryCheck) {
                return StatusResult{Errors::primaryKeyRequired};
            }

            return StatusResult();
        }

        return StatusResult{Errors::badPointer};
    }

    //validate the input of a insert statement against the values given for the insert statement
    StatusResult SQLProcessor::validateInput(std::vector<std::string> &anAttribute, std::vector<std::vector<Value>> aValues) {
        StatusResult theResult;

        // check if number of inputs matches number of attributes
        for(auto valueList: aValues) {
            theResult = (anAttribute.size() == valueList.size()) ?
                        StatusResult() : StatusResult{Errors::invalidArguments};

            if(theResult != StatusResult()) {
                return theResult;
            }
        }

        return theResult;
    }

    //create a row and add it to the RowCollection
    StatusResult SQLProcessor::createRows(RowCollection &aRows, StringList &anAttribute, ValuesList &aValues) {
        for(int i = 0; i < aValues.size(); i++) {
            RowPtr theRow{new Row()};

            for(int j = 0; j < anAttribute.size(); j++) {
                std::pair<std::string, Value> thePair = {anAttribute[j], aValues[i][j]};
                theRow->addAttribute(thePair);
            }

            aRows.emplace_back(std::move(theRow));
        }

        return StatusResult();
    }

    //save a row to the database
    StatusResult SQLProcessor::saveRows(std::string &aTableName, RowCollection &aRows) {
        auto theDb = getDatabase();

        if(theDb) {
            return theDb->saveRows(aTableName, aRows);
        }
        else {
            return StatusResult{Errors::noDatabaseSpecified};
        }
    }

    //add defaults/auto defaults to entries in the row that were not inputted
    StatusResult SQLProcessor::addAutoDefault(std::string &aName, RowCollection &aRows) {
        auto theDb = getDatabase();

        if(!theDb) {
            return StatusResult{Errors::noDatabaseSpecified};
        }

        Entity* theEntity = theDb->getEntity(aName);

        if(theEntity) {
            std::vector<Attribute> attrs;
            std::vector<Attribute> nullAttrs;
            auto keyVals = aRows[0]->getData();

            //aggregate nullables and values that need default
            for (auto attr: theEntity->getAttributes()) {
                auto it = keyVals.find(attr.getName());

                if (it == keyVals.end()) {
                    if (!attr.isNullable() || attr.isAuto()) {
                        attrs.push_back(attr);
                    } else if(attr.isNullable()) {
                        nullAttrs.push_back(attr);
                    }
                }
            }

            StatusResult theResult = StatusResult();

            //apply default
            for(int i = 0; i < aRows.size(); i++) {
                for(auto attr: attrs) {
                    theResult = aRows[i]->addDefault(theEntity, attr);

                    if(theResult != StatusResult()) {
                        return theResult;
                    }
                }

                for(auto attr: nullAttrs) {
                    theResult = aRows[i]->addNull(theEntity, attr);

                    if(theResult != StatusResult()) {
                        return theResult;
                    }
                }
            }

            if(theResult == StatusResult()) {
                theResult = theDb->updateEntity(theEntity);
            }

            return theResult;
        } else {
            return StatusResult{Errors::unknownTable};
        }
    }

    //select statement if there is a join present
    StatusResult SQLProcessor::joinSelect(std::string &aName, RowCollection &aRows,
                                          Query *aQuery, Filters *aFilter, Join *aJoin) {
        if(aQuery && aFilter && aJoin) {
            //validate query
            StatusResult theResult = aQuery->validateJoinQuery(getEntity(aJoin->lhs.table),
                                                                         getEntity(aJoin->rhs.table));
            //validate join
            if(theResult == StatusResult()) {
                theResult = aJoin->validateJoin(aName);
            }
            //validate join tables
            if(theResult == StatusResult()) {
                theResult = validateJoinTables(aJoin);
            }
            //build a row collection
            RowCollection theLeftRows, theRightRows;
            //get left rows
            if(theResult == StatusResult()) {
                theResult = selectRows(aJoin->lhs.table, theLeftRows);
            }
            //get the right rows
            if(theResult == StatusResult()) {
                theResult = selectRows(aJoin->rhs.table, theRightRows);
            }
            //perform the join
            if(theResult == StatusResult()) {
                theResult = aJoin->joinRows(aRows, theLeftRows, theRightRows);
            }
            //filter the rows
            if (theResult == StatusResult()) {
                theResult = aFilter->filterRows(aRows);
            }
            //sort the rows
            if (theResult == StatusResult()) {
                theResult = aQuery->sortRows(aRows);
            }
            //display rows
            if (theResult == StatusResult()) {
                TabularView theView(aRows, aQuery);
                theResult = theView.show(this->output) ? StatusResult() : StatusResult{Errors::writeError};
                this->clock.stop();
                this->output << "(" << this->clock.elapsed() << " sec)\n";
            }

            return theResult;
        }

        return StatusResult{Errors::badPointer};
    }

    // select all the rows of a table of name aName
    StatusResult SQLProcessor::selectRows(std::string &aName, RowCollection &aRows) {
        auto theDb = getDatabase();

        if(theDb) {
            return theDb->selectRows(aName, aRows);
        }
        else {
            return StatusResult{Errors::noDatabaseSpecified};
        }
    }

    StatusResult SQLProcessor::validateJoinTables(Join *aJoin) {
        if(aJoin->table != "") {
            StatusResult theResult = validateTableAttr(aJoin->lhs.table, aJoin->lhs.attribute);

            if (theResult == StatusResult()) {
                theResult = validateTableAttr(aJoin->rhs.table, aJoin->rhs.attribute);
            }

            return theResult;
        }

        return StatusResult();
    }

    StatusResult SQLProcessor::validateTableAttr(std::string &aTableName,
                                                 std::string &anAttrName) {
        Entity* theEntity = getEntity(aTableName);

        if(theEntity) {
            Attribute* theAttr = theEntity->getAttribute(anAttrName);

            delete theEntity;

            if(theAttr) {
                return StatusResult();
            }

            return StatusResult{Errors::unknownAttribute};
        }

        delete theEntity;

        return StatusResult{Errors::unknownTable};
    }

    StatusResult SQLProcessor::validateTable(std::string &aTableName) {
        Entity* theEntity = getEntity(aTableName);

        if(theEntity) {
            delete theEntity;

            return StatusResult();
        }

        delete theEntity;

        return StatusResult{Errors::unknownTable};
    }

    //get the entity of a table of name aTableName
    Entity * SQLProcessor::getEntity(std::string &aTableName) {
        auto theDb = getDatabase();

        if(theDb) {
            return theDb->getEntity(aTableName);
        }

        return nullptr;
    }

    //update the blocks in the database
    StatusResult SQLProcessor::updateRows(std::string &aTableName, RowCollection &aRows) {
        auto theDb = getDatabase();

        if(theDb) {
            return theDb->updateRows(aTableName, aRows);
        }

        return StatusResult{Errors::noDatabaseSpecified};
    }

    //get the current database
    Database * SQLProcessor::getDatabase() {
        if(this->dbproc) {
            return this->dbproc->getDatabase();
        }

        return nullptr;
    }

    CmdProcessor * SQLProcessor::recognizes(Tokenizer &aTokenizer) {
        return CreateTableStatement::recognizes(aTokenizer) ||
                DropTableStatement::recognizes(aTokenizer) ||
                DescribeTableStatement::recognizes(aTokenizer) ||
                ShowTableStatement::recognizes(aTokenizer) ||
                InsertTableStatement::recognizes(aTokenizer) ||
                SelectStatement::recognizes(aTokenizer) ||
                UpdateStatement::recognizes(aTokenizer) ||
                DeleteStatement::recognizes(aTokenizer) ||
                IndexStatement::recognizes(aTokenizer) ||
                IndexesStatement::recognizes(aTokenizer) ? this : nullptr;
    }

    Statement * SQLProcessor::makeStatement(Tokenizer &aTokenizer) {
        static std::map<Keywords, StatementFactory> factories = {
                {Keywords::create_kw, createStatementFactory},
                {Keywords::drop_kw, dropStatementFactory},
                {Keywords::describe_kw, describeStatementFactory},
                {Keywords::show_kw, showStatementFactory},
                {Keywords::insert_kw, insertStatementFactory},
                {Keywords::select_kw, selectStatementFactory},
                {Keywords::update_kw, updateStatementFactory},
                {Keywords::delete_kw, deleteStatementFactory},
                {Keywords::index_kw, indexStatementFactory},
                {Keywords::indexes_kw, indexesStatementFactory},
        };

        if(aTokenizer.size()) {
            Token &theToken = aTokenizer.current();
            if (factories.count(theToken.keyword)) {
                if (Statement *theStatement = factories
                [theToken.keyword](this, aTokenizer)) {
                    if (theStatement->parse(aTokenizer) == StatusResult()) {
                        return theStatement;
                    }
                }
            }
        }

        return nullptr;
    }

    StatusResult SQLProcessor::run(Statement *aStatement, const Timer &aTimer) {
        if(aStatement) {
            this->clock = aTimer;

            return aStatement->dispatch();
        }

        return StatusResult{Errors::badPointer};
    }
}

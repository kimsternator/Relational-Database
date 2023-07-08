//
// Created by Stephen on 4/7/2021.
//

#include "DbProcessor.hpp"
#include "DbStatement.hpp"
#include "SQLProcessor.hpp"

namespace ECE141 {

    using StatementFactory = Statement* (*)(DbProcessor *aProc, Tokenizer &aTokenizer);

    Statement* createStatementFactory(DbProcessor *aProc, Tokenizer &aTokenizer) {
        return new CreateStatement(aProc);
    }

    Statement* dumpStatementFactory(DbProcessor *aProc, Tokenizer &aTokenizer) {
        return new DumpStatement(aProc);
    }

    Statement* dropStatementFactory(DbProcessor *aProc, Tokenizer &aTokenizer) {
        return new DropStatement(aProc);
    }

    Statement* showStatementFactory(DbProcessor *aProc, Tokenizer &aTokenizer) {
        return new ShowStatement(aProc);
    }

    Statement* useStatementFactory(DbProcessor *aProc, Tokenizer &aTokenizer) {
        return new UseStatement(aProc);
    }

    DbProcessor::DbProcessor(std::ostream &anOutput) : CmdProcessor(anOutput) {
        theSQL = new SQLProcessor(this, anOutput);
        currDb = nullptr;
    }

    DbProcessor::~DbProcessor() {
        //destroy the processor
        releaseDB();
    }

    // create a database
    StatusResult DbProcessor::createDatabase(const std::string &aName) {
        if (!DBExists(aName)) {
            try {
                // delete curr DB if found
                releaseDB();

                if (auto theDb = new Database(aName, CreateDB())) {
                    this->clock.stop();
                    this->output << "Query OK, 1 rows affected (" << this->clock.elapsed() << " sec)\n";

                    delete theDb;

                    return StatusResult();
                }

                return StatusResult{Errors::databaseCreationError};
            }
            catch(...) {}
        }

        return StatusResult{Errors::databaseExists};
    }

    // drop a database
    StatusResult DbProcessor::dropDatabase(const std::string &aName) {
        if (DBExists(aName)) {
            try {
                auto theDb = new Database(aName, OpenDB());
                //count number of tables
                uint32_t theTables = theDb->getTableCount();
                //delete the database
                std::string path = Config::getDBPath(aName);
                std::filesystem::remove(path);
                this->clock.stop();
                this->output << "Query OK, " << theTables << " rows affected ("
                << this->clock.elapsed() << " sec)\n";

                if(this->currDb) {
                    if(this->currDb->getDBName() == aName) {
                        releaseDB();
                    }
                }

                return StatusResult{Errors::noError};
            }
            catch (...) {}
        }

        return StatusResult{Errors::unknownDatabase};
    }

    // show databases using FolderView
    StatusResult DbProcessor::showDatabases() {
        FolderView theView(Config::getStoragePath(), Config::getDBExtension());
        theView.show(this->output);
        this->clock.stop();
        this->output << "(" << this->clock.elapsed() << " sec)\n";

        return StatusResult();
    }

    // load a database into the currDb data member
    StatusResult DbProcessor::useDatabase(const std::string &aName) {
        if (DBExists(aName)) {
            try {
                releaseDB();

                std::string path = Config::getDBPath(aName);

                if((this->currDb = new Database(aName, OpenDB()))) {
                    this->output << "Database changed\n";

                    return StatusResult();
                }
            }
            catch (...) {}
        }

        return StatusResult{Errors::unknownDatabase};
    }

    // dump a database
    StatusResult DbProcessor::dumpDatabase(const std::string &aName) {
        if (DBExists(aName)) {
            try {
                if(auto aDb = new Database(aName, OpenDB())) {
                    StatusResult theResult = aDb->dumpDatabase(this->output);
                    this->clock.stop();
                    this->output << "(" << clock.elapsed() << " sec)\n";

                    delete aDb;

                    return theResult;
                }
            }
            catch (...) {}
        }

        return StatusResult{Errors::unknownDatabase};
    }

    // recognizes statement using all statement's static recognize
    CmdProcessor * DbProcessor::recognizes(Tokenizer &aTokenizer) {
        if(CreateStatement::recognizes(aTokenizer) ||
                DumpStatement::recognizes(aTokenizer) ||
                DropStatement::recognizes(aTokenizer) ||
                ShowStatement::recognizes(aTokenizer) ||
                UseStatement::recognizes(aTokenizer)) {
            return this;
        }

        // link to next processor
        return this->theSQL ? this->theSQL->recognizes(aTokenizer) : nullptr;
    }

    // make a statement using a factory
    Statement * DbProcessor::makeStatement(Tokenizer &aTokenizer) {
        static std::map<Keywords, StatementFactory> factories = {
                {Keywords::create_kw, createStatementFactory},
                {Keywords::drop_kw, dropStatementFactory},
                {Keywords::dump_kw, dumpStatementFactory},
                {Keywords::use_kw, useStatementFactory},
                {Keywords::show_kw, showStatementFactory},
        };

        if(aTokenizer.size()) {
            Token &theToken = aTokenizer.current();
            if (factories.count(theToken.keyword)) {
                if (Statement *theStatement = factories[theToken.keyword](this, aTokenizer)) {
                    if (theStatement->parse(aTokenizer) == StatusResult()) {
                        return theStatement;
                    }
                }
            }
        }

        // no valid statement factory found
        return nullptr;
    }

    // run using overloaded run statement
    StatusResult DbProcessor::run(Statement *aStatement, const Timer &aTimer) {
        this->clock = aTimer;

        return aStatement->dispatch();
    }

    Database * DbProcessor::getDatabase() {
        return this->currDb;
    }
}
//
// Created by Stephen on 4/7/2021.
//

#ifndef ECE141B_DBPROCESSOR_HPP
#define ECE141B_DBPROCESSOR_HPP

#include <cmath>
#include <filesystem>

#include "CmdProcessor.hpp"
#include "FolderView.hpp"
#include "Config.hpp"


namespace ECE141 {
    class DbProcessor : public CmdProcessor {
    public:
        DbProcessor(std::ostream &anOutput);
        virtual ~DbProcessor();

        StatusResult createDatabase(const std::string &aName);
        StatusResult dropDatabase(const std::string &aName);
        StatusResult useDatabase(const std::string &aName);
        StatusResult showDatabases();
        StatusResult dumpDatabase(const std::string &aName);

        //Statement commands
        CmdProcessor *recognizes(Tokenizer &aTokenizer) override;
        Statement *makeStatement(Tokenizer &aTokenizer) override;
        StatusResult run(Statement *aStmt, const Timer &aTimer) override;

        // Getters
        Database* getDatabase() override;

    protected:
        Database* currDb;
        CmdProcessor* theSQL;

        bool DBExists(const std::string &aName) {
            std::string thePath = Config::getDBPath(aName);

            return std::filesystem::exists(thePath);
        }

        void releaseDB() {
            delete this->currDb;

            this->currDb = nullptr;
        }
    };
}


#endif //ECE141B_DBPROCESSOR_HPP

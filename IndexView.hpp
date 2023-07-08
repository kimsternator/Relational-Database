//
// Created by Stephen on 5/25/2021.
//

#ifndef ECE141B_INDEXVIEW_HPP
#define ECE141B_INDEXVIEW_HPP

#include "Database.hpp"
#include "View.hpp"

namespace ECE141 {
    class IndexesView : public View {
    public:
        IndexesView(Database *aDb) : thisDb(aDb) {}

        virtual bool show(std::ostream &anOutput) {
            size_t rows = this->thisDb->showTableIndexes(anOutput);
            anOutput << rows << " rows in set ";

            return true;
        }

    protected:
        Database* thisDb;
    };

    class IndexView : public View {
    public:
        IndexView(std::string &aTableName, StringList &anAttrs, Database *aDb) :
                tableName(aTableName), attrs(anAttrs), thisDb(aDb) {}

        virtual bool show(std::ostream &anOutput) {
            size_t theRows = this->thisDb->showTableIndex(this->tableName, this->attrs, anOutput);
            anOutput << theRows << " rows in set ";

            return true;
        }

    protected:
        StringList attrs;
        std::string tableName;
        Database* thisDb;
    };
}

#endif //ECE141B_INDEXVIEW_HPP

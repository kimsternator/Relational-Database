//
// Created by Stephen on 4/19/2021.
//

#ifndef ECE141B_TABLEDESCRIPTIONVIEW_HPP
#define ECE141B_TABLEDESCRIPTIONVIEW_HPP

#include "Database.hpp"

namespace ECE141 {
    class TableDescriptionView : public View {
    public:           
        TableDescriptionView(const std::string &aName, Database *aDb) : name(aName), thisDb(aDb) {}

        virtual bool show(std::ostream &anOutput) {
            size_t rows = this->thisDb->describeTable(this->name, anOutput);
            anOutput << rows << " rows in set ";

            return true;
        }

    protected:
        Database* thisDb;
        std::string name;
    };
}

#endif //ECE141B_TABLEDESCRIPTIONVIEW_HPP

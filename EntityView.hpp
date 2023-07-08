//
// Created by Stephen on 4/19/2021.
//

#ifndef ECE141B_ENTITYVIEW_HPP
#define ECE141B_ENTITYVIEW_HPP

#include "Database.hpp"
#include "View.hpp"

namespace ECE141 {
    class EntityView : public View {
    public:
        EntityView(Database *aDb) : thisDb(aDb) {}

        virtual bool show(std::ostream &anOutput) {
            size_t rows = this->thisDb->showTables(anOutput);
            anOutput << rows << " rows in set ";

            return true;
        }

    protected:
        Database* thisDb;
    };
}

#endif //ECE141B_ENTITYVIEW_HPP

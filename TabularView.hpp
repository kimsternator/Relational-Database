
//
//  RecordsView.hpp
//
//  Created by rick gessner on 4/26/20.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#ifndef TabularView_h
#define TabularView_h

#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include <vector>
#include <string>
#include "View.hpp"
#include "Row.hpp"
#include "Query.hpp"

//IGNORE THIS CLASS if you already have a class that does this...

namespace ECE141 {
    // USE: general tabular view (with columns)
    class TabularView : public View {
    public:
        TabularView(RowCollection &aRows, Query* aQuery) : rows(aRows) {
            selects = aQuery->getSelects();
            limit = aQuery->getLimit();
            offset = aQuery->getOffset();
        }

        void addSelect(std::string &aSelect) {
            this->sizePairs.insert({aSelect, aSelect.size()});
        }

        void addRow(StringList &aRowData) {
            for(size_t i = 0; i < aRowData.size(); i++) {
                this->sizePairs[this->selects[i]] = std::max(this->sizePairs[this->selects[i]], aRowData[i].size());
            }

            this->printerRows.push_back(aRowData);
        }

        void createBar() {
            for(auto& select: this->selects) {
                this->bar +=  "+" + std::string(this->sizePairs[select] + 2, '-');
            }

            this->bar += "+\n";
        }

        void showHeader(std::ostream &anOutput) {
            anOutput << this->bar;

            for(auto &select: this->selects) {
                size_t whiteSpace = this->sizePairs[select] - select.length();
                anOutput << "| " << std::setw(this->sizePairs[select])
                        << (std::string(whiteSpace / 2, ' ') + select +
                        std::string(whiteSpace / 2, ' ')) << " ";
            }

            anOutput << "|\n" << this->bar;
        }

        void showRows(std::ostream &anOutput) {
            for(auto &row: this->printerRows) {
                for(size_t i = 0; i < row.size(); i++) {
                    anOutput << "| " << std::left << std::setw(this->sizePairs[this->selects[i]])
                    << row[i] << " ";
                }

                anOutput << "|\n";
            }

            anOutput << this->bar;
        }

        bool show(std::ostream &anOutput) override {
            //output each value that is selected
            for(auto select: this->selects) {
                addSelect(select);
            }

            size_t upperLimit = this->limit == 0 ? this->rows.size() : this->offset + this->limit;

            for(size_t i = this->offset; i < this->rows.size() && i < upperLimit; i++) {
                StringList theRowData;

                for(auto select: this->selects) {
                    theRowData.push_back(this->rows[i]->getDecodedValue(select));
                }

                addRow(theRowData);
            }

            createBar();
            showHeader(anOutput);
            showRows(anOutput);
            anOutput << this->printerRows.size() << " rows in set ";

            return true;
        }

    protected:
        StringList selects;
        size_t limit;
        size_t offset;
        RowCollection &rows;

        std::string bar;
        std::map<std::string, size_t> sizePairs;
        std::vector<StringList> printerRows;
    };

}

#endif /* TabularView_h */

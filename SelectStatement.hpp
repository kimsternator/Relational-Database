//
// Created by Stephen on 4/28/2021.
//

#ifndef ECE141B_SELECTSTATEMENT_HPP
#define ECE141B_SELECTSTATEMENT_HPP

#include "SQLStatement.hpp"
#include "TabularView.hpp"
#include "Join.hpp"

namespace ECE141 {
    class SelectStatement : public SQLStatement {
    public:
        SelectStatement(SQLProcessor* aProc, Keywords aStatementType=Keywords::select_kw)
        : SQLStatement(aProc, aStatementType){
            query = new Query();
            filter = new Filters();
            join = new Join();
        }

        ~SelectStatement() {
            delete query;
            delete filter;
            delete join;
        }

        static bool recognizes(Tokenizer &aTokenizer);
        StatusResult parse(Tokenizer &aTokenizer) override;
        StatusResult dispatch() override;

        //select helpers
        StatusResult parseSelect(Tokenizer &aTokenizer);
        StatusResult parseTable(Tokenizer &aTokenizer);
        StatusResult parseSelectInputs(Tokenizer &aTokenizer);
        StatusResult parseTableField(Tokenizer &aTokenizer, TableField &aField);
        StatusResult parseJoin(Tokenizer &aTokenizer);
        StatusResult parseExtra(Tokenizer &aTokenizer);

    protected:
        std::string tableName;
        RowCollection rows;
        Query* query;
        Filters* filter;
        Join* join;
    };
}


#endif //ECE141B_SELECTSTATEMENT_HPP

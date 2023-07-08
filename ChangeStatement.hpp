//
// Created by Stephen on 5/12/2021.
//

#ifndef ECE141B_CHANGESTATEMENT_HPP
#define ECE141B_CHANGESTATEMENT_HPP

#include "SelectStatement.hpp"

namespace ECE141 {
    class UpdateStatement : public SelectStatement {
    public:
        UpdateStatement(SQLProcessor *aProc, Keywords aStatementType=Keywords::update_kw)
        : SelectStatement(aProc, aStatementType) {}

        static bool recognizes(Tokenizer &aTokenizer);
        StatusResult parse(Tokenizer &aTokenizer) override;
        StatusResult dispatch() override;

        //update helpers
        StatusResult parseUpdate(Tokenizer &aTokenizer);
        StatusResult parseUpdateTable(Tokenizer &aTokenizer);
        StatusResult parseUpdateExtra(Tokenizer &aTokenizer);
        StatusResult parseSet(Tokenizer &aTokenizer);
        StatusResult parseQueryUpdate(Tokenizer &aTokenizer);

    protected:
    };

    class DeleteStatement : public SelectStatement {
    public:
        DeleteStatement(SQLProcessor *aProc, Keywords aStatementType=Keywords::update_kw)
                : SelectStatement(aProc, aStatementType) {}

        static bool recognizes(Tokenizer &aTokenizer);
        StatusResult parse(Tokenizer &aTokenizer) override;
        StatusResult dispatch() override;

        //delete helpers
        StatusResult parseDelete(Tokenizer &aTokenizer);
        StatusResult parseDeleteTable(Tokenizer &aTokenizer);
        StatusResult parseDeleteExtra(Tokenizer &aTokenizer);

    protected:
    };
}


#endif //ECE141B_CHANGESTATEMENT_HPP

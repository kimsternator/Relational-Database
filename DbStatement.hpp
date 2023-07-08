//
// Created by Stephen on 4/13/2021.
//

#ifndef ECE141B_DBSTATEMENT_HPP
#define ECE141B_DBSTATEMENT_HPP

#include "Statement.hpp"
#include "View.hpp"
#include "DbProcessor.hpp"

namespace ECE141 {
    class DbStatement : public Statement {
    public:
        DbStatement(DbProcessor* aProc, Keywords aStatementType=Keywords::unknown_kw) : proc(aProc) , Statement(aStatementType) {}

    protected:
        DbProcessor* proc;
    };

    class CreateStatement : public DbStatement {
    public:
        CreateStatement(DbProcessor* aProc, Keywords aStatementType=Keywords::create_kw) : DbStatement(aProc, aStatementType){}

        static bool recognizes(Tokenizer &aTokenizer);
        StatusResult parse(Tokenizer &aTokenizer) override;
        StatusResult dispatch() override;

    protected:
        std::string name;
    };

    class ShowStatement :public DbStatement {
    public:
        ShowStatement(DbProcessor* aProc, Keywords aStatementType=Keywords::show_kw) : DbStatement(aProc, aStatementType){}

        static bool recognizes(Tokenizer &aTokenizer);
        StatusResult parse(Tokenizer &aTokenizer) override;
        StatusResult dispatch() override;

    protected:
    };

    class DropStatement :public DbStatement {
    public:
        DropStatement(DbProcessor* aProc, Keywords aStatementType=Keywords::unknown_kw) : DbStatement(aProc, aStatementType){}

        static bool recognizes(Tokenizer &aTokenizer);
        StatusResult parse(Tokenizer &aTokenizer) override;
        StatusResult dispatch() override;

    protected:
        std::string name;
    };

    class UseStatement :public DbStatement {
    public:
        UseStatement(DbProcessor* aProc, Keywords aStatementType=Keywords::unknown_kw) : DbStatement(aProc, aStatementType){}

        static bool recognizes(Tokenizer &aTokenizer);
        StatusResult parse(Tokenizer &aTokenizer) override;
        StatusResult dispatch() override;

    protected:
        std::string name;
    };

    class DumpStatement :public DbStatement {
    public:
        DumpStatement(DbProcessor* aProc, Keywords aStatementType=Keywords::unknown_kw) : DbStatement(aProc, aStatementType){}

        static bool recognizes(Tokenizer &aTokenizer);
        StatusResult parse(Tokenizer &aTokenizer) override;
        StatusResult dispatch() override;

    protected:
        std::string name;
    };
}


#endif //ECE141B_DBSTATEMENT_HPP

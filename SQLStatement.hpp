//
// Created by Stephen on 4/15/2021.
//

#ifndef ECE141B_SQLSTATEMENT_HPP
#define ECE141B_SQLSTATEMENT_HPP

#include "Statement.hpp"
#include "SQLProcessor.hpp"
#include "TokenSequence.hpp"
#include "Row.hpp"

namespace ECE141 {
    class SQLStatement : public Statement {
    public:
        SQLStatement(SQLProcessor* aProc, Keywords aStatementType=Keywords::unknown_kw) : proc(aProc) , Statement(aStatementType) {}

    protected:
        SQLProcessor* proc;
    };

    class CreateTableStatement : public SQLStatement {
    public:
        CreateTableStatement(SQLProcessor* aProc, Keywords aStatementType=Keywords::create_kw) : SQLStatement(aProc, aStatementType){
            entity = nullptr;
        }

        static bool recognizes(Tokenizer &aTokenizer);
        StatusResult parse(Tokenizer &aTokenizer) override;
        StatusResult dispatch() override;

        //create helpers
        StatusResult parseCreate(Tokenizer &aTokenizer);
        StatusResult parseCreateAttributes(Tokenizer &aTokenizer);

    protected:
        Entity* entity;
    };

    class DropTableStatement : public SQLStatement {
    public:
        DropTableStatement(SQLProcessor* aProc, Keywords aStatementType=Keywords::drop_kw) : SQLStatement(aProc, aStatementType){}

        static bool recognizes(Tokenizer &aTokenizer);
        StatusResult parse(Tokenizer &aTokenizer) override;
        StatusResult dispatch() override;

    protected:
        std::string name;
    };

    class DescribeTableStatement : public SQLStatement {
    public:
        DescribeTableStatement(SQLProcessor* aProc, Keywords aStatementType=Keywords::describe_kw) : SQLStatement(aProc, aStatementType){}

        static bool recognizes(Tokenizer &aTokenizer);
        StatusResult parse(Tokenizer &aTokenizer) override;
        StatusResult dispatch() override;

    protected:
        std::string name;
    };

    class ShowTableStatement : public SQLStatement {
    public:
        ShowTableStatement(SQLProcessor* aProc, Keywords aStatementType=Keywords::show_kw) : SQLStatement(aProc, aStatementType){}

        static bool recognizes(Tokenizer &aTokenizer);
        StatusResult parse(Tokenizer &aTokenizer) override;
        StatusResult dispatch() override;

    protected:
    };

    class InsertTableStatement : public SQLStatement {
    public:
        InsertTableStatement(SQLProcessor* aProc, Keywords aStatementType=Keywords::show_kw) : SQLStatement(aProc, aStatementType){}

        static bool recognizes(Tokenizer &aTokenizer);
        StatusResult parse(Tokenizer &aTokenizer) override;
        StatusResult dispatch() override;

        //insert helpers
        StatusResult parseInsert(Tokenizer &aTokenizer);
        StatusResult parseInsertAttributes(Tokenizer &aTokenizer);
        StatusResult parseInsertValues(Tokenizer &aTokenizer);

    protected:
        std::string name;
        ValuesList values;
        StringList attributes;
        RowCollection rows;
    };

    class IndexStatement : public SQLStatement {
    public:
        IndexStatement(SQLProcessor* aProc, Keywords aStatementType=Keywords::index_kw)
                : SQLStatement(aProc, aStatementType) {
        }

        //index helpers
        StatusResult parseIndexFields(Tokenizer &aTokenizer);
        StatusResult parseTableName(Tokenizer &aTokenizer);

        static bool recognizes(Tokenizer &aTokenizer);
        StatusResult parse(Tokenizer &aTokenizer) override;
        StatusResult dispatch() override;

    protected:
        std::string tableName;
        StringList attrNames;
    };

    class IndexesStatement : public SQLStatement {
    public:
        IndexesStatement(SQLProcessor* aProc, Keywords aStatementType=Keywords::indexes_kw)
                : SQLStatement(aProc, aStatementType) {
        }

        static bool recognizes(Tokenizer &aTokenizer);
        StatusResult parse(Tokenizer &aTokenizer) override;
        StatusResult dispatch() override;

    protected:
    };
}

#endif //ECE141B_SQLSTATEMENT_HPP

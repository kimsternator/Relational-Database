//
// Created by Stephen on 4/28/2021.
//

#include "SelectStatement.hpp"

namespace ECE141 {
    //***********************************Factories**********************************

    using ParseFactory = StatusResult (*)(Query *aQuery, Filters *aFilter, Tokenizer &aTokenizer);

    //***********************************Parse Factories****************************

    StatusResult orderFactory(Query *aQuery, Filters *aFilter, Tokenizer &aTokenizer) {
        if(aQuery) {
            TokenSequence theSeq(aTokenizer);
            StatusResult theResult = theSeq.is(Keywords::order_kw)
                                             .then(Keywords::by_kw)
                                             .then(TokenType::identifier).matches() ?
                                     StatusResult() : StatusResult{Errors::invalidArguments};
            aTokenizer.next(theSeq.offset);

            if (theResult == StatusResult()) {
                while (aTokenizer.remaining() >= 2 && aTokenizer.current().type == TokenType::identifier) {
                    theResult = aQuery->addOrder(aTokenizer.current().data);
                    aTokenizer.next();

                    if (theResult != StatusResult()) {
                        break;
                    }

                    if (aTokenizer.current().type == TokenType::punctuation) {
                        if(aTokenizer.current().data == ",") {
                            aTokenizer.next();
                        }
                        else {
                            break;
                        }
                    }
                }
            }

            return theResult;
        }

        return StatusResult{Errors::badPointer};
    }

    StatusResult limitFactory(Query *aQuery, Filters *aFilter, Tokenizer &aTokenizer) {
        if(aQuery) {
            StatusResult theResult = StatusResult();

            if (aTokenizer.current().keyword == Keywords::limit_kw) {
                aTokenizer.next();

                if (aTokenizer.current().type == TokenType::number) {
                    theResult = aQuery->setLimit(stoi(aTokenizer.current().data));
                    aTokenizer.next();
                }
            }

            return theResult;
        }

        return StatusResult{Errors::badPointer};
    }

    StatusResult groupFactory(Query *aQuery, Filters *aFilter, Tokenizer &aTokenizer) {
        return StatusResult{Errors::notImplemented};
    }

    /* --------------------Statement Implementations------------------------ */

    bool SelectStatement::recognizes(Tokenizer &aTokenizer) {
        TokenSequence theSeq(aTokenizer);

        return theSeq.has(Keywords::select_kw).matches();
    }

    StatusResult SelectStatement::parse(Tokenizer &aTokenizer) {
        return parseSelect(aTokenizer);
    }

    StatusResult SelectStatement::dispatch() {
        if(this->proc) {
            return this->proc->select(this->tableName, this->rows, this->query, this->filter, this->join);
        }

        return StatusResult{Errors::badPointer};
    }

    /* --------------------Helper Functions--------------------- */

    StatusResult SelectStatement::parseSelect(Tokenizer &aTokenizer) {
        if(this->query && this->filter) {
            aTokenizer.next(); //skip select
            //parse query
            StatusResult theResult = parseSelectInputs(aTokenizer);
            //parse tableName entity into query
            if (theResult == StatusResult()) {
                theResult = parseTable(aTokenizer);
            }
            //parse filters & query (where, order_by, limit, *group_by, *join) *later
            if (theResult == StatusResult()) {
                theResult = parseExtra(aTokenizer);
            }
            if (this->query->selectAll()) {
                this->query->setAll();
            }

            return theResult;
        }

        return StatusResult{Errors::badPointer};
    }

    //parse the table name of a select statement and add that to aQuery
    StatusResult SelectStatement::parseTable(Tokenizer &aTokenizer) {
        if(this->query) {
            TokenSequence theSeq(aTokenizer);

            if (theSeq.has(Keywords::from_kw)
                    .thenId(this->tableName).matches()) {
                if(this->proc) {
                    auto theEntity = this->proc->getEntity(this->tableName);

                    if (theEntity) {
                        this->query->setFrom(theEntity);
                        aTokenizer.next(theSeq.offset);
                    }
                    else {
                        return StatusResult{Errors::unknownTable};
                    }
                }
                else {
                    return StatusResult{Errors::badPointer};
                }
            }
            else {
                return StatusResult{Errors::invalidCommand};
            }

            return StatusResult();
        }

        return StatusResult{Errors::badPointer};
    }

    //parse select inputs of a select statement
    StatusResult SelectStatement::parseSelectInputs(Tokenizer &aTokenizer) {
        if(this->query) {
            if (aTokenizer.current().type == TokenType::operators) {
                if (aTokenizer.current().data == "*") {
                    aTokenizer.next();
                    return this->query->setSelectAll(true).selectAll() ?
                           StatusResult() : StatusResult{Errors::unknownError};
                } else {
                    return StatusResult{Errors::invalidArguments};
                }
            }

            StringList theList;

            while (aTokenizer.remaining() >= 2 && aTokenizer.current().type == TokenType::identifier) {
                theList.push_back(aTokenizer.current().data);
                aTokenizer.next();

                if (aTokenizer.current().type == TokenType::punctuation)
                    if (aTokenizer.current().data == ",") {
                        aTokenizer.next();
                    }
                    else {
                        return StatusResult{Errors::invalidArguments};
                    }
            }

            return !this->query->setSelect(theList).selectAll() ?
                   StatusResult() : StatusResult{Errors::unknownError};
        }

        return StatusResult{Errors::badPointer};
    }

    //parse a tablefield
    StatusResult SelectStatement::parseTableField(Tokenizer &aTokenizer, TableField &aField) {
        if(aTokenizer.current().type == TokenType::identifier) {
            aField.table = aTokenizer.current().data;
            aTokenizer.next();

            if(aTokenizer.current().type == TokenType::operators &&
            aTokenizer.current().data == ".") {
                aTokenizer.next();

                if(aTokenizer.current().type == TokenType::identifier) {
                    aField.attribute = aTokenizer.current().data;

                    if(aTokenizer.more()) {
                        aTokenizer.next();
                    }

                    return StatusResult();
                }
            }
        }

        return StatusResult{Errors::invalidAttribute};
    }

    //parse the join clause
    StatusResult SelectStatement::parseJoin(Tokenizer &aTokenizer) {
        StatusResult theResult{Errors::joinTypeExpected};
        Keywords theJoinType{Keywords::join_kw};

        if(in_array<Keywords>(gJoinTypes, aTokenizer.current().keyword)) {
            theJoinType = aTokenizer.current().keyword;
            aTokenizer.next();

            if(aTokenizer.current().keyword == Keywords::join_kw) {
                aTokenizer.next();

                if(aTokenizer.current().type == TokenType::identifier) {
                    std::string theTableName = aTokenizer.current().data;
                    this->join->table = theTableName;
                    this->join->joinType = theJoinType;
                    aTokenizer.next();

                    if(aTokenizer.current().keyword == Keywords::on_kw) {
                        aTokenizer.next();
                        theResult = parseTableField(aTokenizer, this->join->lhs);

                        if(theResult == StatusResult()) {
                            if (aTokenizer.current().type == TokenType::operators &&
                            aTokenizer.current().data == "=") {
                                aTokenizer.next();
                                theResult = parseTableField(aTokenizer, this->join->rhs);
                            }
                        }
                    }
                }
            }


            return theResult;
        }

        return theResult;
    }

    //parse the additionals of a select statement (group by, order by, limit, etc)
    StatusResult SelectStatement::parseExtra(Tokenizer &aTokenizer) {
        if(this->query && this->filter) {
            static std::map<Keywords, ParseFactory> parseFactories = {
                    {Keywords::order_kw, orderFactory},
                    {Keywords::limit_kw, limitFactory},
                    {Keywords::group_kw, groupFactory},
            };

            StatusResult theResult = StatusResult();
            aTokenizer.next(); //skip to first entry

            while (aTokenizer.more() && aTokenizer.current().data != ";") {
                if (aTokenizer.current().keyword == Keywords::where_kw) {
                    aTokenizer.next();

                    if(this->proc) {
                        theResult = this->filter->parse(aTokenizer, *(this->proc->getEntity(this->tableName)));
                    }
                    else {
                        return StatusResult{Errors::badPointer};
                    }
                }
                else if(in_array<Keywords>(gJoinTypes, aTokenizer.current().keyword) ||
                        aTokenizer.current().keyword == Keywords::join_kw) {
                    theResult = parseJoin(aTokenizer);
                }
                else {
                    theResult = parseFactories[aTokenizer.current().keyword](this->query, this->filter, aTokenizer);
                }

                if (theResult != StatusResult()) {
                    break;
                }
            }

            return theResult;
        }

        return StatusResult{Errors::badPointer};
    }
}
//
// Created by Stephen on 5/12/2021.
//

#include "ChangeStatement.hpp"

namespace ECE141 {
    bool UpdateStatement::recognizes(Tokenizer &aTokenizer) {
        TokenSequence theSeq(aTokenizer);

        return theSeq.has(Keywords::update_kw).matches();
    }

    StatusResult UpdateStatement::parse(Tokenizer &aTokenizer) {
        return parseUpdate(aTokenizer);
    }

    StatusResult UpdateStatement::dispatch() {
        if(this->proc) {
            return this->proc->update(this->tableName, this->rows, this->query, this->filter);
        }

        return StatusResult{Errors::badPointer};
    }

    /* -------------------------------------------------------- */

    bool DeleteStatement::recognizes(Tokenizer &aTokenizer) {
        TokenSequence theSeq(aTokenizer);

        return theSeq.has(Keywords::delete_kw).matches();
    }

    StatusResult DeleteStatement::parse(Tokenizer &aTokenizer) {
        return parseDelete(aTokenizer);
    }

    StatusResult DeleteStatement::dispatch() {
        if(this->proc) {
            return this->proc->deleteFunc(this->tableName, this->rows, this->filter);
        }

        return StatusResult{Errors::badPointer};
    }

    /* --------------------Helper Functions--------------------- */

    //parse update command
    StatusResult UpdateStatement::parseUpdate(Tokenizer &aTokenizer) {
        if(this->query && this->filter) {
            //get the tableName
            StatusResult theResult = parseUpdateTable(aTokenizer);
            //parse set and where
            if (theResult == StatusResult()) {
                theResult = parseUpdateExtra(aTokenizer);
            }

            return theResult;
        }

        return StatusResult{Errors::badPointer};
    }

    //parse the table from update
    StatusResult UpdateStatement::parseUpdateTable(Tokenizer &aTokenizer) {
        if(this->query) {
            TokenSequence theSeq(aTokenizer);

            StatusResult theResult = theSeq.reqId(this->tableName).matches()
                                     ? StatusResult() : StatusResult{Errors::invalidArguments};

            if (theResult == StatusResult()) {
                if(this->proc) {
                    auto theEntity = this->proc->getEntity(this->tableName);

                    if (theEntity) {
                        this->query->setFrom(theEntity);
                        aTokenizer.next(theSeq.offset);
                    } else {
                        theResult = StatusResult{Errors::unknownTable};
                    }
                } else {
                    return StatusResult{Errors::badPointer};
                }
            }

            return theResult;
        }

        return StatusResult{Errors::badPointer};
    }

    //parse the set and where of the update command
    StatusResult UpdateStatement::parseUpdateExtra(Tokenizer &aTokenizer) {
        if(this->query && this->filter) {
            aTokenizer.next(); //skip to either set or where
            StatusResult theResult = StatusResult();

            while (aTokenizer.more() && aTokenizer.current().data != ";") {
                if (aTokenizer.current().keyword == Keywords::set_kw) {
                    theResult = parseSet(aTokenizer);
                } else if (aTokenizer.current().keyword == Keywords::where_kw) {
                    aTokenizer.next();

                    if(this->proc) {
                        theResult = this->filter->parse(aTokenizer, *(this->proc->getEntity(this->tableName)));
                    } else {
                        return StatusResult{Errors::badPointer};
                    }
                } else {
                    return StatusResult{Errors::invalidArguments};
                }
            }

            return theResult;
        }

        return StatusResult{Errors::badPointer};
    }

    //parse the set of an update statement
    StatusResult UpdateStatement::parseSet(Tokenizer &aTokenizer) {
        if(this->query) {
            aTokenizer.next();
            StatusResult theResult = StatusResult();

            while (aTokenizer.remaining() >= 2 && aTokenizer.current().type == TokenType::identifier) {
                theResult = parseQueryUpdate(aTokenizer);

                if (theResult != StatusResult()) {
                    return theResult;
                }
            }

            return theResult;
        }

        return StatusResult{Errors::badPointer};
    }

    //parse a single operand
    StatusResult UpdateStatement::parseQueryUpdate(Tokenizer &aTokenizer) {
        if(this->query) {
            Operand theOperand;
            theOperand.name = aTokenizer.current().data;
            aTokenizer.next();

            if (aTokenizer.current().type == TokenType::operators) {
                if (aTokenizer.current().data == "=") {
                    aTokenizer.next();
                    theOperand.ttype = aTokenizer.current().type;

                    if (aTokenizer.current().type == TokenType::identifier) {
                        theOperand.value = aTokenizer.current().data;
                        theOperand.dtype = DataTypes::varchar_type;
                    } else if (aTokenizer.current().type == TokenType::number) {
                        if (aTokenizer.current().data.find('.') != std::string::npos) {
                            theOperand.dtype = DataTypes::float_type;
                            theOperand.value = std::stof(aTokenizer.current().data);
                        } else {
                            theOperand.dtype = DataTypes::int_type;
                            theOperand.value = std::stoi(aTokenizer.current().data);
                        }
                    } else {
                        return StatusResult{Errors::invalidArguments};
                    }

                    aTokenizer.next();
                }
            }

            this->query->addUpdate(theOperand);

            return StatusResult();
        }

        return StatusResult{Errors::badPointer};
    }

    /* -------------------------------------------------------- */

    //parse delete command
    StatusResult DeleteStatement::parseDelete(Tokenizer &aTokenizer) {
        if(this->filter) {
            aTokenizer.next();
            //get the tableName
            StatusResult theResult = parseDeleteTable(aTokenizer);
            //parse set and where
            if (theResult == StatusResult()) {
                theResult = parseDeleteExtra(aTokenizer);
            }

            return theResult;
        }

        return StatusResult{Errors::badPointer};
    }

    //parsing the delete statement table
    StatusResult DeleteStatement::parseDeleteTable(Tokenizer &aTokenizer) {
        TokenSequence theSeq(aTokenizer);

        StatusResult theResult = theSeq.is(Keywords::from_kw)
                                         .thenId(this->tableName).matches() ?
                                 StatusResult() : StatusResult{Errors::invalidCommand};
        aTokenizer.next(theSeq.offset);

        return theResult;
    }

    //parse the where in a delete statement
    StatusResult DeleteStatement::parseDeleteExtra(Tokenizer &aTokenizer) {
        if(this->filter) {
            aTokenizer.next(); //skip tablename
            StatusResult theResult = StatusResult();

            if (aTokenizer.current().keyword == Keywords::where_kw) {
                aTokenizer.next();

                if(this->proc) {
                    theResult = this->filter->parse(aTokenizer, *(this->proc->getEntity(this->tableName)));
                } else {
                    return StatusResult{Errors::badPointer};
                }
            } else {
                theResult = StatusResult{Errors::invalidCommand};
            }

            return theResult;
        }

        return StatusResult{Errors::badPointer};
    }
}
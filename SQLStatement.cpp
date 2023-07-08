//
// Created by Stephen on 4/15/2021.
//

#include "SQLStatement.hpp"

namespace ECE141 {
    /* --------------------Statement Implementations------------------------ */

    StatusResult CreateTableStatement::parse(Tokenizer &aTokenizer) {
        this->entity = new Entity(aTokenizer.tokenAt(2).data);

        return parseCreate(aTokenizer);
    }

    bool CreateTableStatement::recognizes(Tokenizer &aTokenizer) {
        TokenSequence theSeq(aTokenizer);
        std::string theName;

        return theSeq.has(Keywords::create_kw)
                .then(Keywords::table_kw)
                .thenId(theName).matches();
    }

    StatusResult CreateTableStatement::dispatch() {
        if(this->proc) {
            return this->proc->createTable(this->entity);
        }

        return StatusResult{Errors::badPointer};
    }

    /* -------------------------------------------------------- */

    StatusResult DropTableStatement::parse(Tokenizer &aTokenizer) {
        this->name = aTokenizer.tokenAt(2).data;
        aTokenizer.next(3);

        return StatusResult{Errors::noError};
    }

    bool DropTableStatement::recognizes(Tokenizer &aTokenizer) {
        TokenSequence theSeq(aTokenizer);
        std::string theName;

        return theSeq.has(Keywords::drop_kw)
                .then(Keywords::table_kw)
                .thenId(theName).matches();
    }

    StatusResult DropTableStatement::dispatch() {
        if(this->proc) {
            return this->proc->dropTable(name);
        }

        return StatusResult{Errors::badPointer};
    }

    /* -------------------------------------------------------- */

    StatusResult DescribeTableStatement::parse(Tokenizer &aTokenizer) {
        this->name = aTokenizer.tokenAt(1).data;
        aTokenizer.next(2);

        return StatusResult{Errors::noError};
    }

    bool DescribeTableStatement::recognizes(Tokenizer &aTokenizer) {
        TokenSequence theSeq(aTokenizer);
        std::string theName;

        return theSeq.has(Keywords::describe_kw)
                .thenId(theName).matches();
    }

    StatusResult DescribeTableStatement::dispatch() {
        if(this->proc) {
            return this->proc->describeTable(this->name);
        }

        return StatusResult{Errors::badPointer};
    }

    /* -------------------------------------------------------- */

    StatusResult ShowTableStatement::parse(Tokenizer &aTokenizer) {
        if(aTokenizer.remaining() >= 2) {
            Token &theFirstToken = aTokenizer.peek(1);

            if(theFirstToken.keyword == Keywords::tables_kw) {
                aTokenizer.next(2);

                return StatusResult{Errors::noError};
            }
        }

        return StatusResult{Errors::invalidCommand};
    }

    bool ShowTableStatement::recognizes(Tokenizer &aTokenizer) {
        TokenSequence theSeq(aTokenizer);

        return theSeq.has(Keywords::show_kw)
                .then(Keywords::tables_kw).matches();
    }

    StatusResult ShowTableStatement::dispatch() {
        if(this->proc) {
            return this->proc->showTables();
        }

        return StatusResult{Errors::badPointer};
    }

    /* -------------------------------------------------------- */

    bool InsertTableStatement::recognizes(Tokenizer &aTokenizer) {
        TokenSequence theSeq(aTokenizer);
        std::string theName;

        return theSeq.has(Keywords::insert_kw)
                .then(Keywords::into_kw)
                .thenId(theName).matches();
    }

    StatusResult InsertTableStatement::parse(Tokenizer &aTokenizer) {
        return parseInsert(aTokenizer);
    }

    StatusResult InsertTableStatement::dispatch() {
        if(this->proc) {
            return this->proc->insert(this->name, this->values, this->attributes, this->rows);
        }

        return StatusResult{Errors::badPointer};
    }

    /* -------------------------------------------------------- */

    bool IndexStatement::recognizes(Tokenizer &aTokenizer) {
        TokenSequence theSeq(aTokenizer);

        if(theSeq.has(Keywords::show_kw)
                .then(Keywords::index_kw).matches()) {
            aTokenizer.next(); //skip to index
        }

        return theSeq.matches();
    }

    StatusResult IndexStatement::parse(Tokenizer &aTokenizer) {
        StatusResult theResult = parseIndexFields(aTokenizer);

        if(theResult == StatusResult()) {
            theResult = parseTableName(aTokenizer);
        }

        return theResult;
    }

    StatusResult IndexStatement::dispatch() {
        if(this->proc) {
            return this->proc->showIndex(this->tableName, this->attrNames);
        }

        return StatusResult{Errors::badPointer};
    }

    /* -------------------------------------------------------- */

    bool IndexesStatement::recognizes(Tokenizer &aTokenizer) {
        TokenSequence theSeq(aTokenizer);

        if(theSeq.has(Keywords::show_kw)
                .then(Keywords::indexes_kw).matches()) {
            aTokenizer.next(); //skip to indexes
        }

        return theSeq.matches();
    }

    StatusResult IndexesStatement::parse(Tokenizer &aTokenizer) {
        aTokenizer.next(); //skip to end

        return StatusResult();
    }

    StatusResult IndexesStatement::dispatch() {
        if(this->proc) {
            return this->proc->showIndexes();
        }

        return StatusResult{Errors::badPointer};
    }

    /* --------------------Helper Functions--------------------- */

    //parse the create command
    StatusResult CreateTableStatement::parseCreate(Tokenizer &aTokenizer) {
        if(this->entity) {
            aTokenizer.next(3);
            StatusResult theResult = parseCreateAttributes(aTokenizer);

            if (theResult == StatusResult()) {
                theResult = this->entity->setAutos();
            }

            return theResult;
        }

        return StatusResult{Errors::badPointer};
    }

    // parse the attributes of an attribute in create statement
    StatusResult CreateTableStatement::parseCreateAttributes(Tokenizer &aTokenizer) {
        if(this->entity) {
            StatusResult theResult = StatusResult();

            if (aTokenizer.current().data == "(") {
                bool done = false;

                while (!done) {
                    if (aTokenizer.current().type == TokenType::punctuation &&
                        aTokenizer.current().data == ")") {
                        done = true;
                    } else {
                        TokenSequence theSeq(aTokenizer);
                        Attribute *theAttribute = new Attribute();
                        std::string theName;
                        DataTypes theType;
                        std::optional<size_t> theLength;
                        std::optional<bool> theAuto, thePrimary, theNull;

                        theResult = theSeq.reqId(theName)
                                            .thenReqType(theType)
                                            .thenOptionals(theLength, theAuto, thePrimary, theNull)
                                            .matches() ?
                                    StatusResult() : StatusResult{Errors::invalidCommand};

                        if (theResult != StatusResult()) {
                            return theResult;
                        }

                        theAttribute->initialize(theName, theType, theLength, *theAuto, *thePrimary, !*theNull);

                        this->entity->addAttribute(*theAttribute);
                        aTokenizer.next(theSeq.offset + 1);
                    }
                }

                if (aTokenizer.more()) {//get to the ; token
                    aTokenizer.next();
                }
            }

            return theResult;
        }

        return StatusResult{Errors::badPointer};
    }

    /* -------------------------------------------------------- */

    //parse the insert command
    StatusResult InsertTableStatement::parseInsert(Tokenizer &aTokenizer) {
        this->name = aTokenizer.tokenAt(2).data;
        aTokenizer.next(3);
        StatusResult theResult = parseInsertAttributes(aTokenizer);

        if(theResult == StatusResult() && aTokenizer.current().keyword == Keywords::values_kw) {
            aTokenizer.next();
            theResult = parseInsertValues(aTokenizer);

            // another row of input
            while(aTokenizer.remaining() >= 1 && aTokenizer.current().type == TokenType::punctuation
                  && aTokenizer.current().data == ",") {
                aTokenizer.next(); //skip to '('
                theResult = parseInsertValues(aTokenizer);

                if(theResult != StatusResult()) {
                    return theResult;
                }
            }
        }

        return theResult;
    }

    //parse the attributes of the insert statement
    StatusResult InsertTableStatement::parseInsertAttributes(Tokenizer &aTokenizer) {
        if(aTokenizer.current().data == "(") {
            bool done = false;

            while(!done) {
                if(aTokenizer.current().type == TokenType::punctuation &&
                   aTokenizer.current().data == ")") {
                    aTokenizer.next();
                    done = true;
                }
                else {
                    TokenSequence theSeq(aTokenizer);
                    std::string theName;

                    StatusResult theResult = theSeq.reqId(theName)
                                                     .then(TokenType::punctuation).matches() ?
                                             StatusResult() : StatusResult{Errors::invalidAttribute};

                    if(theResult != StatusResult()) {
                        return theResult;
                    }

                    this->attributes.push_back(theName);
                    aTokenizer.next(theSeq.offset);
                }
            }
        }

        return StatusResult();
    }

    //parse the values of an insert statement
    StatusResult InsertTableStatement::parseInsertValues(Tokenizer &aTokenizer) {
        if(aTokenizer.current().data == "(") {
            bool done = false;
            std::vector<Value> theInput;

            while(!done) {
                if(aTokenizer.current().type == TokenType::punctuation &&
                   aTokenizer.current().data == ")") {
                    aTokenizer.next();
                    done = true;
                }
                else {
                    TokenSequence theSeq(aTokenizer);
                    Value theValue;

                    StatusResult theResult = theSeq.reqVal(theValue)
                                                     .then(TokenType::punctuation).matches() ?
                                             StatusResult() : StatusResult{Errors::invalidArguments};

                    if(theResult != StatusResult()) {
                        return theResult;
                    }

                    theInput.emplace_back(theValue);
                    aTokenizer.next(theSeq.offset);
                }
            }

            this->values.emplace_back(theInput);
        }
        else {
            return StatusResult{Errors::invalidArguments};
        }

        return StatusResult();
    }

    /* -------------------------------------------------------- */

    StatusResult IndexStatement::parseIndexFields(Tokenizer &aTokenizer) {
        aTokenizer.next(); //skip to fields

        while(aTokenizer.current().type == TokenType::identifier) {
            this->attrNames.push_back(aTokenizer.current().data);
            aTokenizer.next();

            if(aTokenizer.current().type == TokenType::punctuation) {
                if(aTokenizer.current().data == ",") {
                    aTokenizer.next();
                }
                else {
                    return StatusResult{invalidAttribute};
                }
            }
        }

        return StatusResult();
    }

    StatusResult IndexStatement::parseTableName(Tokenizer &aTokenizer) {
        TokenSequence theSeq(aTokenizer);

        StatusResult theResult =  theSeq.has(Keywords::from_kw)
                    .thenId(this->tableName).matches() ?
                    StatusResult() : StatusResult{Errors::invalidArguments};

        if(theResult == StatusResult()) {
            aTokenizer.next(theSeq.offset);

            if(aTokenizer.more()) {
                aTokenizer.next();
            }
        }

        return theResult;
    }
}

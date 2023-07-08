//
// Created by Stephen on 4/13/2021.
//

#include "DbStatement.hpp"
#include "TokenSequence.hpp"

namespace ECE141 {
    StatusResult CreateStatement::parse(Tokenizer &aTokenizer) {
        this->name = aTokenizer.tokenAt(2).data; //name should be at token 2
        aTokenizer.next(3); //skip past name

        return StatusResult{Errors::noError};
    }

    bool CreateStatement::recognizes(Tokenizer &aTokenizer) {
        TokenSequence theSeq(aTokenizer);
        std::string theName;

        return theSeq.has(Keywords::create_kw)
                .then(Keywords::database_kw)
                .thenId(theName).matches();
    }

    StatusResult CreateStatement::dispatch() {
        if(this->proc) {
            return this->proc->createDatabase(name);
        }

        return StatusResult{Errors::badPointer};
    }

    /* -------------------------------------------------------- */

    bool ShowStatement::recognizes(Tokenizer &aTokenizer) {
        TokenSequence theSeq(aTokenizer);

        return theSeq.has(Keywords::show_kw)
                .then(Keywords::databases_kw).matches();
    }

    StatusResult ShowStatement::parse(Tokenizer &aTokenizer) {
        if(aTokenizer.remaining() >= 2) {
            Token &theFirstToken = aTokenizer.peek(1);

            if(theFirstToken.keyword == Keywords::databases_kw) {
                aTokenizer.next(2);

                return StatusResult{Errors::noError};
            }
        }

        return StatusResult{Errors::invalidCommand};
    }

    StatusResult ShowStatement::dispatch() {
        if(this->proc) {
            return this->proc->showDatabases();
        }

        return StatusResult{Errors::badPointer};
    }

    /* -------------------------------------------------------- */

    bool DropStatement::recognizes(Tokenizer &aTokenizer) {
        TokenSequence theSeq(aTokenizer);
        std::string theName;

        return theSeq.has(Keywords::drop_kw)
                .then(Keywords::database_kw)
                .thenId(theName).matches();
    }

    StatusResult DropStatement::parse(Tokenizer &aTokenizer) {
        if(aTokenizer.remaining() >= 3) {
            Token &theFirstToken = aTokenizer.peek(1);

            if(theFirstToken.keyword == Keywords::database_kw) {
                Token &theSecondToken = aTokenizer.peek(2);

                if(theSecondToken.type == TokenType::identifier) {
                    this->name = theSecondToken.data;
                    aTokenizer.next(3);

                    return StatusResult{Errors::noError};
                }

                return StatusResult{Errors::noDatabaseSpecified};
            }
        }

        return StatusResult{Errors::invalidCommand};
    }

    StatusResult DropStatement::dispatch() {
        if(this->proc) {
            return this->proc->dropDatabase(this->name);
        }

        return StatusResult{Errors::badPointer};
    }

    /* -------------------------------------------------------- */

    bool UseStatement::recognizes(Tokenizer &aTokenizer) {
        TokenSequence theSeq(aTokenizer);
        std::string theName;

        return theSeq.has(Keywords::use_kw)
                .thenId(theName).matches();
    }

    StatusResult UseStatement::parse(Tokenizer &aTokenizer) {
        if(aTokenizer.remaining() >= 2) {
            Token &theFirstToken = aTokenizer.peek(1);

            if(theFirstToken.type == TokenType::identifier) {
                this->name = theFirstToken.data;
                aTokenizer.next(2);

                return StatusResult{Errors::noError};
            }

            return StatusResult{Errors::noDatabaseSpecified};
        }

        return StatusResult{Errors::invalidCommand};
    }

    StatusResult UseStatement::dispatch() {
        if(this->proc) {
            return this->proc->useDatabase(this->name);
        }

        return StatusResult{Errors::badPointer};
    }

    /* -------------------------------------------------------- */

    bool DumpStatement::recognizes(Tokenizer &aTokenizer) {
        TokenSequence theSeq(aTokenizer);
        std::string theName;

        return theSeq.has(Keywords::dump_kw)
                .then(Keywords::database_kw)
                .thenId(theName).matches();
    }

    StatusResult DumpStatement::parse(Tokenizer &aTokenizer) {
        if(aTokenizer.remaining() >= 3) {
            Token &theFirstToken = aTokenizer.peek(1);

            if(theFirstToken.keyword == Keywords::database_kw) {
                Token &theSecondToken = aTokenizer.peek(2);

                if(theSecondToken.type == TokenType::identifier) {
                    this->name = theSecondToken.data;
                    aTokenizer.next(3);

                    return StatusResult{Errors::noError};
                }

                return StatusResult{Errors::noDatabaseSpecified};
            }
        }

        return StatusResult{Errors::invalidCommand};
    }

    StatusResult DumpStatement::dispatch() {
        if(this->proc) {
            return this->proc->dumpDatabase(this->name);
        }

        return StatusResult{Errors::badPointer};
    }
}
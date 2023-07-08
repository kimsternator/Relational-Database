//
//  CommandProcessor.cpp
//  ECEDatabase
//
//  Created by rick gessner on 3/30/18.
//  Copyright © 2018 rick gessner. All rights reserved.
//

#include <iostream>
#include "Application.hpp"
#include "DbProcessor.hpp"
#include "Tokenizer.hpp"
#include <memory>
#include <vector>

namespace ECE141 {

    Application::Application(std::ostream &anOutput)
            : CmdProcessor(anOutput) {
        theDb = new DbProcessor(anOutput);
    }

    Application::~Application() {
        //if(database) delete database;
//    delete theDb;
    }

    // USE: -----------------------------------------------------

    bool Application::isKnown(Keywords aKeyword) {
        static Keywords theKnown[] = {Keywords::quit_kw, Keywords::help_kw, Keywords::version_kw};
        auto theIt = std::find(std::begin(theKnown),
                               std::end(theKnown), aKeyword);
        return theIt != std::end(theKnown);
    }

    CmdProcessor *Application::recognizes(Tokenizer &aTokenizer) {
        if (isKnown(aTokenizer.current().keyword)) {
            return this;
        }

        //STUDENT: Pass control to your next processor here...
        return this->theDb ? this->theDb->recognizes(aTokenizer) : nullptr;
    }

    StatusResult shutdown(std::ostream &anOutput) {
        anOutput << "DB::141 is shutting down.\n";

        return StatusResult(ECE141::userTerminated);
    }

    StatusResult Application::run(Statement *aStatement, const Timer &aTimer) {
        switch (aStatement->getType()) {
            case Keywords::quit_kw: {
                return shutdown(output);
            }
            case Keywords::help_kw: {
                output << "Help system ready.\n";
                break;
            }
            case Keywords::version_kw: {
                output << "Version 0.9.\n";
                break;
            }
            default: {
                break;
            }
        }

        return StatusResult{Errors::noError};
    }

    Database *Application::getDatabase() {
        return this->theDb->getDatabase();
    }

    // USE: retrieve a statement based on given text input...
    Statement *Application::makeStatement(Tokenizer &aTokenizer) {
        Token theToken = aTokenizer.current();

        if (isKnown(theToken.keyword)) {
            aTokenizer.next(); //skip ahead...
            return new Statement(theToken.keyword);
        }

        return nullptr;
    }

    //build a tokenizer, tokenize input, ask processors to handle...
    StatusResult Application::handleInput(std::istream &anInput) {
        Tokenizer theTokenizer(anInput);
        ECE141::StatusResult theResult = theTokenizer.tokenize();

        while (theResult && theTokenizer.remaining()) {
            if (auto *theProc = recognizes(theTokenizer)) {
                Timer theClock;
                theClock.start();
                if (auto *theCmd = theProc->makeStatement(theTokenizer)) {
                    theResult = theProc->run(theCmd, theClock);
                    if (theResult) theTokenizer.skipIf(';');
                    delete theCmd;
                }
            } else {
                return StatusResult{Errors::unknownCommand};
            }
        }
        return theResult;
    }
}

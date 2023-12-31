//
//  CmdProcessor.hpp
//  Database
//
//  Created by rick gessner on 3/17/19.
//  Copyright © 2019 rick gessner. All rights reserved.
//

#ifndef CmdProcessor_hpp
#define CmdProcessor_hpp

#include <stdio.h>
#include <string>
#include <algorithm>

#include "Statement.hpp"
#include "Tokenizer.hpp"
#include "Database.hpp"
#include "Timer.hpp"

namespace ECE141 {
      
  class CmdProcessor { //processor interface
  public:
            CmdProcessor(std::ostream &anOutput);
    virtual ~CmdProcessor();

    virtual CmdProcessor* recognizes(Tokenizer &aTokenizer)=0;
    virtual Statement*    makeStatement(Tokenizer &aTokenizer)=0;
    virtual StatusResult  run(Statement *aStmt, const Timer &aTimer)=0;
    virtual Database*     getDatabase()=0;

  protected:
    std::ostream &output;
    Timer clock;
  };
   
}

#endif /* CmdProcessor */

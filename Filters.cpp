//
//  Filters.cpp
//  Datatabase5
//
//  Created by rick gessner on 3/5/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#include <string>
#include <stack>

#include "Filters.hpp"
#include "Helpers.hpp"
#include "Compare.hpp"
#include "Row.hpp"

namespace ECE141 {

    using Comparitor = bool (*)(Value &aLHS, Value &aRHS);
    using Evaluate = std::function<bool(bool, bool)>;

    Logical keywordToLogical(Keywords aKeyword) {
        switch(aKeyword) {
            case Keywords::and_kw: return Logical::and_lg;
            case Keywords::or_kw: return Logical::or_lg;
            default: return Logical::no_lg;
        }
    }

    bool eval(bool aLeft, Logical aLogical, bool aRight) {
        static  std::map<Logical, Evaluate> switched {
                {Logical::and_lg, [](bool lhs, bool rhs) {return lhs && rhs;} },
                {Logical::or_lg, [](bool lhs, bool rhs) {return lhs || rhs;} },
        };

        return switched[aLogical](aLeft, aRight);
    }

    bool equals(Value &aLHS, Value &aRHS) {
        bool theResult = false;

        std::visit([&](auto const &aLeft) {
            std::visit([&](auto const &aRight) {
                theResult = isEqual(aLeft, aRight);
            }, aRHS);
        }, aLHS);

        return theResult;
    }

    bool greaterThan(Value &aLHS, Value &aRHS) {
        bool theResult = false;

        std::visit([&](auto const &aLeft) {
            std::visit([&](auto const &aRight) {
                theResult = isGreaterThan(aLeft, aRight);
            }, aRHS);
        }, aLHS);

        return theResult;
    }

    bool greaterThanEqual(Value &aLHS, Value &aRHS) {
        bool theResult = false;

        std::visit([&](auto const &aLeft) {
            std::visit([&](auto const &aRight) {
                theResult = isGreaterThanEqual(aLeft, aRight);
            }, aRHS);
        }, aLHS);

        return theResult;
    }

    bool lessThan(Value &aLHS, Value &aRHS) {
        bool theResult = false;

        std::visit([&](auto const &aLeft) {
            std::visit([&](auto const &aRight) {
                theResult = isLessThan(aLeft, aRight);
            }, aRHS);
        }, aLHS);

        return theResult;
    }

    bool lessThanEqual(Value &aLHS, Value &aRHS) {
        bool theResult = false;

        std::visit([&](auto const &aLeft) {
            std::visit([&](auto const &aRight) {
                theResult = isLessThanEqual(aLeft, aRight);
            }, aRHS);
        }, aLHS);

        return theResult;
    }

    bool notEquals(Value &aLHS, Value &aRHS) {
        bool theResult = false;

        std::visit([&](auto const &aLeft) {
            std::visit([&](auto const &aRight) {
                theResult = !isEqual(aLeft, aRight);
            }, aRHS);
        }, aLHS);

        return theResult;
    }

    static std::map<Operators, Comparitor> comparitors{
            {Operators::equal_op, equals},
            {Operators::gt_op, greaterThan},
            {Operators::gte_op, greaterThanEqual},
            {Operators::lt_op, lessThan},
            {Operators::lte_op, lessThanEqual},
            {Operators::notequal_op, notEquals},
            //STUDENT: Add more for other operators...
    };

    bool Expression::operator()(KeyValues &aList) {
        Value theLHS{this->lhs.value};
        Value theRHS{this->rhs.value};

        if (TokenType::identifier == lhs.ttype) {
            theLHS = aList[this->lhs.name]; //get row value
        }

        if (TokenType::identifier == rhs.ttype) {
            theRHS = aList[this->rhs.name]; //get row value
        }

        return comparitors.count(this->op)
               ? comparitors[this->op](theLHS, theRHS) : false;
    }

    //--------------------------------------------------------------

    Filters::Filters() {}

    Filters::Filters(const Filters &aCopy) {
    }

    Filters::~Filters() {
        //no need to delete expressions, they're unique_ptrs!
    }

    Filters &Filters::add(Expression *anExpression) {
        this->expressions.push_back(std::unique_ptr<Expression>(anExpression));
        return *this;
    }

    //compare expressions to row; return true if matches
    bool Filters::matches(KeyValues &aList) {
        //STUDENT: You'll need to add code here to deal with
        //         logical combinations (AND, OR, NOT):
        //         like:  WHERE zipcode=92127 AND age>20

        std::stack<bool> allLogicals;

        for (auto &theExpr : this->expressions) {
            if(theExpr->logic == Logical::no_lg) {
                allLogicals.push((*theExpr)(aList));
            }
            else {
                bool theTop = allLogicals.top();
                bool theNext = (*theExpr)(aList);
                allLogicals.pop();
                allLogicals.push(eval(theTop, theExpr->logic, theNext));
            }
        }

        return allLogicals.empty() ? true : allLogicals.top();
    }


    //where operand is field, number, string...
    StatusResult parseOperand(Tokenizer &aTokenizer,
                              Entity &anEntity, Operand &anOperand) {
        StatusResult theResult{noError};
        Token &theToken = aTokenizer.current();
        if (TokenType::identifier == theToken.type) {
            if (auto *theAttr = anEntity.getAttribute(theToken.data)) {
                anOperand.ttype = theToken.type;
                anOperand.name = theToken.data; //hang on to name...
                anOperand.entityId = Entity::hashString(theToken.data);
                anOperand.dtype = theAttr->getType();
            }
            else {
                anOperand.ttype = TokenType::string;
                anOperand.dtype = DataTypes::varchar_type;
                anOperand.value = theToken.data;
            }
        }
        else if (TokenType::number == theToken.type) {
            anOperand.ttype = TokenType::number;
            anOperand.dtype = DataTypes::int_type;

            if (theToken.data.find('.') != std::string::npos) {
                anOperand.dtype = DataTypes::float_type;
                anOperand.value = std::stof(theToken.data);
            }
            else
                anOperand.value = std::stoi(theToken.data);
        }
        else
            theResult.error = syntaxError;

        if (theResult)
            aTokenizer.next();

        return theResult;
    }

    //STUDENT: Add validation here...
    bool validateOperands(Operand &aLHS, Operand &aRHS, Entity &anEntity) {
        return aLHS.dtype == aRHS.dtype;
        bool theResult = false;

        if (TokenType::identifier == aLHS.ttype) { //most common case...
            //STUDENT: Add code for validation as necessary
            if(aLHS.dtype == aRHS.dtype)
                return true;
        }
        else if (TokenType::identifier == aRHS.ttype) {
            //STUDENT: Add code for validation as necessary
            return true;
        }

        return theResult;
    }

    bool Filters::addLogical(Keywords aKeyword) {
        this->expressions[this->expressions.size() - 1]->logic = keywordToLogical(aKeyword);

        return true;
    }

    //STUDENT: This starting point code may need adaptation...
    StatusResult Filters::parse(Tokenizer &aTokenizer, Entity &anEntity) {
        StatusResult theResult{noError};

        while (theResult && (2 < aTokenizer.remaining())) {
            Operand theLHS, theRHS;
            Token &theToken = aTokenizer.current();

            if (theToken.type != TokenType::identifier)
                return theResult;

            if ((theResult = parseOperand(aTokenizer, anEntity, theLHS))) {
                theToken = aTokenizer.current();

                if (theToken.type == TokenType::operators) {
                    Operators theOp = Helpers::toOperator(theToken.data);
                    aTokenizer.next();

                    if ((theResult = parseOperand(aTokenizer, anEntity, theRHS))) {
                        if (validateOperands(theLHS, theRHS, anEntity)) {
                            add(new Expression(theLHS, theOp, theRHS));

                            if(aTokenizer.current().keyword == Keywords::and_kw ||
                                    aTokenizer.current().keyword == Keywords::or_kw) {
                                if(addLogical(aTokenizer.current().keyword))
                                    aTokenizer.next();
                            }

                            if (aTokenizer.current().data == ";") {
                                break;
                            }
                        }
                        else
                            theResult.error = syntaxError;
                    }
                }
            }
            else
                theResult.error = syntaxError;
        }

        return theResult;
    }

    StatusResult Filters::filterRows(RowCollection &aRows) {
        for(int i = aRows.size() - 1; i >= 0; i--) {
            if(!this->matches(aRows[i]->getData())) {
                aRows.erase(aRows.begin() + i);
            }
        }

        return StatusResult();
    }

}


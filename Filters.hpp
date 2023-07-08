//
//  Filters.hpp
//  RGAssignment5
//
//  Created by rick gessner on 4/4/21.
//  Copyright © 2021 rick gessner. All rights reserved.
//

#ifndef Filters_h
#define Filters_h

#include <stdio.h>
#include <vector>
#include <memory>
#include <string>

#include "Errors.hpp"
#include "Tokenizer.hpp"
#include "BasicTypes.hpp"

namespace ECE141 {

    class Row;

    class Entity;

    using RowCollection = std::vector<std::unique_ptr<Row>>;
    using RowPtr = std::unique_ptr<Row>;

    struct Operand {
        Operand() {}

        Operand(std::string &aName, TokenType aType, Value &aValue, size_t anId = 0)
                : ttype(aType), dtype(DataTypes::varchar_type), name(aName),
                  value(aValue), entityId(anId) {}

        TokenType ttype; //is it a field, or const (#, string)...
        DataTypes dtype;
        std::string name;  //attr name
        Value value;
        size_t entityId;
    };

    //---------------------------------------------------

    struct Expression {
        Operand lhs;  //id
        Operand rhs;  //usually a constant; maybe a field...
        Operators op;   //=     //users.id=books.author_id
        Logical logic; //and, or, not...

        Expression(Operand &aLHSOperand, Operators anOp,
                   Operand &aRHSOperand)
                : lhs(aLHSOperand), rhs(aRHSOperand),
                  op(anOp), logic(Logical::no_lg) {}

        bool operator()(KeyValues &aList);
    };

    using Expressions = std::vector<std::unique_ptr<Expression> >;

    //---------------------------------------------------

    class Filters {
    public:

        Filters();

        Filters(const Filters &aFilters);

        ~Filters();

        size_t getCount() const { return expressions.size(); }

        bool matches(KeyValues &aList);

        Filters &add(Expression *anExpression);

        bool addLogical(Keywords aKeyword);

        StatusResult parse(Tokenizer &aTokenizer, Entity &anEntity);

        StatusResult filterRows(RowCollection &aRows);

    protected:
        Expressions expressions;
    };
}

#endif /* Filters_h */

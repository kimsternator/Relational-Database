//
// Created by Stephen on 4/13/2021.
//

#ifndef ECE141B_TOKENSEQUENCE_HPP
#define ECE141B_TOKENSEQUENCE_HPP

#include "keywords.hpp"
#include "Errors.hpp"
#include "Tokenizer.hpp"
#include "Filters.hpp"
#include "BasicTypes.hpp"
#include <map>

namespace ECE141 {
    class TokenSequence {
    public:
        TokenSequence(Tokenizer &aTokenizer) : tokenizer(aTokenizer), match(false), offset(0) {}

        static bool hasDec(std::string &aVal) {
            for(auto c: aVal)
                if(c == '.')
                    return true;

            return false;
        }

        static bool isNumber(Token aToken) {
            return TokenType::number == aToken.type;
        }

        template<typename T, size_t aSize>
        StatusResult getKeywords(T (&aKeywords)[aSize]) {
            StatusResult theResults{};
            return theResults;
        }


        TokenSequence& startWith(Keywords aKeyword) {
            return *this;
        }

        TokenSequence& has(Keywords aKeyword) {
            this->match = this->tokenizer.current().keyword == aKeyword;

            return *this;
        }

        TokenSequence& is(Keywords aKeyword) {
            this->match = this->tokenizer.current().keyword == aKeyword;

            return *this;
        }

        TokenSequence& is(TokenType aType) {
            this->match = this->tokenizer.current().type == aType;

            return *this;
        }

        TokenSequence& then(Keywords aKeyword) {
            if(this->match) {
                Token &theNext = this->tokenizer.peek(++offset);
                this->match = this->match && aKeyword == theNext.keyword;
            }

            return *this;
        }

        TokenSequence& then(TokenType aType) {
            if(this->match) {
                Token &theNext = this->tokenizer.peek(++offset);
                this->match = this->match && aType == theNext.type;
            }

            return *this;
        }

        TokenSequence& thenId(std::string &anId) {
            if(this->match) {
                Token &theNext = this->tokenizer.peek(++offset);
                this->match = this->match && TokenType::identifier == theNext.type;

                if(this->match)
                    anId = theNext.data;
            }

            return *this;
        }

        TokenSequence& reqId(std::string &anId) {
            Token &theNext = this->tokenizer.peek(++offset);
            this->match = TokenType::identifier == theNext.type;

            if(this->match)
                anId = theNext.data;

            return *this;
        }

        TokenSequence& reqVal(Value &aValue) {
            Token &theNext = this->tokenizer.peek(++offset);
            this->match = (theNext.type != TokenType::unknown);

            if(this->match) {
                if (theNext.type == TokenType::number) {
                    if (hasDec(theNext.data))
                        aValue = std::stod(theNext.data);
                    else
                        aValue = std::stoi(theNext.data);
                }
                else if(theNext.type == TokenType::identifier)
                    aValue = theNext.data;
            }

            return *this;
        }

        TokenSequence& thenReqType(DataTypes &aType) {
            static std::map<Keywords, DataTypes> types = {
                    {Keywords::integer_kw, DataTypes::int_type},
                    {Keywords::varchar_kw, DataTypes::varchar_type},
                    {Keywords::float_kw, DataTypes::float_type},
//                    {Keywords::current_timestamp_kw, DataTypes::datetime_type},
                    {Keywords::boolean_kw, DataTypes::bool_type},
            };

            if(this->match) {
                Token &theNext = this->tokenizer.peek(++offset);
                this->match = this->match && TokenType::keyword == theNext.type;

                if(this->match)
                    aType = types[theNext.keyword];
            }

            return *this;
        }


        TokenSequence& thenOptionals(std::optional<size_t> &theLength, std::optional<bool> &theAuto,
                                     std::optional<bool> &thePrimary, std::optional<bool> &theNull) {
            bool done = false;

            while(!done) {
                Token &theNext = this->tokenizer.peek(this->offset + 1);

                if(theNext.data == "," || theNext.data == ")") {
                    done = true;
                }
                else {
                    if(theNext.data == "(")
                        thenOptLength(theLength);
                    else if(theNext.keyword == Keywords::not_kw)
                        thenOptNotNull(theNull);
                    else if(theNext.keyword == Keywords::primary_kw)
                        thenOptPrimary(thePrimary);
                    else if(theNext.keyword == Keywords::auto_increment_kw)
                        thenOptAuto(theAuto);
                }
            }

            return *this;
        }

        void thenOptLength(std::optional<size_t> &theLength) {
            if(this->match) {
                if(this->then(TokenType::punctuation).matches()) {
                    Token &theLen = this->tokenizer.peek(++offset);
                    this->match = this->match && TokenType::number == theLen.type;
                    this->match = this->match && this->then(TokenType::punctuation).matches();

                    if (this->match)
                        theLength = std::stoi(theLen.data);
                }
            }
        }

        void thenOptNotNull(std::optional<bool> &theNull) {
            if(this->match) {
                Token &theNext = this->tokenizer.peek(++offset);
                this->match = this->match && Keywords::not_kw == theNext.keyword;

                if(this->match) {
                    Token &theSecond = this->tokenizer.peek(++offset);
                    this->match = this->match && Keywords::null_kw == theSecond.keyword;

                    if(this->match)
                        theNull = true; //weird behavior do opposite
                }
            }
        }

        void thenOptPrimary(std::optional<bool> &thePrimary) {
            if(this->match) {
                Token &theNext = this->tokenizer.peek(++offset);
                this->match = this->match && Keywords::primary_kw == theNext.keyword;

                if(this->match) {
                    Token &theSecond = this->tokenizer.peek(++offset);
                    this->match = this->match && Keywords::key_kw == theSecond.keyword;

                    if(this->match)
                        thePrimary = true;
                }
            }
        }

        void thenOptAuto(std::optional<bool> &theAuto) {
            if(this->match) {
                Token &theNext = this->tokenizer.peek(++offset);
                this->match = this->match && Keywords::auto_increment_kw == theNext.keyword;

                if(this->match)
                    theAuto = true;
            }
        }

        TokenSequence& thenOperand(Operand &theOperand) {
            if(this->match) {
                Token &theNext = this->tokenizer.peek(++offset);

                if(theNext.type == TokenType::identifier) {
                    theOperand.name = theNext.data;
                    theOperand.ttype = theNext.type;
                }
            }

            return *this;
        }

        TokenSequence& thenOperator(Operators &theOp) {
            if(this->match) {

            }

            return *this;
        }

        bool matches() {
            return this->match;
        }

        Tokenizer tokenizer;
        size_t offset;
        bool match;
    };
}

#endif //ECE141B_TOKENSEQUENCE_HPP
#pragma once

#include <iostream>
#include <functional>
#include <vector>

#include <fmt/format.h>

#include "..\Enums.hpp"

namespace Quark
{
    struct Token
    {
        TokenType type = TokenType::NONE; // Type of token, defined in Enums.hpp
        std::string value = "";           // value of token

        unsigned int line = 0;            // line at which the token is present(used for error msg)
        unsigned int charPos = 0;         // position from start of line at which the token is present(used for error msg)

        //internal
        bool isOptional = false;
    };

    struct TokeniserData
    {
        // Comments
        bool enableSingleLineComments = true;
        std::string singleLineComments = "//";

        bool enableMultiLineComments = true;
        std::string multiLineCommentsBegin = "/*";
        std::string multiLineCommentsEnd = "*/";

        // What can be part of a "word"
        std::function<bool(const char&)> word_rule = [](const char& c) { return isalnum(c) || c == '_'; };

        // What can be part of a "number" (not string)
        std::function<bool(const char&)> number_rule = [](const char& c) { return isdigit(c) || c == '.'; };
    };

    struct Lexer
    {
    private:
        size_t find_first_not_rule(const std::string& string, size_t index, std::function<bool(const char&)>& func)
        {
#include <algorithm>
            return std::find_if_not(string.begin() + index, string.end(), func) - string.begin();
        }

        // array of lines of code
        std::vector<std::string> codelines;

    public:
        // token array
        std::vector<Token> tokens;

        // get a line in code
        std::string& getLineInCode(unsigned int lineNumber)
        {
            return codelines[lineNumber];
        }

        // parse string
        void parse(const std::string& string, TokeniserData data = TokeniserData())
        {
            Token current;
            std::string currentLine;

            unsigned int lineNo = 0;
            unsigned int lastNewLinePos = 0;

            bool multilinecomment = false;

            for (unsigned int i = 0; i < string.size(); i++)
            {
                auto& c = string[i];

                if (data.enableSingleLineComments && c == data.singleLineComments[0])
                {
                    unsigned int j = i;

                    bool singlelinecomment = true;

                    for (; j < i + data.singleLineComments.size(); j++)
                    {
                        if (string[j] != data.singleLineComments[j - i])
                        {
                            singlelinecomment = false;
                        }
                    }

                    if (singlelinecomment)
                    {
                        for (; i < string.size(); i++)
                        {
                            if (string[i] == '\n')
                            {
                                lineNo++;
                                lastNewLinePos = i;
                                currentLine = "";
                                codelines.push_back(currentLine);
                                break;
                            }
                        }
                        continue;
                    }
                }
                if (data.enableMultiLineComments && c == data.multiLineCommentsBegin[0])
                {
                    unsigned int j = i;

                    multilinecomment = true;

                    for (; j < i + data.multiLineCommentsBegin.size(); j++)
                    {
                        if (string[j] != data.multiLineCommentsBegin[j - i])
                        {
                            multilinecomment = false;
                        }
                    }
                }
                if (data.enableMultiLineComments && multilinecomment && c == data.multiLineCommentsEnd[0])
                {
                    unsigned int j = i;

                    multilinecomment = false;

                    for (; j < i + data.multiLineCommentsEnd.size(); j++)
                    {
                        if (string[j] != data.multiLineCommentsEnd[j - i])
                        {
                            multilinecomment = true;
                        }
                    }

                    if (!multilinecomment)
                    {
                        i = j - 1;
                    }

                    continue;
                }

                if (c == '\n') { codelines.push_back(currentLine); currentLine = ""; lineNo++; lastNewLinePos = i; continue; }

                // if this part of a multi line comment, ignore
                if (multilinecomment) continue;

                currentLine += c;

                if (data.number_rule(c)) // is this start of a number
                {
                    auto lastIndex = find_first_not_rule(string, i, data.number_rule);

                    current.type = TokenType::Value;
                    current.value = string.substr(i, lastIndex - i);
                    current.charPos = i - lastNewLinePos;
                    current.line = lineNo;

                    currentLine.pop_back();
                    currentLine += current.value;

                    tokens.push_back(current);
                    current = Token{};

                    i = lastIndex - 1;
                }
                else if (c == '\"') // is this is a start of a string
                {
                    bool isSlashActive = false;
                    unsigned int j = i + 1;

                    for (;; j++)
                    {
                        if (string[j] == '\"')
                        {
                            if (!isSlashActive) // if this is a \", ignore because \ is escaping "
                            {
                                break;
                            }
                        }

                        if (string[j] == '\\')
                        {
                            isSlashActive = true;
                        }
                    }

                    current.type = TokenType::Value;
                    current.value = string.substr(i, j - i + 1);
                    current.charPos = i - lastNewLinePos;
                    current.line = lineNo;

                    currentLine.pop_back();
                    currentLine += current.value;

                    tokens.push_back(current);
                    current = Token{};

                    i = j;
                }
                else if (data.word_rule(c)) // is this is a start of a word
                {
                    auto lastIndex = find_first_not_rule(string, i, data.word_rule);

                    current.type = TokenType::Word;
                    current.value = string.substr(i, lastIndex - i);
                    current.charPos = i - lastNewLinePos;
                    current.line = lineNo;

                    currentLine.pop_back();
                    currentLine += current.value;

                    tokens.push_back(current);
                    current = Token{};

                    i = lastIndex - 1;
                }
                else // is this a special charecter
                {
                    auto t = ValidateTokenType(c);

                    if (t != TokenType::NONE)
                    {
                        current.type = t;
                        current.value = std::string(1, c);
                        current.charPos = i - lastNewLinePos;
                        current.line = lineNo;

                        tokens.push_back(current);
                        current = Token{};
                    }
                }
            }

            if(!currentLine.empty()) codelines.push_back(currentLine);
        }
    };

    // Iterator to iterate through lexer.tokens
    struct LexerIterator
    {
        Lexer& thisLexer;
        size_t iterator_position = 0;

        LexerIterator(const LexerIterator& a) : thisLexer(a.thisLexer), iterator_position(a.iterator_position) { ; }
        LexerIterator(Lexer& lexer) : thisLexer(lexer) { ; }

        LexerIterator operator=(const LexerIterator& rhs)
        {
            //if (this->thisLexer.tokens != rhs.thisLexer.tokens) { throw std::invalid_argument("Can only set equal to same lexer type"); return *this; }

            this->iterator_position = rhs.iterator_position;

            return *this;
        }

        void advance() { iterator_position++; }
        void previous() { iterator_position--; }
        Token thisToken()
        {
            if (iterator_position >= thisLexer.tokens.size()) return Token{ TokenType::NONE, "" };

            return thisLexer.tokens[iterator_position];
        }
        Token nextToken()
        {
            if (iterator_position + 1 >= thisLexer.tokens.size()) return Token{ TokenType::NONE, "" };

            return thisLexer.tokens[iterator_position + 1];
        }
        Token prevToken()
        {
            if (iterator_position == 0) return Token{ TokenType::NONE, "" };

            return thisLexer.tokens[iterator_position - 1];
        }

        bool isValid() { return iterator_position < thisLexer.tokens.size() && iterator_position >= 0; }
    };

    // this is a unused not updated function that may/may not come in use
    inline std::vector<Token> getEnclosedTokens(LexerIterator& iterator, TokenType encloseTokenBegin, TokenType encloseTokenEnd)
    {
        std::vector<Token> enclosedTokens;

        if (iterator.thisToken().type != encloseTokenBegin)
        {
            // Error: unexpected token {thisToken}
            throw std::logic_error(fmt::format("Unexpected Token {}", iterator.thisToken().type));
        }

        for (; iterator.thisToken().type != encloseTokenEnd; iterator.advance())
        {
            enclosedTokens.push_back(iterator.thisToken());
        }

        return enclosedTokens;
    }
}
#pragma once

namespace Quark
{
    enum class TokenType
    {
        NONE,

        BlockStart = '{',
        BlockEnd = '}',

        ParenStart = '(',
        ParenEnd = ')',

        BraketStart = '[',
        BraketEnd = ']',

        Value, // int A = 0; 0 is Value (or) string B = "ABC"; "ABC" is Value
        Word, // int A = 0; int and A are word tokens

        Comma = ',', // ,
        Semicolon = ';',
        Colon = ':',

        // Math
        Equal = '=',
        Plus = '+',
        Minus = '-',
        Multiply = '*',
        Divide = '/', // a / b

        // Boolean
        AND = '&',
        OR = '|',
        NOT = '!',

        // Internal
        Hashtag = '#',
    };

    enum class NodeType
    {
        NONE = 0,
        Unknown,

        // Begin
        GLOBAL,

        // OOP/Abstraction
        Class,
        Struct,
        Function,
        Namesapce,

        Word,
        Type,

        // Maths
        Value,
        ADD, // a + b
        SUB, // a - b
        MUL, // a * b
        DIV, // a / b
        POSITIVE, // +a
        NEGATIVE, // -a
        EQUALTO, // a = b

        // Boolean Logic
        AND,
        OR,
        NOT,

        // Compare Logic
        LESSTHAN, // a < b
        GREATERTHAN, // a > b
        EQUALS, // a == b
        NOTEQUALS, // a != b
    };

    inline TokenType ValidateTokenType(char c)
    {
        switch (c)
        {
        case '{':
            return TokenType::BlockStart;
            break;
        case '}':
            return TokenType::BlockEnd;
            break;
        case '(':
            return TokenType::ParenStart;
            break;
        case ')':
            return TokenType::ParenEnd;
            break;
        case '[':
            return TokenType::BraketStart;
            break;
        case ']':
            return TokenType::BraketEnd;
            break;
        case ',':
            return TokenType::Comma;
            break;
        case ';':
            return TokenType::Semicolon;
            break;
        case ':':
            return TokenType::Colon;
            break;
        case '=':
            return TokenType::Equal;
            break;
        case '+':
            return TokenType::Plus;
            break;
        case '-':
            return TokenType::Minus;
            break;
        case '*':
            return TokenType::Multiply;
            break;
        case '/':
            return TokenType::Divide;
            break;
        case '&':
            return TokenType::AND;
            break;
        case '|':
            return TokenType::OR;
            break;
        case '!':
            return TokenType::NOT;
            break;
        case '#':
            return TokenType::Hashtag;
            break;
        default:
            return TokenType::NONE;
        }
    }
}
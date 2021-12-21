#pragma once

#include <iostream>
#include <vector>

#include <initializer_list>
#include <unordered_map>
#include <functional>

#include <fmt/format.h>

#include "Lexer.hpp"
#include "..\Node.hpp"

#include "..\RaiseMessage.hpp"

namespace Quark
{
    struct Match
    {
        bool hasMatched;
        Node* nodeTree;
        RaiseMessage raisedMessage;

        Match() : nodeTree(nullptr), hasMatched(false), raisedMessage(noRaise) { ; }
        Match(bool m, Node* node) : hasMatched(m), nodeTree(node), raisedMessage(noRaise) { ; }
        Match(Node* node) : hasMatched(true), nodeTree(node), raisedMessage(noRaise) { ; }

        Match(RaiseMessage rs) : hasMatched(false), nodeTree(nullptr), raisedMessage(rs) { ; }
        Match(bool m, Node* node, RaiseMessage rs) : hasMatched(m), nodeTree(node), raisedMessage(rs) { ; }

        Match operator ||(bool rhs)
        {
            return { hasMatched || rhs, nodeTree, raisedMessage };
        }

        operator bool() { return hasMatched; }
    };
    
    inline Match nomatch = Match{};
    inline Match invalidmatch = Match{ false, nullptr, RaiseMessage{ RaiseSeverity::error, "invalidPos", "" }};
    using NodeTreeRule = std::function<Match(LexerIterator&)>;

    struct MatchStatusPair
    {
        Match match;
        LexerIterator lexerIterator;

        MatchStatusPair(LexerIterator& p_lexer) : lexerIterator(p_lexer) { ; }

        operator bool() { return match.hasMatched; }
    };

    struct MatchString
    {
        std::vector<Match> match;
        LexerIterator lexerIterator;

        MatchString(LexerIterator& p_lexer) : lexerIterator(p_lexer) { ; }

        operator bool()
        {
#include <algorithm>
            return std::all_of(match.begin(), match.end(), [](const Match& m) { return m.hasMatched; });
        }
    };

    struct MatchString_Ad
    {
        bool hasMatched = false;
        std::vector<Match> match;

        RaiseMessage raisedMessage = noRaise;

        MatchString_Ad() { ; }
        MatchString_Ad(RaiseMessage raised_msg) : raisedMessage(raised_msg) { ; }

        operator bool()
        {
#include <algorithm>
            return hasMatched && std::all_of(match.begin(), match.end(), [](const Match& m) { return m.hasMatched; });
        }
    };

    struct TokenString
    {
        std::vector<Token> tokens;
        std::unordered_map<std::string, NodeTreeRule> tokenMatchers;

        Match MatchValue(LexerIterator& lexerIterator)
        {
            if (lexerIterator.thisToken().type == TokenType::Value)
            {
                return { true, Node::create(NodeType::Value, lexerIterator.thisToken().value) };
            }

            return nomatch;
        }

        void expand(const std::string& string)
        {
            Token current;

            for (unsigned int i = 0; i < string.size(); i++)
            {
                auto& c = string[i];

                if (c == '?')
                {
                    current.isOptional = true;
                }

                if (isalpha(c) || c == '_' || c == '~')
                {
                    auto lastIndex = string.find_first_not_of("qwertyuiopasdfghjklzxcvbnm_", i + 1);

                    current.type = TokenType::NONE;
                    current.value = "";
                    auto str = string.substr(i, lastIndex - i);

                    if (c == '@')
                    {
                        current.type = TokenType::Value;
                        current.value = string.substr(1);
                    }
                    else if (str == "word")
                    {
                        current.type = TokenType::Word;
                    }
                    else
                    {
                        current.type = TokenType::NONE;
                        current.value = str;
                    }

                    tokens.push_back(current);
                    current = Token{};

                    i = lastIndex - 1;
                }
                else
                {
                    auto t = ValidateTokenType(c);

                    if (t != TokenType::NONE)
                    {
                        current.type = t;
                        current.value = std::string(1, c);

                        tokens.push_back(current);
                        current = Token{};
                    }
                }
            }
        }

        std::vector<Match> compare(LexerIterator& lexerIterator, bool breakOnFail = false)
        {
            std::vector<Match> result;

            for (unsigned int i = 0; i < tokens.size(); i++)
            {
                auto a = tokens[i];
                auto b = lexerIterator.thisToken();

                // check if this is optional
                if (i + 1 < tokens.size())
                {
                    if (tokens[i + 1].type == TokenType::Hashtag)
                    {
                        std::vector<Token> matchTokens;
                        Match matchedOption = { false, nullptr };

                        for (; i < tokens.size(); i += 2)
                        {
                            matchTokens.push_back(tokens[i]);
                            
                            if (i + 1 < tokens.size())
                            {
                                if (tokens[i + 1].type != TokenType::Hashtag)
                                {
                                    break;
                                }
                            }
                            else break;
                        }

                        for (auto& j : matchTokens)
                        {
                            if (j.type == TokenType::Word)
                            {
                                if (b.type == TokenType::Word)
                                {
                                    matchedOption = { true, Node::create(NodeType::Word, b.value) };
                                    break;
                                }
                                else
                                {
                                    matchedOption.raisedMessage.raiseSeverity = RaiseSeverity::error;
                                    matchedOption.raisedMessage.raiseData = fmt::format("exTokenType:word readTokenID:{0}", lexerIterator.iterator_position);
                                }
                            }
                            else if (j.type == TokenType::Value)
                            {
                                if (b.type == TokenType::Word && b.value == j.value)
                                {
                                    matchedOption = { true, nullptr };
                                    break;
                                }
                                else
                                {
                                    matchedOption.raisedMessage.raiseSeverity = RaiseSeverity::error;
                                    matchedOption.raisedMessage.raiseData = fmt::format("exTokenType:{1} readTokenID:{0}", lexerIterator.iterator_position, j.value);
                                }
                            }
                            else if (j.type == TokenType::NONE)
                            {
                                if (j.value != "value") throw std::invalid_argument("Custom rules cannot be optional");

                                if (lexerIterator.thisToken().type == TokenType::Value)
                                {
                                    matchedOption = { true, Node::create(NodeType::Value, lexerIterator.thisToken().value) };
                                    break;
                                }
                            }
                            else
                            {
                                if (b.type == j.type)
                                {
                                    matchedOption = { true, Node::create(NodeType::Unknown, std::string({ static_cast<char>(b.type) })) };
                                    break;
                                }
                                else
                                {
                                    matchedOption.raisedMessage.raiseSeverity = RaiseSeverity::error;
                                    matchedOption.raisedMessage.raiseData = fmt::format("exTokenType:{1} readTokenID:{0}", lexerIterator.iterator_position, std::string(1, static_cast<char>(b.type)));
                                }
                            }
                        }

                        result.push_back(matchedOption);

                        if (breakOnFail && matchedOption.raisedMessage.isSerious()) break;

                        lexerIterator.advance();

                        continue;
                    }
                }

                if (a.type == TokenType::Word)
                {
                    if (b.type == TokenType::Word)
                    {
                        result.push_back({ true, Node::create(NodeType::Word, b.value) });
                    }
                    else
                    {
                        result.push_back({ a.isOptional, nullptr });

                        if (!a.isOptional)
                        {
                            result.back().raisedMessage.raiseSeverity = RaiseSeverity::error;
                            result.back().raisedMessage.raiseData = fmt::format("exTokenType:word readTokenID:{0}", lexerIterator.iterator_position);

                            if (breakOnFail) break;
                        }
                    }

                    lexerIterator.advance();
                }
                else if (a.type == TokenType::Value)
                {
                    if (b.type == TokenType::Word && b.value == a.value)
                    {
                        result.push_back({ true, nullptr });
                    }
                    else
                    {
                        result.push_back({ a.isOptional, nullptr });

                        if (!a.isOptional)
                        {
                            result.back().raisedMessage.raiseSeverity = RaiseSeverity::error;
                            result.back().raisedMessage.raiseData = fmt::format("exTokenType:value readTokenID:{0}", lexerIterator.iterator_position);

                            if (breakOnFail) break;
                        }
                    }

                    lexerIterator.advance();
                }
                else if (a.type == TokenType::NONE)
                {
                    if (a.value == "value")
                    {
                        result.push_back(MatchValue(lexerIterator) || a.isOptional);
                    }
                    else
                    {
                        auto m = tokenMatchers[a.value](lexerIterator);

                        result.push_back(m || a.isOptional);

                        if (!result.back().hasMatched && breakOnFail) break;
                    }
                }
                else
                {
                    if (b.type == a.type)
                    {
                        result.push_back({ true, Node::create(NodeType::Unknown, std::string({ static_cast<char>(b.type) })) });
                    }
                    else
                    {
                        result.push_back({ a.isOptional, nullptr });

                        if (!a.isOptional)
                        {
                            result.back().raisedMessage.raiseSeverity = RaiseSeverity::error;
                            result.back().raisedMessage.raiseData = fmt::format("exTokenType:{1} readTokenID:{0}", lexerIterator.iterator_position, std::string(1, static_cast<char>(b.type)));
                            
                            if (breakOnFail) break;
                        }
                    }

                    lexerIterator.advance();
                }
            }

            return result;
        }
    };
    struct TreeRule__Advanced
    {
        std::string name;
        NodeTreeRule matcher;

        TreeRule__Advanced(const std::string& matcher_name) : name(matcher_name) { ; }
        TreeRule__Advanced(const std::string& matcher_name, NodeTreeRule token_matcher) : name(matcher_name), matcher(token_matcher) { ; }

        TreeRule__Advanced operator=(NodeTreeRule token_matcher)
        {
            this->matcher = token_matcher;
            return *this;
        }
    };
    
    inline std::unordered_map<std::string, NodeTreeRule> defaultTokenMatchers;
    using TreeRule = std::pair<std::string, NodeTreeRule>;

    struct TokenStringIterator
    {
        TokenString& thisTokenString;
        size_t iterator_position = 0;

        TokenStringIterator(const TokenStringIterator& a) : thisTokenString(a.thisTokenString), iterator_position(a.iterator_position) { ; }
        TokenStringIterator(TokenString& tokenString) : thisTokenString(tokenString) { ; }

        TokenStringIterator operator=(const TokenStringIterator& rhs)
        {
            //if (this->thisLexer.tokens != rhs.thisLexer.tokens) { throw std::invalid_argument("Can only set equal to same lexer type"); return *this; }

            this->iterator_position = rhs.iterator_position;

            return *this;
        }

        void advance() { iterator_position++; }
        void previous() { iterator_position--; }
        Token thisToken()
        {
            if (iterator_position >= thisTokenString.tokens.size()) return Token{ TokenType::NONE, "" };

            return thisTokenString.tokens[iterator_position];
        }
        Token nextToken()
        {
            if (iterator_position + 1 >= thisTokenString.tokens.size()) return Token{ TokenType::NONE, "" };

            return thisTokenString.tokens[iterator_position + 1];
        }
        Token prevToken()
        {
            if (iterator_position == 0) return Token{ TokenType::NONE, "" };

            return thisTokenString.tokens[iterator_position - 1];
        }
    };

    inline Match failMatch(const RaiseMessage& rm) { return Match{ false, nullptr, rm }; }

    inline std::vector<Token> getEnclosedTokens(TokenStringIterator& iterator, TokenType encloseTokenBegin, TokenType encloseTokenEnd)
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

inline Quark::TreeRule__Advanced operator"" _rule(const char* str, size_t size)
{
    return Quark::TreeRule__Advanced(std::string(str, size));
}
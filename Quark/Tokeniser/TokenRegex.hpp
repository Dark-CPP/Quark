#pragma once

#include <iostream>
#include <sstream>

#include "Match.hpp"

namespace Quark
{
    namespace defaults
    {
        // indicate that this is a default rule
        inline auto defaultRaise = RaiseMessage{ RaiseSeverity::no_severity, Raise::DataString{"defaultRule"}, "" };

        inline NodeTreeRule defaultSplitRule = [](LexerIterator& lexerIt) -> Match { return { true,  nullptr, defaultRaise }; };
        inline NodeTreeRule defaultStartRule = [](LexerIterator& lexerIt) -> Match { return { true,  nullptr, defaultRaise }; };
        inline NodeTreeRule defaultEndRule   = [](LexerIterator& lexerIt) -> Match { return { false, nullptr, defaultRaise }; };
    }

    using TreeRuleList = std::initializer_list<TreeRule__Advanced>;

    struct TokenRegex
    {
        LexerIterator& lexerIterator;
        std::vector<TreeRule__Advanced> rule_list;

        TokenRegex(LexerIterator& li) : lexerIterator(li) { ; }
        TokenRegex(const TokenRegex& tr) : lexerIterator(tr.lexerIterator), rule_list(tr.rule_list) { ; }

        // the compareString is the syntax
        inline std::vector<Match> match_regex(const std::string& compareString, bool breakOnFail = false)
        {
            TokenString ts;
            ts.expand(compareString);
            ts.tokenMatchers = defaultTokenMatchers;

            for (auto& i : rule_list)
            {
                ts.tokenMatchers.insert(std::pair<std::string, NodeTreeRule>(i.name, i.matcher));
            }

            return ts.compare(lexerIterator, breakOnFail);
        }

        // match a rule
        inline Match match_rule(NodeTreeRule rule) { auto res = rule(lexerIterator); return res; }

        // choose which syntax is matching
        inline std::vector<MatchString> choose_regex(const std::vector<std::string>& comparelist)
        {
            std::vector<MatchString> compareResults;

            for (auto& i : comparelist)
            {
                MatchString c(lexerIterator);
                TokenRegex tr(c.lexerIterator); tr.rule_list = rule_list;

                c.match = tr.match_regex(i);

                compareResults.push_back(c);
            }

            return compareResults;
        }

        // choose which rule matches
        inline std::vector<MatchStatusPair> choose_rule(std::initializer_list<NodeTreeRule> compareRuleList)
        {
            std::vector<MatchStatusPair> compareResults;

            for (auto& i : compareRuleList)
            {
                MatchStatusPair c(lexerIterator);
                TokenRegex tr(c.lexerIterator); tr.rule_list = rule_list;

                c.match = tr.match_rule(i);

                compareResults.push_back(c);
            }

            return compareResults;
        }

        /// <summary>
        /// Repeats a rule seperated by a Split rule with start and end points defined
        /// </summary>
        inline MatchString_Ad repeat_rule(
            NodeTreeRule repeat_rule,
            NodeTreeRule Split = defaults::defaultSplitRule,
            NodeTreeRule Start = defaults::defaultStartRule,
            NodeTreeRule End = defaults::defaultEndRule
        )
        {
            MatchString_Ad ms;

            auto start_match = this->match_rule(Start);

            // Start rule did not match
            if (!start_match.hasMatched) return RaiseMessage{ RaiseSeverity::error, start_match.raisedMessage.raiseData + " repeatFail:startRuleFail", ""};

            while (true)
            {
                auto repeat_match = this->match_rule(repeat_rule);

                auto split_or_end_match = this->choose_rule({ Split, End });

                if (repeat_match.hasMatched)
                {
                    ms.match.push_back(repeat_match);
                }
                else
                {
                    // split rule matched, but repeat rule did not
                    // eg. 10 + + 10
                    ms.hasMatched = false;
                    ms.raisedMessage = repeat_match.raisedMessage;
                    ms.raisedMessage.raiseData += " repeatFail:repeatRuleFail";
                    return ms;
                }

                // check if end rule matched
                if (split_or_end_match[1])
                {
                    ms.hasMatched = true;
                    ms.raisedMessage.raiseSeverity = RaiseSeverity::no_severity;
                    ms.raisedMessage.raiseData += " repeatFail:noFail"; // can be optimised
                    lexerIterator = split_or_end_match[1].lexerIterator;
                    return ms;
                }
                
                // check if split rule matched
                if (split_or_end_match[0])
                {
                    // split rule matched, but repeat rule did not
                    // eg. 10 + + 10
                    if (!repeat_match.hasMatched)
                    {
                        ms.hasMatched = false;
                        ms.raisedMessage = repeat_match.raisedMessage;
                        ms.raisedMessage.raiseData += " repeatFail:repeatRuleFail";
                        return ms;
                    }

                    // raiseData contains defaultRule flag, this rule is default
                    if (split_or_end_match[0].match.raisedMessage.isFlagSet("defaultRule")) continue;

                    lexerIterator = split_or_end_match[0].lexerIterator;
                    ms.match.push_back(split_or_end_match[0].match);
                }
                else
                {
                    // raiseData contains defaultRule flag, this rule is default
                    if (split_or_end_match[0].match.raisedMessage.isFlagSet("defaultRule"))
                    {
                        ms.hasMatched = false;
                        ms.raisedMessage = repeat_match.raisedMessage;
                        ms.raisedMessage.raiseData += " repeatFail:repeatRuleFail";
                        return ms;
                    }

                    ms.hasMatched = true;
                    ms.raisedMessage = split_or_end_match[0].match.raisedMessage;
                    ms.raisedMessage.raiseData += " repeatFail:splitRuleFail";
                    return ms;
                }
            }

            return ms;
        }
    };

    inline bool hasAllMatched(const std::vector<Match>& matches)
    {
#include <algorithm>
        return std::all_of(matches.begin(), matches.end(), [](const Match& m) { return m.hasMatched; });
    }

    inline Match getFailedMatch(const std::vector<Match>& matches)
    {
#include <algorithm>
        return *std::find_if(matches.begin(), matches.end(), [](const Match& m) { return !m.hasMatched; });
    }
}

// unused
inline Quark::NodeTreeRule operator"" _rxrule(const char* str, size_t size)
{
    auto regex = std::string(str, size);

    return [regex]lambda_nodeTreeRule({ return tokenRegex.match_regex(regex)[0]; });
}
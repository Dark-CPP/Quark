#include "RulesNT.h"

#include "Utils.hpp"

namespace Quark
{
	namespace abc_Grammar
	{
		namespace treeBuilder
		{
			decl_nodeTreeRule(MathFunctionCallArgsRule) // match (x, y, z)
			{
				init_nodeTreeRule();

				auto repeatMatch = tokenRegex.repeat_rule(MathExpressionRule, ","_rxrule, "("_rxrule, ")"_rxrule);
				auto ErrorData = repeatMatch.raisedMessage.extractData();

				if (ErrorData["repeatFail"] != "noFail")
				{
					if (ErrorData["repeatFail"] == "startRuleFail")
					{
						return repeatMatch.raisedMessage;
					}

					repeatMatch.raisedMessage.raiseData += " functionCallArgsComfirmFail";

					return repeatMatch.raisedMessage;
				}

				auto lhs = unpack_repeat_rule_ig_split(repeatMatch, ",");

				return lhs;
			}

			decl_nodeTreeRule(MathFunctionCallRule)
			{
				init_nodeTreeRule();

				tokenRegex.rule_list = { "args"_rule=MathFunctionCallArgsRule };

				auto matched = tokenRegex.match_regex("word args"); // match f(x, y, z)

				if (hasAllMatched(matched))
				{
					auto& temp = matched[1].nodeTree;
					temp->type = NodeType::Function;
					temp->value = matched[0].nodeTree->value;

					return temp;
				}
				else
				{
					if (!matched[0])
					{
						return matched[0].raisedMessage;
					}
					else
					{
						return matched[1].raisedMessage;
					}
				}

				return nomatch;
			}

			decl_nodeTreeRule(MathBracketRule)
			{
				init_nodeTreeRule();

				tokenRegex.rule_list = { "math_expr"_rule = treeBuilder::MathExpressionRule };

				auto matched = tokenRegex.match_regex("(math_expr)", true);

				if (!hasAllMatched(matched))
				{
					auto failedMatch = getFailedMatch(matched);

					if (failedMatch.raisedMessage.isValuePresent("exprFail"))
					{
						failedMatch.raisedMessage.raiseData = "bracketComfirmFail";
					}

					return failMatch(failedMatch.raisedMessage);
				}

				return matched[1];
			}

			Match MathValueTypeRule(LexerIterator& lexerIterator) // Match a number/variable/function
			{
				init_nodeTreeRule();
				
				tokenRegex.rule_list = { "function"_rule = MathFunctionCallRule };

				auto matchedFunction = tokenRegex.choose_regex({ "function" })[0];

				if (matchedFunction)
				{
					lexerIterator = matchedFunction.lexerIterator;

					return matchedFunction.match[0];
				}
				else if (matchedFunction.match[0].raisedMessage.isValuePresent("functionCallArgsComfirmFail"))
				{
					return matchedFunction.match[0].raisedMessage;
				}
				
				auto matched = tokenRegex.match_regex("word#value")[0];

				if (!matched)
				{
					auto Data = matched.raisedMessage.extractData();

					auto errorTokenID = std::stoi(Data["readTokenID"]);
					auto errorToken = lexerIterator.thisLexer.tokens[errorTokenID];

					matched.raisedMessage.raisedMessage = 
						fmt::format("{0}Syntax Error[unexpected token]: Expected expression before \'{1}\' token\n"
									, matched.raisedMessage.errorPointerString(lexerIterator, errorTokenID), errorToken.value);

					matched.raisedMessage.raiseData = "";
				}

				return matched;
			}

			decl_nodeTreeRule(MathBracketTypeRule)
			{
				init_nodeTreeRule();

				auto matches = tokenRegex.choose_rule({ MathBracketRule, MathValueTypeRule });

				if (matches[0])
				{
					lexerIterator = matches[0].lexerIterator;
					return matches[0].match;
				}
				if (matches[1])
				{
					lexerIterator = matches[1].lexerIterator;
					return matches[1].match;
				}

				std::vector<RaiseMessage> raisedMsgs = { matches[0].match.raisedMessage, matches[1].match.raisedMessage };

				if (raisedMsgs[0].isValuePresent("bracketComfirmFail")) // ( token matched but math_expr did not
				{
					return failMatch(raisedMsgs[0]);
				}
				else // ( did not match i.e most likely valuetype error
				{
					return failMatch(raisedMsgs[1]);
				}

				return nomatch;
			}

			Match MathTermRule(LexerIterator& lexerIterator)
			{
				init_nodeTreeRule();

				auto repeatMatch = tokenRegex.repeat_rule(MathBracketTypeRule, []lambda_nodeTreeRule({ return tokenRegex.match_regex("*#/")[0]; }));
				auto ErrorData = repeatMatch.raisedMessage.extractData();

				if (ErrorData["repeatFail"] != "noFail" && ErrorData["repeatFail"] != "splitRuleFail")
				{
					repeatMatch.raisedMessage.raiseData += " termFail";

					return repeatMatch.raisedMessage;
				}

				auto lhs = unpack_repeat_rule(repeatMatch, NodeType::MUL, "*", NodeType::DIV, "/");

				return lhs;
			}
			Match MathExpressionRule(LexerIterator& lexerIterator)
			{
				init_nodeTreeRule();

				// split rule: match + or -
				auto repeatMatch = tokenRegex.repeat_rule(MathTermRule, []lambda_nodeTreeRule({ return tokenRegex.match_regex("+#-")[0]; }));
				auto ErrorData = repeatMatch.raisedMessage.extractData();

				if (ErrorData["repeatFail"] != "noFail" && ErrorData["repeatFail"] != "splitRuleFail")
				{
					repeatMatch.raisedMessage.raiseData += " exprFail";

					return repeatMatch.raisedMessage;
				}

				auto lhs = unpack_repeat_rule(repeatMatch, NodeType::ADD, "+", NodeType::SUB, "-");

				return lhs;
			}
		}
	}
}
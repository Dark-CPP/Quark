#pragma once

#include <iostream>
#include <vector>

#include <fmt/format.h>

#include "../../Quark/Quark.hpp"

namespace Quark
{
	namespace abc_Grammar
	{
		namespace treeBuilder
		{
			decl_nodeTreeRule(MathFunctionCallArgsRule);
			decl_nodeTreeRule(MathFunctionCallRule);
			
			decl_nodeTreeRule(MathValueTypeRule);   // x
			decl_nodeTreeRule(MathBracketRule);     // (x + y)
			decl_nodeTreeRule(MathBracketTypeRule); // (x + y) & x
			decl_nodeTreeRule(MathTermRule);        // x * y & x / y
			decl_nodeTreeRule(MathExpressionRule);  // x + y & x - y

			inline void Init()
			{
				defaultTokenMatchers.insert(TreeRule("math_value", MathValueTypeRule));
				//defaultTokenMatchers.insert(TreeRule("functioncall", FunctionRule));
			}
		}

		inline auto buildTree(Quark::Lexer& lexer)
		{
			treeBuilder::Init();

			Quark::LexerIterator li(lexer);

			Quark::TokenRegex tr(li);
			tr.rule_list = { "math_expr"_rule = treeBuilder::MathExpressionRule };

			auto matched = tr.match_regex("math_expr?;")[0];

			if (!matched)
			{
				// has reached End of File/String
				if (matched.raisedMessage.isFlagSet("invalidPos"))
				{
					fmt::print("{0}Syntax Error[unexpected EOF]: Expected expression after \'{1}\' token\n"
						, matched.raisedMessage.errorPointerString(li, lexer.tokens.size() - 1), lexer.tokens.back().value);
				}
			}

			return matched;
		}
	}
}
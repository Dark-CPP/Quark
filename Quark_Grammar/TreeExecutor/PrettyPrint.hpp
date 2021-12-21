#pragma once

#include <iostream>
#include <iomanip>
#include <vector>

#include <fmt/format.h>
#include <scn/scn.h>

#include "../../Quark/Quark.hpp"

namespace Quark
{
	namespace abc_Grammar
	{
		namespace treePrinter
		{
			inline unsigned int indent = 0;

			std::string setw(unsigned int i) { return std::string(3 * i, ' '); }

			decl_expansionRule(MathValuePrintRule)
			{
				out << "\n" << setw(indent) << fmt::format("NodeType: Value({})", node->value);

				return noRaise;
			}
			decl_expansionRule(MathWordPrintRule)
			{
				out << "\n" << setw(indent) << fmt::format("NodeType: Word({})", node->value);

				return noRaise;
			}

			decl_expansionRule(MathAddPrintRule)
			{
				out << "\n" << setw(indent++) << "NodeType: Add(+)";
				node->left()->expand(out);
				node->right()->expand(out);
				out << "\n"; indent--;

				return noRaise;
			}
			decl_expansionRule(MathSubPrintRule)
			{
				out << "\n" << setw(indent++) << "NodeType: Subtract(-)";
				node->left()->expand(out);
				node->right()->expand(out);
				out << "\n"; indent--;

				return noRaise;
			}
			decl_expansionRule(MathMulPrintRule)
			{
				out << "\n" << setw(indent++) << "NodeType: Multiply(*)";
				node->left()->expand(out);
				node->right()->expand(out);
				out << "\n"; indent--;

				return noRaise;
			}
			decl_expansionRule(MathDivPrintRule)
			{
				out << "\n" << setw(indent++) << "NodeType: Division(/)";
				node->left()->expand(out);
				node->right()->expand(out);
				out << "\n"; indent--;

				return noRaise;
			}

			decl_expansionRule(MathFunctionCallPrintRule)
			{
				out << "\n" << setw(indent++) << fmt::format("NodeType: FunctionCall({})", node->value);
				
				for (auto& arg_node : node->node_list)
				{
					arg_node->expand(out);
				}

				out << "\n"; indent--;

				return noRaise;
			}
		}

		void printTree()
		{
			ExpansionRuleList list;

			add_expansionRule(list, NodeType::Value, treePrinter::MathValuePrintRule);
			add_expansionRule(list, NodeType::Word, treePrinter::MathWordPrintRule);
			add_expansionRule(list, NodeType::ADD, treePrinter::MathAddPrintRule);
			add_expansionRule(list, NodeType::SUB, treePrinter::MathSubPrintRule);
			add_expansionRule(list, NodeType::MUL, treePrinter::MathMulPrintRule);
			add_expansionRule(list, NodeType::DIV, treePrinter::MathDivPrintRule);
			add_expansionRule(list, NodeType::Function, treePrinter::MathFunctionCallPrintRule);

			ExpansionRules = list;
		}
	}
}
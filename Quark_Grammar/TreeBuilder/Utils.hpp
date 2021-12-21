#pragma once

#include <iostream>

#include "../../Quark/Quark.hpp"

namespace Quark
{
	namespace abc_Grammar
	{
		namespace treeBuilder
		{
			inline Match unpack_repeat_rule(MatchString_Ad& repeatmatch, NodeType a_type, std::string a_value, NodeType b_type, std::string b_value)
			{
				NodePtr lhs = repeatmatch.match[0].nodeTree;

				for (size_t i = 1; i < repeatmatch.match.size(); i++)
				{
					auto& match = repeatmatch.match[i];

					if (match.nodeTree->value == a_value)
					{
						lhs = Node::create(a_type, lhs, nullptr);
					}
					else if (match.nodeTree->value == b_value)
					{
						lhs = Node::create(b_type, lhs, nullptr);
					}
					else
					{
						lhs->node_list[1] = match.nodeTree;
					}
				}

				return lhs;
			}

			/// <summary>
			/// Ignore single token splits while making into a node tree, a_value = single token string value
			/// </summary>
			inline Match unpack_repeat_rule_ig_split(MatchString_Ad& repeatmatch, std::string a_value)
			{
				NodePtr lhs = Node::create(NodeType::Unknown);

				for (size_t i = 0; i < repeatmatch.match.size(); i++)
				{
					auto& match = repeatmatch.match[i];

					if (match.nodeTree->value != a_value)
					{
						lhs->node_list.push_back(match.nodeTree);
					}
				}

				return lhs;
			}
		}
	}
}
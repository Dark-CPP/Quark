#pragma once

#include <iostream>
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>

#include <fmt/format.h>

#include "Enums.hpp"
#include "RaiseMessage.hpp"

namespace Quark
{
    struct Node;

    using NodePtr = std::shared_ptr<Node>;

    using ExpansionRule = std::function<RaiseMessage(NodePtr, std::ostream&)>;
    using ExpansionRuleList = std::unordered_map<NodeType, ExpansionRule>;
    inline ExpansionRuleList ExpansionRules;

    // Use Smart Pointers
    struct Node : std::enable_shared_from_this<Node>
    {
        NodeType type;
        std::string value;
        std::vector<std::shared_ptr<Node>> node_list;

        Node() : type(NodeType::NONE) { ; }
        Node(NodeType node_type) : type(node_type) { ; }

        Node(NodeType node_type, const std::string& node_value)
        {
            type = node_type;
            value = node_value;
        }

        Node(NodeType node_type, NodePtr lhs, NodePtr rhs)
        {
            type = node_type;

            node_list.push_back(lhs);
            node_list.push_back(rhs);
        }

        Node(NodeType node_type, const std::vector<NodePtr>& node_array)
        {
            type = node_type;

            node_list = node_array;
        }

        static NodePtr create(NodeType node_type, const std::string& node_value)
        {
            return std::make_shared<Node>(node_type, node_value);
        }

        static NodePtr create(NodeType node_type, NodePtr lhs, NodePtr rhs)
        {
            return std::make_shared<Node>(node_type, lhs, rhs);
        }

        static NodePtr create(NodeType node_type, const std::vector<NodePtr>& node_array)
        {
            return std::make_shared<Node>(node_type, node_array);
        }

        static NodePtr create(NodeType node_type)
        {
            return std::make_shared<Node>(node_type);
        }

        RaiseMessage expand(std::ostream& os)
        {
            return ExpansionRules[type](shared_from_this(), os);
        }

        NodePtr left() { return node_list[0]; }
        NodePtr right() { return node_list[1]; }
    };
}


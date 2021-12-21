#pragma once

#include <iostream>
#include <functional>
#include <unordered_map>
#include <vector>

#include <fmt/format.h>

#include "Enums.hpp"

namespace Quark
{
    struct Node;

    using ExpansionRule = std::function<void(Node*, std::ostream&)>;
    using ExpansionRuleList = std::unordered_map<NodeType, ExpansionRule>;
    inline ExpansionRuleList ExpansionRules;

    struct Node
    {
        NodeType type;
        std::string value;
        std::vector<Node*> node_list;

        Node() : type(NodeType::NONE) { ; }
        Node(NodeType node_type) : type(node_type) { ; }

        Node(NodeType node_type, const std::string& node_value)
        {
            type = node_type;
            value = node_value;
        }

        Node(NodeType node_type, Node* lhs, Node* rhs)
        {
            type = node_type;

            node_list.push_back(lhs);
            node_list.push_back(rhs);
        }

        Node(NodeType node_type, const std::vector<Node*>& node_array)
        {
            type = node_type;

            node_list = node_array;
        }

        static Node* create(NodeType node_type, const std::string& node_value)
        {
            return new Node(node_type, node_value);
        }

        static Node* create(NodeType node_type, Node* lhs, Node* rhs)
        {
            return new Node(node_type, lhs, rhs);
        }

        static Node* create(NodeType node_type, const std::vector<Node*>& node_array)
        {
            return new Node(node_type, node_array);
        }

        static Node* create(NodeType node_type)
        {
            return new Node(node_type);
        }

        void expand(std::ostream& os)
        {
            ExpansionRules[type](this, os);
        }

        Node* left() { return node_list[0]; }
        Node* right() { return node_list[1]; }
    };
}


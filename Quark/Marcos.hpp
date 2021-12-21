#pragma once

#define decl_nodeTreeRule(rule) Match rule(Quark::LexerIterator& lexerIterator)
#define init_nodeTreeRule() if (!lexerIterator.isValid()) return Quark::invalidmatch; auto tokenRegex = Quark::TokenRegex{ lexerIterator };
#define lambda_nodeTreeRule(body) (Quark::LexerIterator& lexerIterator) -> Quark::Match { init_nodeTreeRule() body }

#define decl_expansionRule(rule) void rule(Node* node, std::ostream& out)
#define decl_expansionRuleArgs(rule, args) void rule(Node* node, std::ostream& out, args)
#define expansionRulePair std::pair<NodeType, ExpansionRule>
#define add_expansionRule(map, type, rule) map.insert(expansionRulePair(type, rule))
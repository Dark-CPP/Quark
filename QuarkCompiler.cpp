// takes input form user and then parses it into a node tree
// (if it fails to parse then an error is displayed)
// if it is successful then the node tree is printed

#include <iostream>

#include <fmt/format.h>

#include "Quark/Quark.hpp"
#include "Quark_Grammar/Grammar.hpp"

int main()
{
	Quark::Lexer lexer;

	std::string line;

	std::cout << "Enter a expression: ";

	std::getline(std::cin, line);

	Quark::TokeniserData tdata{};

	// "word" is like a name, this rule is to determine whether a string is a name for something
	// example, int a = 0;
	// here a is the name of variable i.e it is a word
	tdata.word_rule = [](const char& c) { return isalnum(c) || c == '_' || c == '.' || c == ':'; };

	lexer.parse(line, tdata);

	auto matched = Quark::abc_Grammar::buildTree(lexer);

	// if there was an error
	if (matched.raisedMessage.isMessageRaised())
	{
		fmt::print("{}\n", matched.raisedMessage.raisedMessage);

		if (matched.raisedMessage.isSerious())
		{
			return 0;
		}
	}

	std::cout << "\nCreated Node Tree:";

	// initialize Node Tree for printing
	Quark::abc_Grammar::printTree();

	matched.nodeTree->expand(std::cout);

	std::cout << "\n";

	return 0;
}

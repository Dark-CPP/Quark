#pragma once

#include <iostream>
#include <vector>

#include <fmt/format.h>

#include "Tokeniser/Lexer.hpp"

namespace Quark
{
	enum class RaiseSeverity
	{
		no_severity,
		warning,
		deprecated,
		error,
	};

	struct RaiseMessage
	{
		RaiseSeverity raiseSeverity; // Serverity of error
		std::string raiseData;       // flags raised on error
		std::string raisedMessage;   // message that is to be printed

		bool isSerious() { return raiseSeverity == RaiseSeverity::error; }
		bool isMessageRaised() { return raiseSeverity != RaiseSeverity::no_severity; }

		// point at a charecter i.e print
		// (x + )
		//      ^
		// Some error message
		std::string errorPointerString(LexerIterator lexerIterator, unsigned int tokenID)
		{
			auto& lexer = lexerIterator.thisLexer;
			auto& token = lexer.tokens[tokenID];
			auto& line  = lexer.getLineInCode(token.line);

			return fmt::format("{0}\n{1}^\n", line, std::string(token.charPos, ' '));
		}

		// Check if a flag is present
		bool isValuePresent(const std::string& value) { return raiseData.find(value) != std::string::npos; }

		std::unordered_map<std::string, std::string> extractData()
		{
			std::unordered_map<std::string, std::string> data;

			std::pair<std::string, std::string> dataPair;
			bool isReadingDataName = true;

			for (char& c : raiseData)
			{
				if (isspace(c))
				{
					if (!dataPair.first.empty())
					{
						data.insert(dataPair);
						dataPair = std::pair<std::string, std::string>();
					}
					isReadingDataName = true;
					continue;
				}

				if (c == ':')
				{
					isReadingDataName = false;
				}

				if (isalnum(c))
				{
					if (isReadingDataName)
					{
						dataPair.first += c;
					}
					else
					{
						dataPair.second += c;
					}
				}
			}

			if (!dataPair.first.empty())
			{
				data.insert(dataPair);
			}

			return data;
		}
	};
	
	inline auto noRaise = RaiseMessage{ RaiseSeverity::no_severity, "", ""};
}
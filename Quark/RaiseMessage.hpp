#pragma once

#include <iostream>
#include <vector>

#include <fmt/format.h>

#include "Tokeniser/Lexer.hpp"

namespace Quark
{
	namespace Raise
	{
		struct DataString
		{
			private:
				std::unordered_map<std::string, std::string> data;

				void extractData(const std::string raiseData)
				{
					std::pair<std::string, std::string> dataPair;
					bool isReadingDataName = true;

					for (auto& c : raiseData)
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
				}
			public:
				DataString() = default;

				DataString(const std::string& string)
				{
					extractData(string);
				}

				void operator=(const std::string& string)
				{
					data.clear();

					extractData(string);
				}

				DataString operator+(const std::string& string)
				{
					DataString ds;

					ds.data = data;

					ds.extractData(string);

					return ds;
				}

				void operator+=(const std::string& string)
				{
					extractData(string);
				}

				bool find(const std::string& key)
				{
					return data.find(key) != data.end();
				}

				std::string operator [](const std::string& key)
				{
					return data[key];
				}

				operator std::unordered_map<std::string, std::string>&()
				{
					return data;
				}
		};
	}

	enum class RaiseSeverity
	{
		no_severity,
		warning,
		deprecated,
		error,
	};

	struct RaiseMessage
	{
		RaiseSeverity raiseSeverity;       // Serverity of error
		Raise::DataString raiseData;       // flags raised on error
		std::string raisedMessage;         // message that is to be printed

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
		bool isFlagSet(const std::string& value) { return raiseData.find(value); }
	};
	
	inline auto noRaise = RaiseMessage{ RaiseSeverity::no_severity, Raise::DataString{""}, "" };
}
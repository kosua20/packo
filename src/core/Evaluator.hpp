#pragma once
#include "core/Common.hpp"
#include "core/Graph.hpp"


class ErrorContext {
public:

	void addError(const std::string& message, const Node* node = nullptr, int slot = -1);

	bool hasErrors() const { return !_errors.empty();}

	std::string summarizeErrors();

	void clear();

	bool contains( const Node* const node ) const;
	
	uint errorCount() const { return _errors.size(); }

	void getError( uint i, const char*& message, const Node*& node, int& slot );

private:

	struct Error {
		const std::string message;
		const Node* node;
		const int slot;

		Error(const std::string& _message, const Node* _node, int _slot) : message(_message), node(_node), slot(_slot){}
	};

	std::vector<Error> _errors;
	std::string _cachedSummary;
	bool _dirtySummary{false};
};

bool evaluate(const Graph& _editGraph, ErrorContext& _context, const std::vector<std::string>& inputPaths, const std::string& outputDir );

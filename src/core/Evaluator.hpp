#pragma once
#include "core/Common.hpp"
#include "core/Graph.hpp"
#include "core/system/System.hpp"


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

struct CompiledNode {
	const Node* node;
	std::vector<int> inputs;
	std::vector<int> outputs;
};

class CompiledGraph {
public:
	std::vector<CompiledNode> nodes;
	uint stackSize{0u};
	uint tmpImageCount{0u};

	void ensureGlobalNodesConsistency();

	void collectInputsAndOutputs(std::vector<const Node*>& inputs, std::vector<const Node*>& outputs);

	void clearInternalNodes();

	~CompiledGraph();
};

bool validate(const Graph& editGraph, ErrorContext& context );

bool evaluate(const Graph& editGraph, ErrorContext& context, const std::vector<std::string>& inputPaths, const std::string& outputDir, const glm::ivec2& outputRes);

bool evaluateStaggered(const Graph& editGraph, ErrorContext& errors, const std::vector<std::string>& inputPaths, const std::string& outputDir, const glm::ivec2& fallbackRes);

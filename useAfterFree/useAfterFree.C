// -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*-
// vim: expandtab:shiftwidth=2:tabstop=2

// Use After Free Analysis
// Author: Ubuntu-ROSE-Demo-V,,,
// Date: 02-November-2017

#include "rose.h"
#include "compass.h"

using namespace std;

extern const Compass::Checker* const useAfterFreeChecker;

namespace CompassAnalyses {
namespace UseAfterFree {
/*! \brief Use After Free: Add your description here
 */

// Specification of Checker Output Implementation
class CheckerOutput: public Compass::OutputViolationBase {
public:
	CheckerOutput(SgNode* node, const std::string & reason);
};

// Specification of Checker Traversal Implementation

class Traversal: public Compass::AstSimpleProcessingWithRunFunction {
	Compass::OutputObject* output;
	// Checker specific parameters should be allocated here.

public:
	Traversal(Compass::Parameters inputParameters,
			Compass::OutputObject* output);

	// Change the implementation of this function if you are using inherited attributes.
	void *initialInheritedAttribute() const {
		return NULL;
	}

	// The implementation of the run function has to match the traversal being called.
	// If you use inherited attributes, use the following definition:
	// void run(SgNode* n){ this->traverse(n, initialInheritedAttribute()); }
	void run(SgNode* n) {
		this->traverse(n, preorder);
	}

	// Change this function if you are using a different type of traversal, e.g.
	// void *evaluateInheritedAttribute(SgNode *, void *);
	// for AstTopDownProcessing.
	void visit(SgNode* n);
};
}
}

CompassAnalyses::UseAfterFree::CheckerOutput::CheckerOutput(SgNode* node,
		const std::string & reason) :
		OutputViolationBase(node, ::useAfterFreeChecker->checkerName,
				reason + ::useAfterFreeChecker->shortDescription) {
}

CompassAnalyses::UseAfterFree::Traversal::Traversal(
		Compass::Parameters inputParameters, Compass::OutputObject* output) :
		output(output) {
}

void CompassAnalyses::UseAfterFree::Traversal::visit(SgNode* node) {
	// Implement your traversal here.
	SgInitializedName* init = NULL;
	SgFunctionCallExp* func = isSgFunctionCallExp(node);
	if (func) {
		SgFunctionRefExp *fref = isSgFunctionRefExp(func->get_function());
		SgMemberFunctionRefExp *fmem = isSgMemberFunctionRefExp(
				func->get_function());
		std::string func_name_str("");
		if (fref)
			func_name_str = fref->get_symbol()->get_name().getString();
		if (fmem)
			func_name_str = fmem->get_symbol()->get_name().getString();
		if (func_name_str.compare("free") == 0) {
			SgExprListExp* list = func->get_args();
			Rose_STL_Container<SgExpression*> plist = list->get_expressions();

			SgExpression* argument = *plist.begin();
			while (isSgUnaryOp(argument))
				argument = isSgUnaryOp(argument)->get_operand();

			SgVariableSymbol* var = NULL;
			switch (argument->variantT()) {
			case V_SgVarRefExp: {
				var = isSgVarRefExp(argument)->get_symbol();
				break;
			}
			case V_SgArrowExp: {
				SgExpression* ex = isSgArrowExp(argument)->get_rhs_operand();
				if (isSgVarRefExp(ex))
					var = isSgVarRefExp(ex)->get_symbol();
				break;
			}
			case V_SgDotExp: {
				SgExpression* ex = isSgDotExp(argument)->get_rhs_operand();
				if (isSgVarRefExp(ex))
					var = isSgVarRefExp(ex)->get_symbol();
				break;
			}
			case V_SgPointerDerefExp: {
				SgExpression* ex = isSgPointerDerefExp(argument)->get_operand();
				if (isSgVarRefExp(ex))
					var = isSgVarRefExp(ex)->get_symbol();
				break;
			}
			default:
				return;
			} // switch

			if (var)
				init = var->get_declaration();
		}
	}

	if (!init)
		return;

	// traverse cfg within this function and find malloc with same init
	// if not found trigger error
	vector < FilteredCFGNode<IsDFAFilter> > worklist;
	vector < FilteredCFGNode<IsDFAFilter> > visited;
	// add this node to worklist and work through the outgoing edges
	FilteredCFGNode < IsDFAFilter > source = FilteredCFGNode < IsDFAFilter
			> (node->cfgForEnd());

	worklist.push_back(source);
	while (!worklist.empty()) {
		source = worklist.front();
		worklist.erase(worklist.begin());
		SgVarRefExp* next = isSgVarRefExp(source.getNode());
		if (next) {
			SgVariableSymbol* var = next->get_symbol();
			if (var) {
				SgInitializedName* initAssign = var->get_declaration();
				if (initAssign == init) {
					SgNode* parent = next->get_parent();
					while (!isSgFunctionDeclaration(parent)
							&& !isSgGlobal(parent))
						parent = parent->get_parent();
					std::string funcname = "";
					if (isSgFunctionDeclaration(parent))
						funcname = isSgFunctionDeclaration(parent)->get_name();
					std::string reason = "\tin function: " + funcname + "\t";
					output->addOutput(new CheckerOutput(next, reason));
				}
			}
		}

		vector < FilteredCFGEdge<IsDFAFilter> > out_edges = source.outEdges();
		for (vector<FilteredCFGEdge<IsDFAFilter> >::const_iterator i =
				out_edges.begin(); i != out_edges.end(); ++i) {
			FilteredCFGEdge<IsDFAFilter> filterEdge = *i;
			FilteredCFGNode<IsDFAFilter> filterNode = filterEdge.target();
			if (find(visited.begin(), visited.end(), filterNode)
					== visited.end()) {
				worklist.push_back(filterNode);
				visited.push_back(filterNode);
			}
		}
	}
} //End of the visit function.

// Checker main run function and metadata

static void run(Compass::Parameters params, Compass::OutputObject* output) {
	CompassAnalyses::UseAfterFree::Traversal(params, output).run(
			Compass::projectPrerequisite.getProject());
}

// Remove this function if your checker is not an AST traversal
static Compass::AstSimpleProcessingWithRunFunction* createTraversal(
		Compass::Parameters params, Compass::OutputObject* output) {
	return new CompassAnalyses::UseAfterFree::Traversal(params, output);
}

extern const Compass::Checker* const useAfterFreeChecker =
		new Compass::CheckerUsingAstSimpleProcessing("UseAfterFree",
				// Descriptions should not include the newline character "\n".
				"Should not use after free", "Should not use after free",
				Compass::C | Compass::Cpp,
				Compass::PrerequisiteList(1, &Compass::projectPrerequisite),
				run, createTraversal);


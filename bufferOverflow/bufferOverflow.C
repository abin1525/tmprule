// -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*-
// vim: expandtab:shiftwidth=2:tabstop=2

// Buffer Overflow Analysis
// Author: Ubuntu-ROSE-Demo-V,,,
// Date: 19-October-2017

#include "rose.h"
#include "compass.h"
#include <iostream>

extern const Compass::Checker* const bufferOverflowChecker;

using namespace std;

namespace CompassAnalyses {
namespace BufferOverflow {
/*! \brief Buffer Overflow: Add your description here
 */

// Specification of Checker Output Implementation
class CheckerOutput: public Compass::OutputViolationBase {
public:
	CheckerOutput(std::string problem, SgNode* node);
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

private:
	long long get_const(SgNode* value_node);

	void violation(SgNode* node);
};
}
}

CompassAnalyses::BufferOverflow::CheckerOutput::CheckerOutput(
		std::string problem, SgNode* node) :
		OutputViolationBase(node, ::bufferOverflowChecker->checkerName,
				::bufferOverflowChecker->shortDescription + " " + problem) {
}

CompassAnalyses::BufferOverflow::Traversal::Traversal(
		Compass::Parameters inputParameters, Compass::OutputObject* output) :
		output(output) {
	// Initalize checker specific parameters here, for example:
	// YourParameter = Compass::parseInteger(inputParameters["BufferOverflow.YourParameter"]);

}

long long CompassAnalyses::BufferOverflow::Traversal::get_const(
		SgNode* value_node) {
	SgCharVal* cv = isSgCharVal(value_node);
	SgDoubleVal* dv = isSgDoubleVal(value_node);
	SgFloatVal* fv = isSgFloatVal(value_node);
	SgIntVal* iv = isSgIntVal(value_node);
	SgLongDoubleVal* ldv = isSgLongDoubleVal(value_node);
	SgLongIntVal* liv = isSgLongIntVal(value_node);
	SgLongLongIntVal* lliv = isSgLongLongIntVal(value_node);
	SgShortVal* sv = isSgShortVal(value_node);
	SgUnsignedCharVal* ucv = isSgUnsignedCharVal(value_node);
	SgUnsignedIntVal* uiv = isSgUnsignedIntVal(value_node);
	SgUnsignedLongLongIntVal* ulliv = isSgUnsignedLongLongIntVal(value_node);
	SgUnsignedLongVal* ulv = isSgUnsignedLongVal(value_node);
	SgWcharVal* wv = isSgWcharVal(value_node);

	if (cv)
		return cv->get_value();
	if (dv)
		return dv->get_value();
	if (fv)
		return fv->get_value();
	if (iv)
		return iv->get_value();
	if (ldv)
		return ldv->get_value();
	if (liv)
		return liv->get_value();
	if (lliv)
		return lliv->get_value();
	if (sv)
		return sv->get_value();
	if (ucv)
		return ucv->get_value();
	if (uiv)
		return uiv->get_value();
	if (ulliv)
		return ulliv->get_value();
	if (ulv)
		return ulv->get_value();
	if (wv)
		return wv->get_value();
	return 0;
}

void CompassAnalyses::BufferOverflow::Traversal::violation(SgNode* node) {
	SgNode* parent = node->get_parent();
	while (!isSgFunctionDeclaration(parent) && !isSgGlobal(parent))
		parent = parent->get_parent();
	std::string funcname = "";
	if (isSgFunctionDeclaration(parent))
		funcname = isSgFunctionDeclaration(parent)->get_name();
	std::string reason = "\tin function: " + funcname + "\t";

	// if we reach this point, then we have not detected a free for a malloc
	output->addOutput(new CheckerOutput(reason, node));
}

void CompassAnalyses::BufferOverflow::Traversal::visit(SgNode* node) {

//	dumpNodeInfo(node);
//	return;

	SgAssignInitializer* ain = isSgAssignInitializer(node);
	SgInitializedName* init = NULL;
	SgExprListExp* express = NULL;
	if (ain) {
		SgExpression* expr = ain->get_operand();
		while (isSgUnaryOp(expr) != NULL)
			expr = isSgUnaryOp(expr)->get_operand();
		SgFunctionCallExp* rhop_func_call = isSgFunctionCallExp(expr);
		if (!rhop_func_call)
			return;
		SgFunctionRefExp* rhop_func = isSgFunctionRefExp(
				rhop_func_call->get_function());
		if (!rhop_func)
			return;
		if (rhop_func->get_symbol()->get_name().getString().compare("malloc")
				== 0
				|| rhop_func->get_symbol()->get_name().getString().compare(
						"calloc") == 0) {
			init = isSgInitializedName(ain->get_parent());
			std::cerr << "xxxxxxxxxxxxx init" << init->class_name()
					<< ", name=\"" << init->unparseToString() << "\""
					<< std::endl;
		}
		express = rhop_func_call->get_args();
		std::cerr << "xxxxxxxxxxxxx express" << express->class_name()
				<< ", name=\"" << express->unparseToString() << "\""
				<< std::endl;
	}

	if (!init || !express)
		return;

	long long arrayInit = 0;
	Rose_STL_Container<SgExpression*> exprList = express->get_expressions();
	SgExpression* expr = *(exprList.begin());
	while (isSgUnaryOp(expr) != NULL)
		expr = isSgUnaryOp(expr)->get_operand();
	arrayInit = get_const(expr);
	if (!arrayInit)
		return;
	// query down to find sizeof operation -- we need to know the type!
	Rose_STL_Container<SgNode*> sizeofOp = NodeQuery::querySubTree(expr,
			V_SgSizeOfOp);
	// stop if we have a node with unknown typesize
	unsigned long typeSize = 0;
	if (sizeofOp.size() == 1) {
		typeSize = get_const((*(sizeofOp.begin()))->get_parent());
	} else if (!sizeofOp.size()) {
		typeSize = 1;
	}
	if (!typeSize)
		return;
	arrayInit = arrayInit / typeSize;
	std::cerr << "xxxxxxxxxxxxxxxxxx arrayInit=" << arrayInit << "\n";

	vector < FilteredCFGNode<IsDFAFilter> > worklist;
	vector < FilteredCFGNode<IsDFAFilter> > visited;
	// add this node to worklist and work through the outgoing edges
	FilteredCFGNode < IsDFAFilter > source = FilteredCFGNode < IsDFAFilter
			> (node->cfgForEnd());
	worklist.push_back(source);
	while (!worklist.empty()) {
		source = worklist.front();
		worklist.erase(worklist.begin());
		SgNode* next = source.getNode();

		SgPntrArrRefExp* pntr = isSgPntrArrRefExp(next);
		if (pntr) {
			SgVarRefExp* leftVar = isSgVarRefExp(pntr->get_lhs_operand());
			SgInitializedName* initVar = NULL;
			if (leftVar) {
				SgVariableSymbol* var = leftVar->get_symbol();
				if (var)
					initVar = var->get_declaration();
			}
			if (initVar != init)
				continue;
			if (!pntr->get_rhs_operand())
				continue;

			long long value = get_const(pntr->get_rhs_operand());
			if (value >= arrayInit) {
				violation(next);
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
	CompassAnalyses::BufferOverflow::Traversal(params, output).run(
			Compass::projectPrerequisite.getProject());
}

// Remove this function if your checker is not an AST traversal
static Compass::AstSimpleProcessingWithRunFunction* createTraversal(
		Compass::Parameters params, Compass::OutputObject* output) {
	return new CompassAnalyses::BufferOverflow::Traversal(params, output);
}

extern const Compass::Checker* const bufferOverflowChecker =
		new Compass::CheckerUsingAstSimpleProcessing("BufferOverflow",
				// Descriptions should not include the newline character "\n".
				"BufferOverflow",
				"BufferOverflow", Compass::C | Compass::Cpp,
				Compass::PrerequisiteList(1, &Compass::projectPrerequisite),
				run, createTraversal);


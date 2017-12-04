// -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*-
// vim: expandtab:shiftwidth=2:tabstop=2

// Set Null After Free Analysis
// CheckerType:GJB8114
// CheckerID:GJB8114-R_1_3_6-1
// Author: Ubuntu-ROSE-Demo-V,,,
// Date: 01-December-2017

#include "rose.h"
#include "compass.h"

using namespace std;

extern const Compass::Checker* const setNullAfterFreeChecker;

#define IS_ZERO(x) (x && 0 == x->get_value())

//// DQ (1/17/2009): Added declaration to match external defined in file:
//// rose/projects/compass/extensions/prerequisites/ProjectPrerequisite.h
//// I can't tell that it is defined anywhere in compass except the extern
//// declaration in ProjectPrerequisite.h
//Compass::ProjectPrerequisite Compass::projectPrerequisite;

namespace CompassAnalyses {
namespace SetNullAfterFree {
/*! \brief Set Null After Free: Add your description here
 */

// Specification of Checker Output Implementation
class CheckerOutput: public Compass::OutputViolationBase {
public:
	CheckerOutput(SgNode* node);
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
	bool is_zero_value(SgNode* value_node);
};
}
}

CompassAnalyses::SetNullAfterFree::CheckerOutput::CheckerOutput(SgNode* node) :
		OutputViolationBase(node, ::setNullAfterFreeChecker->checkerName,
				::setNullAfterFreeChecker->shortDescription) {
}

CompassAnalyses::SetNullAfterFree::Traversal::Traversal(
		Compass::Parameters inputParameters, Compass::OutputObject* output) :
		output(output) {
}

bool CompassAnalyses::SetNullAfterFree::Traversal::is_zero_value(
		SgNode* value_node) {
	return (IS_ZERO(isSgBoolValExp(value_node))
			|| IS_ZERO(isSgCharVal(value_node))
			// Complex
			|| IS_ZERO(isSgDoubleVal(value_node))
			// Enum
			|| IS_ZERO(isSgFloatVal(value_node))
			|| IS_ZERO(isSgIntVal(value_node))
			|| IS_ZERO(isSgLongDoubleVal(value_node))
			|| IS_ZERO(isSgLongIntVal(value_node))
			|| IS_ZERO(isSgLongLongIntVal(value_node))
			|| IS_ZERO(isSgShortVal(value_node))
			// String
			|| IS_ZERO(isSgUnsignedCharVal(value_node))
			|| IS_ZERO(isSgUnsignedIntVal(value_node))
			|| IS_ZERO(isSgUnsignedLongLongIntVal(value_node))
			|| IS_ZERO(isSgUnsignedLongVal(value_node))
			// UpcMyThread
			// UpcThreads
			|| IS_ZERO(isSgWcharVal(value_node)));
}

void CompassAnalyses::SetNullAfterFree::Traversal::visit(SgNode* node) {
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
			while (isSgUnaryOp(argument) != NULL)
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
		SgNode* next = source.getNode();
		SgAssignOp* assign = isSgAssignOp(next);
		SgInitializedName* initAssign = NULL;
		if (assign) {
			SgExpression* expr = assign->get_rhs_operand();
			while (isSgUnaryOp(expr) != NULL)
				expr = isSgUnaryOp(expr)->get_operand();
//			SgIntVal* intval = isSgIntVal(expr);
//			int value = -1;
//			if (intval)
//				value = intval->get_value();
			//cerr <<"assign found : intval = " << intval << "   value = " << value << endl;
			if (/*value == 0*/is_zero_value(expr)) {
				SgVariableSymbol* var = NULL;
				switch (assign->get_lhs_operand()->variantT()) {
				case V_SgVarRefExp: {
					var =
							isSgVarRefExp(assign->get_lhs_operand())->get_symbol();
					break;
				}
				case V_SgArrowExp: {
					SgExpression* ex =
							isSgArrowExp(assign->get_lhs_operand())->get_rhs_operand();
					if (isSgVarRefExp(ex))
						var = isSgVarRefExp(ex)->get_symbol();
					break;
				}
				case V_SgDotExp: {
					SgExpression* ex =
							isSgDotExp(assign->get_lhs_operand())->get_rhs_operand();
					if (isSgVarRefExp(ex))
						var = isSgVarRefExp(ex)->get_symbol();
					break;
				}
				case V_SgPointerDerefExp: {
					SgExpression* ex = isSgPointerDerefExp(
							assign->get_lhs_operand())->get_operand();
					if (isSgVarRefExp(ex))
						var = isSgVarRefExp(ex)->get_symbol();
					break;
				}
				default:
					return;
				} // switch
				if (var)
					initAssign = var->get_declaration();
			} // value == 0
			if (initAssign == init) {
				return;
			}
		}

		vector < FilteredCFGEdge<IsDFAFilter> > out_edges = source.outEdges();
		for (vector<FilteredCFGEdge<IsDFAFilter> >::const_iterator i =
				out_edges.begin(); i != out_edges.end(); ++i) {
			FilteredCFGEdge < IsDFAFilter > filterEdge = *i;
			FilteredCFGNode < IsDFAFilter > filterNode = filterEdge.target();
			if (find(visited.begin(), visited.end(), filterNode)
					== visited.end()) {
				worklist.push_back(filterNode);
				visited.push_back(filterNode);
			}
		}
	}

	output->addOutput(new CheckerOutput(node));
} //End of the visit function.

// Checker main run function and metadata

static void run(Compass::Parameters params, Compass::OutputObject* output) {
	CompassAnalyses::SetNullAfterFree::Traversal(params, output).run(
			Compass::projectPrerequisite.getProject());
}

// Remove this function if your checker is not an AST traversal
static Compass::AstSimpleProcessingWithRunFunction* createTraversal(
		Compass::Parameters params, Compass::OutputObject* output) {
	return new CompassAnalyses::SetNullAfterFree::Traversal(params, output);
}

extern const Compass::Checker* const setNullAfterFreeChecker =
		new Compass::CheckerUsingAstSimpleProcessing("SetNullAfterFree",
				// Descriptions should not include the newline character "\n".
				"Should set pointer to nullptr after free()",
				"Should set pointer to nullptr after free()",
				Compass::C | Compass::Cpp,
				Compass::PrerequisiteList(1, &Compass::projectPrerequisite),
				run, createTraversal);


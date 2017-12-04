// -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*-
// vim: expandtab:shiftwidth=2:tabstop=2

// Const Condition Analysis
// Author: Ubuntu-ROSE-Demo-V,,,
// Date: 19-October-2017

#include "rose.h"
#include "compass.h"

extern const Compass::Checker* const constConditionChecker;

#define IS_ZERO(x) (x && 0 == x->get_value())

namespace CompassAnalyses {
namespace ConstCondition {
/*! \brief Const Condition: Add your description here
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

private:
	bool is_const(SgNode* value_node);

	bool is_zero_value(SgNode* value_node);
};
}
}

CompassAnalyses::ConstCondition::CheckerOutput::CheckerOutput(SgNode* node,
		const std::string & reason) :
		OutputViolationBase(node, ::constConditionChecker->checkerName,
				reason + ::constConditionChecker->shortDescription) {
}

CompassAnalyses::ConstCondition::Traversal::Traversal(
		Compass::Parameters inputParameters, Compass::OutputObject* output) :
		output(output) {
	// Initalize checker specific parameters here, for example:
	// YourParameter = Compass::parseInteger(inputParameters["ConstCondition.YourParameter"]);

}

void CompassAnalyses::ConstCondition::Traversal::visit(SgNode* node) {
//	dumpNodeInfo(node);

	// Implement your traversal here.
	if (SgIfStmt* ifStmt = isSgIfStmt(node)) {
		SgExprStatement* exprStatement = isSgExprStatement(
				ifStmt->get_conditional());
		if (!exprStatement)
			return;
		if (exprStatement->get_numberOfTraversalSuccessors() != 1)
			return;
		SgCastExp* castExp = isSgCastExp(exprStatement->get_expression());
		if (!castExp
				|| is_const(castExp->get_traversalSuccessorByIndex(0))) {
			std::ostringstream reason;
			reason << "\tThe if statement's condition contains constant.\t";
			output->addOutput(new CheckerOutput(node, reason.str()));
		}
	}
} //End of the visit function.

bool CompassAnalyses::ConstCondition::Traversal::is_const(SgNode* value_node) {
	return (isSgBoolValExp(value_node) || isSgCharVal(value_node)
			// Complex
			|| isSgDoubleVal(value_node)
			// Enum
			|| isSgFloatVal(value_node) || isSgIntVal(value_node)
			|| isSgLongDoubleVal(value_node) || isSgLongIntVal(value_node)
			|| isSgLongLongIntVal(value_node) || isSgShortVal(value_node)
			// String
			|| isSgUnsignedCharVal(value_node) || isSgUnsignedIntVal(value_node)
			|| isSgUnsignedLongLongIntVal(value_node)
			|| isSgUnsignedLongVal(value_node)
			// UpcMyThread
			// UpcThreads
			|| isSgWcharVal(value_node));
}

bool CompassAnalyses::ConstCondition::Traversal::is_zero_value(
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

// Checker main run function and metadata

static void run(Compass::Parameters params, Compass::OutputObject* output) {
	CompassAnalyses::ConstCondition::Traversal(params, output).run(
			Compass::projectPrerequisite.getProject());
}

// Remove this function if your checker is not an AST traversal
static Compass::AstSimpleProcessingWithRunFunction* createTraversal(
		Compass::Parameters params, Compass::OutputObject* output) {
	return new CompassAnalyses::ConstCondition::Traversal(params, output);
}

extern const Compass::Checker* const constConditionChecker =
		new Compass::CheckerUsingAstSimpleProcessing("ConstCondition",
				// Descriptions should not include the newline character "\n".
				"The condition statement should not contain constant.",
				"The condition statement should not contain constant.",
				Compass::C | Compass::Cpp,
				Compass::PrerequisiteList(1, &Compass::projectPrerequisite),
				run, createTraversal);


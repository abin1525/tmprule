// Divide By Zero Analysis
// Rule: GJB8114-R_1_6_12-1
// Author: Ubuntu-ROSE-Demo-V,,,
// Date: 18-October-2017

#include "rose.h"
#include "compass.h"

extern const Compass::Checker* const divideByZeroChecker;

#define IS_ZERO(x) (x && 0 == x->get_value())

namespace CompassAnalyses {
namespace DivideByZero {
/*! \brief Divide By Zero: Add your description here
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
	bool is_zero_value(SgNode* value_node);
};
}
}

CompassAnalyses::DivideByZero::CheckerOutput::CheckerOutput(SgNode* node,
		const std::string & reason) :
		OutputViolationBase(node, ::divideByZeroChecker->checkerName,
				reason + ::divideByZeroChecker->shortDescription) {
}

CompassAnalyses::DivideByZero::Traversal::Traversal(
		Compass::Parameters inputParameters, Compass::OutputObject* output) :
		output(output) {
	// Initalize checker specific parameters here, for example:
	// YourParameter = Compass::parseInteger(inputParameters["DivideByZero.YourParameter"]);
}

void CompassAnalyses::DivideByZero::Traversal::visit(SgNode* node) {
	// Implement your traversal here.
	if (isSgDivideOp(node) || isSgModOp(node)) {
		size_t childs = node->get_numberOfTraversalSuccessors();
		if (childs != 2)
			return; // Unexpected expression
		SgNode* Divisor = node->get_traversalSuccessorByIndex(1);
		if (Divisor) {
			if (is_zero_value(Divisor)) {
				std::ostringstream reason;
				reason << "\tDivisor is zero\t";
				output->addOutput(new CheckerOutput(node, reason.str()));
			}
		}
	}
} //End of the visit function.

bool CompassAnalyses::DivideByZero::Traversal::is_zero_value(
		SgNode* value_node) {
	return (
	IS_ZERO(isSgBoolValExp(value_node)) || IS_ZERO(isSgCharVal(value_node))
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
	CompassAnalyses::DivideByZero::Traversal(params, output).run(
			Compass::projectPrerequisite.getProject());
}

// Remove this function if your checker is not an AST traversal
static Compass::AstSimpleProcessingWithRunFunction* createTraversal(
		Compass::Parameters params, Compass::OutputObject* output) {
	return new CompassAnalyses::DivideByZero::Traversal(params, output);
}

extern const Compass::Checker* const divideByZeroChecker =
		new Compass::CheckerUsingAstSimpleProcessing("DivideByZero",
				// Descriptions should not include the newline character "\n".
				"Divisor should not be zero!", "Divisor should not be zero!",
				Compass::C | Compass::Cpp,
				Compass::PrerequisiteList(1, &Compass::projectPrerequisite),
				run, createTraversal);


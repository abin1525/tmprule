// -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*-
// vim: expandtab:shiftwidth=2:tabstop=2

// Overload Operators Analysis
// CheckerType:GJB8114
// CheckerID:GJB8114-CPP-A_2_9_3-3
// Author: Ubuntu-ROSE-Demo-V,,,
// Date: 20-December-2017

#include "rose.h"
#include "compass.h"
#include <string>

extern const Compass::Checker* const overloadOperatorsChecker;

//// DQ (1/17/2009): Added declaration to match external defined in file:
//// rose/projects/compass/extensions/prerequisites/ProjectPrerequisite.h
//// I can't tell that it is defined anywhere in compass except the extern
//// declaration in ProjectPrerequisite.h
//Compass::ProjectPrerequisite Compass::projectPrerequisite;

namespace CompassAnalyses {
namespace OverloadOperators {
/*! \brief Overload Operators: Add your description here
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
};
}
}

CompassAnalyses::OverloadOperators::CheckerOutput::CheckerOutput(SgNode* node) :
		OutputViolationBase(node, ::overloadOperatorsChecker->checkerName,
				::overloadOperatorsChecker->shortDescription) {
}

CompassAnalyses::OverloadOperators::Traversal::Traversal(
		Compass::Parameters inputParameters, Compass::OutputObject* output) :
		output(output) {
}

void CompassAnalyses::OverloadOperators::Traversal::visit(SgNode* node) {
	std::string fName;
	SgMemberFunctionDeclaration *mfn = isSgMemberFunctionDeclaration(node);
	SgFunctionDeclaration* fn = isSgFunctionDeclaration(node);
	if (mfn)
		fName = mfn->get_name().str();
	else if (fn)
		fName = fn->get_name().str();
	if (fName.empty())
		return;
	std::string op1 = "operator&&";
	std::string op2 = "operator||";
	std::string op3 = "operator,";
	if (op1 == fName || op2 == fName || op3 == fName) {
		output->addOutput(new CheckerOutput(node));
	}
} //End of the visit function.

// Checker main run function and metadata

static void run(Compass::Parameters params, Compass::OutputObject* output) {
	CompassAnalyses::OverloadOperators::Traversal(params, output).run(
			Compass::projectPrerequisite.getProject());
}

// Remove this function if your checker is not an AST traversal
static Compass::AstSimpleProcessingWithRunFunction* createTraversal(
		Compass::Parameters params, Compass::OutputObject* output) {
	return new CompassAnalyses::OverloadOperators::Traversal(params, output);
}

extern const Compass::Checker* const overloadOperatorsChecker =
		new Compass::CheckerUsingAstSimpleProcessing("OverloadOperators",
				// Descriptions should not include the newline character "\n".
				"Should not overload '&&', '||', ',' operators",
				"Should not overload '&&', '||', ',' operators",
				Compass::C | Compass::Cpp,
				Compass::PrerequisiteList(1, &Compass::projectPrerequisite),
				run, createTraversal);


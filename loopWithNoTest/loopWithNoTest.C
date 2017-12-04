// -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*-
// vim: expandtab:shiftwidth=2:tabstop=2

// Loop With No Test Analysis
// CheckerType:GJB8114
// CheckerID:GJB8114-A_1_5_1-3
// Author: Ubuntu-ROSE-Demo-V,,,
// Date: 28-November-2017

#include "rose.h"
#include "compass.h"

extern const Compass::Checker* const loopWithNoTestChecker;

//// DQ (1/17/2009): Added declaration to match external defined in file:
//// rose/projects/compass/extensions/prerequisites/ProjectPrerequisite.h
//// I can't tell that it is defined anywhere in compass except the extern
//// declaration in ProjectPrerequisite.h
//Compass::ProjectPrerequisite Compass::projectPrerequisite;

namespace CompassAnalyses {
namespace LoopWithNoTest {
/*! \brief Loop With No Test: Add your description here
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

CompassAnalyses::LoopWithNoTest::CheckerOutput::CheckerOutput(SgNode* node) :
		OutputViolationBase(node, ::loopWithNoTestChecker->checkerName,
				::loopWithNoTestChecker->shortDescription) {
}

CompassAnalyses::LoopWithNoTest::Traversal::Traversal(
		Compass::Parameters inputParameters, Compass::OutputObject* output) :
		output(output) {
	// Initalize checker specific parameters here, for example:
	// YourParameter = Compass::parseInteger(inputParameters["LoopWithNoTest.YourParameter"]);
}

void CompassAnalyses::LoopWithNoTest::Traversal::visit(SgNode* node) {
	SgForStatement* ForStatement = isSgForStatement(node);
	if (ForStatement) {
		SgStatement* test = ForStatement->get_test();
		if (test && isSgNullStatement(test)) {
			output->addOutput(new CheckerOutput(node));
			return;
		}
	}
} //End of the visit function.

// Checker main run function and metadata

static void run(Compass::Parameters params, Compass::OutputObject* output) {
	CompassAnalyses::LoopWithNoTest::Traversal(params, output).run(
			Compass::projectPrerequisite.getProject());
}

// Remove this function if your checker is not an AST traversal
static Compass::AstSimpleProcessingWithRunFunction* createTraversal(
		Compass::Parameters params, Compass::OutputObject* output) {
	return new CompassAnalyses::LoopWithNoTest::Traversal(params, output);
}

extern const Compass::Checker* const loopWithNoTestChecker =
		new Compass::CheckerUsingAstSimpleProcessing("LoopWithNoTest",
				// Descriptions should not include the newline character "\n".
				"loop with no test", "loop with no test",
				Compass::C | Compass::Cpp,
				Compass::PrerequisiteList(1, &Compass::projectPrerequisite),
				run, createTraversal);


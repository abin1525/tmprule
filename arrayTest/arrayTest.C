// -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*-
// vim: expandtab:shiftwidth=2:tabstop=2

// Array Test Analysis
// Author: Ubuntu-ROSE-Demo-V,,,
// Date: 28-September-2017

#include "rose.h"
#include "compass.h"

extern const Compass::Checker* const arrayTestChecker;

// DQ (1/17/2009): Added declaration to match external defined in file:
// rose/projects/compass/extensions/prerequisites/ProjectPrerequisite.h
// I can't tell that it is defined anywhere in compass except the extern 
// declaration in ProjectPrerequisite.h
Compass::ProjectPrerequisite Compass::projectPrerequisite;

namespace CompassAnalyses {
namespace ArrayTest {
/*! \brief Array Test: Add your description here
 */

// Specification of Checker Output Implementation
class CheckerOutput: public Compass::OutputViolationBase {
public:
	CheckerOutput(SgNode* node, const std::string &);
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

CompassAnalyses::ArrayTest::CheckerOutput::CheckerOutput(SgNode* node,
		const std::string & reason) :
		OutputViolationBase(node, ::arrayTestChecker->checkerName,
				reason + ::arrayTestChecker->shortDescription) {
}

CompassAnalyses::ArrayTest::Traversal::Traversal(
		Compass::Parameters inputParameters, Compass::OutputObject* output) :
		output(output) {
	// Initalize checker specific parameters here, for example:
	// YourParameter = Compass::parseInteger(inputParameters["ArrayTest.YourParameter"]);

}

void CompassAnalyses::ArrayTest::Traversal::visit(SgNode* node) {
	if (isSgPntrArrRefExp(node)) {
		SgInitializedName* init = NULL;
		int ArraySize = 0;
		size_t childs = node->get_numberOfTraversalSuccessors();
		if (childs != 2)
			return; // Unexpected array expression
		if (SgVarRefExp* Arr = isSgVarRefExp(
				node->get_traversalSuccessorByIndex(0))) {
			if (Arr->get_symbol())
				init = Arr->get_symbol()->get_declaration();
			if (init) {
				SgType* t = init->get_type();
				std::string typeName = t->unparseToString();
				std::cerr << "XXXXXXXXXXXtypeName=" << typeName << std::endl;
				int tokStart = typeName.find("[");
				int tokEnd = typeName.find("]");
				std::string ArraySizeStr = typeName.substr(tokStart + 1,
						tokEnd - tokStart);
				atoi(ArraySizeStr.c_str());
				std::cerr << "xxxxxxxxxxxxx ArraySize=" << ArraySize
						<< std::endl;

				if (SgIntVal* Idx = isSgIntVal(
						node->get_traversalSuccessorByIndex(1))) {
					int IdxValue = Idx->get_value();
					if (IdxValue/* in case of: char* p = xx; p[0];*/
							&& (IdxValue >= ArraySize || IdxValue < ArraySize)) {
						std::ostringstream reason;
						reason << "\tIndex value \"" << IdxValue
								<< "\" of Array \"" << typeName
								<< "\" out of bounds\t";
						output->addOutput(
								new CheckerOutput(node, reason.str()));
					}
				}
			}
		}

	}
} //End of the visit function.

// Checker main run function and metadata

static void run(Compass::Parameters params, Compass::OutputObject* output) {
	CompassAnalyses::ArrayTest::Traversal(params, output).run(
			Compass::projectPrerequisite.getProject());
}

// Remove this function if your checker is not an AST traversal
static Compass::AstSimpleProcessingWithRunFunction* createTraversal(
		Compass::Parameters params, Compass::OutputObject* output) {
	return new CompassAnalyses::ArrayTest::Traversal(params, output);
}

extern const Compass::Checker* const arrayTestChecker =
		new Compass::CheckerUsingAstSimpleProcessing("ArrayTest",
				// Descriptions should not include the newline character "\n".
				"Accessing array should in bounds",
				"Accessing array should in bounds",
				Compass::C | Compass::Cpp,
				Compass::PrerequisiteList(1, &Compass::projectPrerequisite),
				run, createTraversal);


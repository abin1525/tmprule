// Pointer Depth Analysis
// Rule: GJB8114-R_1_3_1-1
// Author: Ubuntu-ROSE-Demo-V,,,
// Date: 26-November-2017

#include "rose.h"
#include "compass.h"

extern const Compass::Checker* const pointerDepthChecker;

// DQ (1/17/2009): Added declaration to match external defined in file:
// rose/projects/compass/extensions/prerequisites/ProjectPrerequisite.h
// I can't tell that it is defined anywhere in compass except the extern
// declaration in ProjectPrerequisite.h
//Compass::ProjectPrerequisite Compass::projectPrerequisite;

namespace CompassAnalyses {
namespace PointerDepth {
/*! \brief Pointer Depth: Add your description here
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

CompassAnalyses::PointerDepth::CheckerOutput::CheckerOutput(SgNode* node) :
		OutputViolationBase(node, ::pointerDepthChecker->checkerName,
				::pointerDepthChecker->shortDescription) {
}

CompassAnalyses::PointerDepth::Traversal::Traversal(
		Compass::Parameters inputParameters, Compass::OutputObject* output) :
		output(output) {
	// Initalize checker specific parameters here, for example:
	// YourParameter = Compass::parseInteger(inputParameters["PointerDepth.YourParameter"]);
}

static int countStars(std::string varName) {
	bool PreIsStar = false;
	int total = 0;
	for (size_t i = 0; i < varName.length(); i++) {
		if (varName[i] == '*') {
			if (PreIsStar) {
				total++;
			} else if (total == 0) {
				PreIsStar = true;
				total++;
			} else
				break;
		}
	}
	return total;
}

static bool isConsideredNode(SgNode* node) {
	return isSgFunctionParameterList(node) || isSgVariableDeclaration(node)
			|| isSgTypedefDeclaration(node);
}

void CompassAnalyses::PointerDepth::Traversal::visit(SgNode* node) {
	Rose_STL_Container<SgNode*> pointers = NodeQuery::querySubTree(node,
			V_SgPointerType);

	for (Rose_STL_Container<SgNode *>::iterator i = pointers.begin();
			i != pointers.end(); ++i) {
		std::string typeName = (*i)->unparseToString();
		if (countStars(typeName) > 2) {
			if (isConsideredNode(node)) {
				output->addOutput(new CheckerOutput(node));
				return;
			}
		}
	}

	if (isSgFunctionDeclaration(node)) {
		if (countStars(node->unparseToString()) > 2) {
			output->addOutput(new CheckerOutput(node));
			return;
		}
	}
} //End of the visit function.

// Checker main run function and metadata

static void run(Compass::Parameters params, Compass::OutputObject* output) {
	CompassAnalyses::PointerDepth::Traversal(params, output).run(
			Compass::projectPrerequisite.getProject());
}

// Remove this function if your checker is not an AST traversal
static Compass::AstSimpleProcessingWithRunFunction* createTraversal(
		Compass::Parameters params, Compass::OutputObject* output) {
	return new CompassAnalyses::PointerDepth::Traversal(params, output);
}

extern const Compass::Checker* const pointerDepthChecker =
		new Compass::CheckerUsingAstSimpleProcessing("PointerDepth",
				// Descriptions should not include the newline character "\n".
				"Pointer depth exceed 2", "Pointer depth exceed 2",
				Compass::C | Compass::Cpp,
				Compass::PrerequisiteList(1, &Compass::projectPrerequisite),
				run, createTraversal);


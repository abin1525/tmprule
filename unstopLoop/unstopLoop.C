// -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*-
// vim: expandtab:shiftwidth=2:tabstop=2

// Unstop Loop Analysis
// CheckerType:GJB8114
// CheckerID:GJB8114-A_1_5_1-3
// Author: Ubuntu-ROSE-Demo-V,,,
// Date: 28-November-2017

#include "rose.h"
#include "compass.h"

extern const Compass::Checker* const unstopLoopChecker;

//// DQ (1/17/2009): Added declaration to match external defined in file:
//// rose/projects/compass/extensions/prerequisites/ProjectPrerequisite.h
//// I can't tell that it is defined anywhere in compass except the extern
//// declaration in ProjectPrerequisite.h
//Compass::ProjectPrerequisite Compass::projectPrerequisite;

namespace CompassAnalyses {
namespace UnstopLoop {
/*! \brief Unstop Loop: Add your description here
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

CompassAnalyses::UnstopLoop::CheckerOutput::CheckerOutput(SgNode* node) :
		OutputViolationBase(node, ::unstopLoopChecker->checkerName,
				::unstopLoopChecker->shortDescription) {
}

CompassAnalyses::UnstopLoop::Traversal::Traversal(
		Compass::Parameters inputParameters, Compass::OutputObject* output) :
		output(output) {
	// Initalize checker specific parameters here, for example:
	// YourParameter = Compass::parseInteger(inputParameters["UnstopLoop.YourParameter"]);
}

static void dumpNodeInfo(SgNode* TheNode) {
	static std::set<SgNode*> DoneSet;

	std::string ClassName = TheNode->class_name();
	if (ClassName.compare("SgProject") == 0
			|| ClassName.compare("SgFileList") == 0
			|| ClassName.compare("SgSourceFile") == 0
			|| ClassName.compare("SgGlobal") == 0)
		return;

	if (!DoneSet.count(TheNode)) {
		std::cerr << "\nxxxxxxxxxxxxxxxx vvvv Info about node with class_name="
				<< TheNode->class_name() << ", name=\""
				<< TheNode->unparseToString() << "\"" << std::endl;
		// Deal with children
		size_t ChildCnt = TheNode->get_numberOfTraversalSuccessors();
		for (size_t idx = 0; idx < ChildCnt; ++idx) {
			SgNode* Child = TheNode->get_traversalSuccessorByIndex(idx);
			if (Child) {
				std::cerr << "Child_" << idx << ":" << std::endl;
				std::cerr << "\tclass_name=\"" << Child->class_name() << "\""
						<< std::endl;
				std::cerr << "\tname=\"" << Child->unparseToString() << "\""
						<< std::endl;
			}
		}

		// Record it
		DoneSet.insert(TheNode);

		// Deal with parent
		SgNode* Parent = TheNode->get_parent();
		if (Parent && !isSgGlobal(Parent)) {
			std::cerr << "The parent is: class_name=\"" << Parent->class_name()
					<< "\"" << ", name=\"" << Parent->unparseToString() << "\""
					<< std::endl;
		}

		std::cerr << "xxxxxxxxxxxxxxxx ^^^^ Info about node with class_name=\""
				<< TheNode->class_name() << "\", name=\""
				<< TheNode->unparseToString() << "\"" << std::endl;
		dumpNodeInfo(Parent);
	}
}

void CompassAnalyses::UnstopLoop::Traversal::visit(SgNode* node) {
	// Implement your traversal here.
	dumpNodeInfo(node);
	if (SgForStatement* ForStatement = isSgForStatement(node)) {
		SgExpression* test = ForStatement->get_test_expr();
		if (isSgNullExpression(test)) {
			output->addOutput(new CheckerOutput(node));
			return;
		}
	}

} //End of the visit function.

// Checker main run function and metadata

static void run(Compass::Parameters params, Compass::OutputObject* output) {
	CompassAnalyses::UnstopLoop::Traversal(params, output).run(
			Compass::projectPrerequisite.getProject());
}

// Remove this function if your checker is not an AST traversal
static Compass::AstSimpleProcessingWithRunFunction* createTraversal(
		Compass::Parameters params, Compass::OutputObject* output) {
	return new CompassAnalyses::UnstopLoop::Traversal(params, output);
}

extern const Compass::Checker* const unstopLoopChecker =
		new Compass::CheckerUsingAstSimpleProcessing("UnstopLoop",
				// Descriptions should not include the newline character "\n".
				"Unstop loop", "Unstop loop", Compass::C | Compass::Cpp,
				Compass::PrerequisiteList(1, &Compass::projectPrerequisite),
				run, createTraversal);


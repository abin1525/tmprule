// -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*-
// vim: expandtab:shiftwidth=2:tabstop=2

// C Prefix Char Var Analysis
// CheckerType:GJB8114
// CheckerID:GJB8114-A_1_13_1_a-3
// Author: Ubuntu-ROSE-Demo-V,,,
// Date: 29-November-2017

#include "rose.h"
#include "compass.h"

extern const Compass::Checker* const cPrefixCharVarChecker;

//// DQ (1/17/2009): Added declaration to match external defined in file:
//// rose/projects/compass/extensions/prerequisites/ProjectPrerequisite.h
//// I can't tell that it is defined anywhere in compass except the extern
//// declaration in ProjectPrerequisite.h
//Compass::ProjectPrerequisite Compass::projectPrerequisite;

namespace CompassAnalyses {
namespace CPrefixCharVar {
/*! \brief C Prefix Char Var: Add your description here
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

CompassAnalyses::CPrefixCharVar::CheckerOutput::CheckerOutput(SgNode* node) :
		OutputViolationBase(node, ::cPrefixCharVarChecker->checkerName,
				::cPrefixCharVarChecker->shortDescription) {
}

CompassAnalyses::CPrefixCharVar::Traversal::Traversal(
		Compass::Parameters inputParameters, Compass::OutputObject* output) :
		output(output) {
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

void CompassAnalyses::CPrefixCharVar::Traversal::visit(SgNode* node) {
	dumpNodeInfo(node);

} //End of the visit function.

// Checker main run function and metadata

static void run(Compass::Parameters params, Compass::OutputObject* output) {
	CompassAnalyses::CPrefixCharVar::Traversal(params, output).run(
			Compass::projectPrerequisite.getProject());
}

// Remove this function if your checker is not an AST traversal
static Compass::AstSimpleProcessingWithRunFunction* createTraversal(
		Compass::Parameters params, Compass::OutputObject* output) {
	return new CompassAnalyses::CPrefixCharVar::Traversal(params, output);
}

extern const Compass::Checker* const cPrefixCharVarChecker =
		new Compass::CheckerUsingAstSimpleProcessing("CPrefixCharVar",
				// Descriptions should not include the newline character "\n".
				"char variable should named with c prefix",
				"char variable should named with c prefix",
				Compass::C | Compass::Cpp,
				Compass::PrerequisiteList(1, &Compass::projectPrerequisite),
				run, createTraversal);


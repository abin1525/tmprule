// -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*-
// vim: expandtab:shiftwidth=2:tabstop=2

// Init To Null Analysis
// CheckerType:GJB8114
// CheckerID:GJB8114-R_1_3_7-1
// Author: Ubuntu-ROSE-Demo-V,,,
// Date: 01-December-2017

#include "rose.h"
#include "compass.h"

extern const Compass::Checker* const initToNullChecker;

//// DQ (1/17/2009): Added declaration to match external defined in file:
//// rose/projects/compass/extensions/prerequisites/ProjectPrerequisite.h
//// I can't tell that it is defined anywhere in compass except the extern
//// declaration in ProjectPrerequisite.h
//Compass::ProjectPrerequisite Compass::projectPrerequisite;

namespace CompassAnalyses {
namespace InitToNull {
/*! \brief Init To Null: Add your description here
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

CompassAnalyses::InitToNull::CheckerOutput::CheckerOutput(SgNode* node) :
		OutputViolationBase(node, ::initToNullChecker->checkerName,
				::initToNullChecker->shortDescription) {
}

CompassAnalyses::InitToNull::Traversal::Traversal(
		Compass::Parameters inputParameters, Compass::OutputObject* output) :
		output(output) {
}

void CompassAnalyses::InitToNull::Traversal::visit(SgNode* node) {
	if (SgVariableDeclaration* vardecl = isSgVariableDeclaration(node)) {
		SgInitializedNamePtrList& vars = vardecl->get_variables();
		SgInitializedNamePtrList::iterator var;
		for (var = vars.begin(); var != vars.end(); ++var) {
			SgInitializedName* initName = *var;
			SgType* VarType = initName->get_type();
			if (!VarType)
				continue;
			int VarTypeVT = VarType->variantT();
			if (VarTypeVT == V_SgPointerMemberType
					|| VarTypeVT == V_SgPointerType) {
				if (initName->get_initializer() == NULL
						&& !vardecl->get_declarationModifier().get_storageModifier().isExtern()) {
					output->addOutput(new CheckerOutput(node));
				}
			}
		}
	}
} //End of the visit function.

// Checker main run function and metadata

static void run(Compass::Parameters params, Compass::OutputObject* output) {
	CompassAnalyses::InitToNull::Traversal(params, output).run(
			Compass::projectPrerequisite.getProject());
}

// Remove this function if your checker is not an AST traversal
static Compass::AstSimpleProcessingWithRunFunction* createTraversal(
		Compass::Parameters params, Compass::OutputObject* output) {
	return new CompassAnalyses::InitToNull::Traversal(params, output);
}

extern const Compass::Checker* const initToNullChecker =
		new Compass::CheckerUsingAstSimpleProcessing("InitToNull",
				// Descriptions should not include the newline character "\n".
				"Pointer should init to null", "Pointer should init to null",
				Compass::C | Compass::Cpp,
				Compass::PrerequisiteList(1, &Compass::projectPrerequisite),
				run, createTraversal);


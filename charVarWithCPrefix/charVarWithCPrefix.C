// -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*-
// vim: expandtab:shiftwidth=2:tabstop=2

// Char Var With C Prefix Analysis
// CheckerType:GJB8114
// CheckerID:GJB8114-A_1_13_1_a-3
// Author: Ubuntu-ROSE-Demo-V,,,
// Date: 30-November-2017

#include "rose.h"
#include "compass.h"

extern const Compass::Checker* const charVarWithCPrefixChecker;

//// DQ (1/17/2009): Added declaration to match external defined in file:
//// rose/projects/compass/extensions/prerequisites/ProjectPrerequisite.h
//// I can't tell that it is defined anywhere in compass except the extern 
//// declaration in ProjectPrerequisite.h
//Compass::ProjectPrerequisite Compass::projectPrerequisite;

namespace CompassAnalyses {
namespace CharVarWithCPrefix {
/*! \brief Char Var With C Prefix: Add your description here 
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

CompassAnalyses::CharVarWithCPrefix::CheckerOutput::CheckerOutput(SgNode* node) :
		OutputViolationBase(node, ::charVarWithCPrefixChecker->checkerName,
				::charVarWithCPrefixChecker->shortDescription) {
}

CompassAnalyses::CharVarWithCPrefix::Traversal::Traversal(
		Compass::Parameters inputParameters, Compass::OutputObject* output) :
		output(output) {
}

void CompassAnalyses::CharVarWithCPrefix::Traversal::visit(SgNode* node) {
	if (SgVariableDeclaration* vardecl = isSgVariableDeclaration(node)) {
		SgInitializedNamePtrList& vars = vardecl->get_variables();
		SgInitializedNamePtrList::iterator var;
		for (var = vars.begin(); var != vars.end(); ++var) {
			SgInitializedName* initName = *var;
			if (!initName)
				continue;
			std::string VarName = initName->get_name().getString();
			SgType* VarType = initName->get_type();
			if (!VarType)
				continue;
			int VarTypeVT = VarType->variantT();
			if (VarTypeVT == V_SgTypeChar || VarTypeVT == V_SgTypeSignedChar
					|| VarTypeVT == V_SgTypeUnsignedChar
					|| VarTypeVT == V_SgTypeWchar) {
				if (VarName[0] != 'c' && VarName[0] != 'C') {
					output->addOutput(new CheckerOutput(node));
					return;
				}
			}
		}
	}

} //End of the visit function.

// Checker main run function and metadata

static void run(Compass::Parameters params, Compass::OutputObject* output) {
	CompassAnalyses::CharVarWithCPrefix::Traversal(params, output).run(
			Compass::projectPrerequisite.getProject());
}

// Remove this function if your checker is not an AST traversal
static Compass::AstSimpleProcessingWithRunFunction* createTraversal(
		Compass::Parameters params, Compass::OutputObject* output) {
	return new CompassAnalyses::CharVarWithCPrefix::Traversal(params, output);
}

extern const Compass::Checker* const charVarWithCPrefixChecker =
		new Compass::CheckerUsingAstSimpleProcessing("CharVarWithCPrefix",
				// Descriptions should not include the newline character "\n".
				"char var should named with c prefix",
				"char var should named with c prefix",
				Compass::C | Compass::Cpp,
				Compass::PrerequisiteList(1, &Compass::projectPrerequisite),
				run, createTraversal);


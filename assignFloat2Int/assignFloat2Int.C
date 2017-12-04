// -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*-
// vim: expandtab:shiftwidth=2:tabstop=2

// Assign Float 2 Int Analysis
// CheckerType:GJB8114
// CheckerID:GJB8114-R_1_10_1-1
// Author: Ubuntu-ROSE-Demo-V,,,
// Date: 27-November-2017

#include "rose.h"
#include "compass.h"

extern const Compass::Checker* const assignFloat2IntChecker;

//// DQ (1/17/2009): Added declaration to match external defined in file:
//// rose/projects/compass/extensions/prerequisites/ProjectPrerequisite.h
//// I can't tell that it is defined anywhere in compass except the extern
//// declaration in ProjectPrerequisite.h
//Compass::ProjectPrerequisite Compass::projectPrerequisite;

namespace CompassAnalyses {
namespace AssignFloat2Int {
/*! \brief Assign Float 2 Int: Add your description here
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

CompassAnalyses::AssignFloat2Int::CheckerOutput::CheckerOutput(SgNode* node) :
		OutputViolationBase(node, ::assignFloat2IntChecker->checkerName,
				::assignFloat2IntChecker->shortDescription) {
}

CompassAnalyses::AssignFloat2Int::Traversal::Traversal(
		Compass::Parameters inputParameters, Compass::OutputObject* output) :
		output(output) {
	// Initalize checker specific parameters here, for example:
	// YourParameter = Compass::parseInteger(inputParameters["AssignFloat2Int.YourParameter"]);

}

static bool isIntType(SgType *type) {
	int VT = type->variantT();
	return VT == V_SgTypeInt || VT == V_SgTypeSignedInt
			|| VT == V_SgTypeUnsignedInt;
}

static bool isFloatType(SgType *type) {
	int VT = type->variantT();
	return VT == V_SgTypeComplex || VT == V_SgTypeDouble || VT == V_SgTypeFloat
			|| VT == V_SgTypeLongDouble;
}

static int countCast(std::string varName) {
	int total = 0;
	for (size_t i = 0; i < varName.length(); i++) {
		if (varName[i] == '(') {
			total++;
		}
	}
	return total;
}

static SgInitializedName* getSgInitializedName(SgExpression* expr) {
	SgVariableSymbol* var = NULL;
	SgInitializedName* init = NULL;

	while (isSgUnaryOp(expr) != NULL)
		expr = isSgUnaryOp(expr)->get_operand();
	switch (expr->variantT()) {
	case V_SgVarRefExp: {
		var = isSgVarRefExp(expr)->get_symbol();
		break;
	}
	case V_SgArrowExp: {
		SgExpression* ex = isSgArrowExp(expr)->get_rhs_operand();
		if (isSgVarRefExp(ex))
			var = isSgVarRefExp(ex)->get_symbol();
		break;
	}
	case V_SgDotExp: {
		SgExpression* ex = isSgDotExp(expr)->get_rhs_operand();
		if (isSgVarRefExp(ex))
			var = isSgVarRefExp(ex)->get_symbol();
		break;
	}
	case V_SgPointerDerefExp: {
		SgExpression* ex = isSgPointerDerefExp(expr)->get_operand();
		if (isSgVarRefExp(ex))
			var = isSgVarRefExp(ex)->get_symbol();
		break;
	}
	} // switch
	if (var)
		init = var->get_declaration();
	return init;
}

void CompassAnalyses::AssignFloat2Int::Traversal::visit(SgNode* node) {
	SgAssignOp* aop = isSgAssignOp(node);

	if (aop) {
		SgExpression* lexpr = aop->get_lhs_operand();
		if (!lexpr)
			return;
		SgExpression* rexpr = aop->get_rhs_operand();
		if (!rexpr)
			return;

		SgCastExp *cast = isSgCastExp(rexpr);
		if (cast) {
			SgType* toType = cast->get_type()->stripType(
					SgType::STRIP_TYPEDEF_TYPE | SgType::STRIP_MODIFIER_TYPE);
			if (isIntType(toType) && countCast(rexpr->unparseToString()))
				return;
		}

		SgInitializedName* linit = getSgInitializedName(lexpr);
		if (!linit)
			return;
		SgInitializedName* rinit = getSgInitializedName(rexpr);
		if (!rinit)
			return;
		if (isIntType(linit->get_type()) && isFloatType(rinit->get_type())) {
			output->addOutput(new CheckerOutput(node));
		}
	}
} //End of the visit function.

// Checker main run function and metadata

static void run(Compass::Parameters params, Compass::OutputObject* output) {
	CompassAnalyses::AssignFloat2Int::Traversal(params, output).run(
			Compass::projectPrerequisite.getProject());
}

// Remove this function if your checker is not an AST traversal
static Compass::AstSimpleProcessingWithRunFunction* createTraversal(
		Compass::Parameters params, Compass::OutputObject* output) {
	return new CompassAnalyses::AssignFloat2Int::Traversal(params, output);
}

extern const Compass::Checker* const assignFloat2IntChecker =
		new Compass::CheckerUsingAstSimpleProcessing("AssignFloat2Int",
				// Descriptions should not include the newline character "\n".
				"Assign float to integer", "Assign float to integer",
				Compass::C | Compass::Cpp,
				Compass::PrerequisiteList(1, &Compass::projectPrerequisite),
				run, createTraversal);


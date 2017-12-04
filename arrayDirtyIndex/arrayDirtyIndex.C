// -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*-
// vim: expandtab:shiftwidth=2:tabstop=2

// Array Dirty Index Analysis
// Author: Ubuntu-ROSE-Demo-V,,,
// Date: 05-November-2017

#include "rose.h"
#include "compass.h"

using namespace std;

extern const Compass::Checker* const arrayDirtyIndexChecker;

namespace CompassAnalyses {
namespace ArrayDirtyIndex {

typedef std::pair<bool, std::vector<SgExpression*> > BoolWithTrace;

// Specification of Checker Output Implementation
class CheckerOutput: public Compass::OutputViolationBase {
public:
	CheckerOutput(std::string problem, SgNode* node);
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

CompassAnalyses::ArrayDirtyIndex::CheckerOutput::CheckerOutput(
		std::string problem, SgNode* node) :
		OutputViolationBase(node, ::arrayDirtyIndexChecker->checkerName,
				::arrayDirtyIndexChecker->shortDescription) {
}

CompassAnalyses::ArrayDirtyIndex::Traversal::Traversal(
		Compass::Parameters inputParameters, Compass::OutputObject* output) :
		output(output) {
}

template<typename T>
static std::string ToString(T t) {
	std::ostringstream myStream; //creates an ostringstream object
	myStream << t << std::flush;
	return myStream.str(); //returns the string form of the stringstream object
}

static std::map<SgExpression*, CompassAnalyses::ArrayDirtyIndex::BoolWithTrace> traces;

static CompassAnalyses::ArrayDirtyIndex::BoolWithTrace expressionIsDirty(
		SgExpression* expr) {
	if (traces.find(expr) != traces.end()) {
		return traces[expr];
	}
	traces[expr].first = false;
	CompassAnalyses::ArrayDirtyIndex::BoolWithTrace result(false,
			vector<SgExpression*>());

	switch (expr->variantT()) {
	case V_SgVarRefExp: {
		SgVarRefExp* vr = isSgVarRefExp(expr);
		SgInitializedName* var = vr->get_symbol()->get_declaration();
		ROSE_ASSERT(var);
		vector<SgNode*> defs =
				Compass::sourceDefUsePrerequisite.getSourceDefUse()->getDefFor(
						vr, var);
		for (size_t i = 0; i < defs.size(); ++i) {
			SgExpression* def = isSgExpression(defs[i]);
			if (!def) {
				SgInitializedName* DefInit = isSgInitializedName(defs[i]);
				if (DefInit) {
					SgNode* DefInitParent = DefInit->get_parent();
					if (isSgFunctionParameterList(DefInitParent)) {
						result.first = true;
						break;
					}
				}
				result.first = false;
				break;
			}
			CompassAnalyses::ArrayDirtyIndex::BoolWithTrace tr =
					expressionIsDirty(def);
			if (tr.first) {
				result = tr;
				break;
			}
		}
		break;
	}
	case V_SgAddOp: {
		SgExpression* lhs = isSgAddOp(expr)->get_lhs_operand();
		CompassAnalyses::ArrayDirtyIndex::BoolWithTrace tr = expressionIsDirty(
				lhs);
		if (tr.first) {
			result = tr;
			break;
		}
		SgExpression* rhs = isSgAddOp(expr)->get_rhs_operand();
		result = expressionIsDirty(rhs);
		break;
	}
	case V_SgCastExp: {
		SgExpression* op = isSgUnaryOp(expr)->get_operand();
		result = expressionIsDirty(op);
		break;
	}
	case V_SgAssignInitializer: {
		SgExpression* op = isSgAssignInitializer(expr)->get_operand();
		result = expressionIsDirty(op);
		break;
	}
	default: {
		result.first = false;
		break;
	}
	}
	if (result.first) {
		result.second.push_back(expr);
	}
	traces[expr] = result;
	return result;
}

void CompassAnalyses::ArrayDirtyIndex::Traversal::visit(SgNode* sgNode) {

//	dumpNodeInfo(sgNode);

	if (SgPntrArrRefExp* pntr = isSgPntrArrRefExp(sgNode)) {
		SgVarRefExp* rightVar = isSgVarRefExp(pntr->get_rhs_operand());
		BoolWithTrace tr = expressionIsDirty(rightVar);
		if (tr.first) {
			std::string trace = "Argument of free() could be NULL: stack is:\n";
			for (size_t i = tr.second.size(); i > 0; --i) {
				std::string classname = (tr.second[i - 1]->class_name());
				std::string unparsed = (tr.second[i - 1]->unparseToString());
				int line =
						(tr.second[i - 1]->get_startOfConstruct()->get_line());
				trace.append(ToString(i));
				trace.append(": ");
				trace.append(classname);
				trace.append(" ");
				trace.append(unparsed);
				trace.append(" (line ");
				trace.append(ToString(line));
				trace.append(")\n");
			}
			trace += "End of stack\n";
			output->addOutput(new CheckerOutput(trace, rightVar));
		}
	}
} //End of the visit function.

// Checker main run function and metadata

static void run(Compass::Parameters params, Compass::OutputObject* output) {
	CompassAnalyses::ArrayDirtyIndex::Traversal(params, output).run(
			Compass::projectPrerequisite.getProject());
}

// Remove this function if your checker is not an AST traversal
static Compass::AstSimpleProcessingWithRunFunction* createTraversal(
		Compass::Parameters params, Compass::OutputObject* output) {
	return new CompassAnalyses::ArrayDirtyIndex::Traversal(params, output);
}

static Compass::PrerequisiteList getPrerequisites() {
	Compass::PrerequisiteList defusePre;
	defusePre.push_back(&Compass::projectPrerequisite);
	defusePre.push_back(&Compass::sourceDefUsePrerequisite);
	return defusePre;
}

extern const Compass::Checker* const arrayDirtyIndexChecker =
		new Compass::CheckerUsingAstSimpleProcessing("ArrayDirtyIndex",
				// Descriptions should not include the newline character "\n".
				"Index of array is dirty", "Index of array is dirty",
				Compass::C | Compass::Cpp, getPrerequisites(), run,
				createTraversal);


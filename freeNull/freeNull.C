// -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*-
// vim: expandtab:shiftwidth=2:tabstop=2

#include "rose.h"
#include "compass.h"
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include "BoostGraphInterface.hxx"

using namespace std;

extern const Compass::Checker* const freeNullChecker;

namespace CompassAnalyses {
namespace FreeNull {

typedef std::pair<bool, std::vector<SgExpression*> > BoolWithTrace;

/*! \brief Free Null: Add your description here
 */

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

CompassAnalyses::FreeNull::CheckerOutput::CheckerOutput(std::string problem,
		SgNode* node) :
		OutputViolationBase(node, ::freeNullChecker->checkerName, problem) {
}

CompassAnalyses::FreeNull::Traversal::Traversal(
		Compass::Parameters inputParameters, Compass::OutputObject* output) :
		output(output) {
}

template<typename T>
static std::string ToString(T t) {
	std::ostringstream myStream; //creates an ostringstream object
	myStream << t << std::flush;
	return myStream.str(); //returns the string form of the stringstream object
}

static std::map<SgExpression*, CompassAnalyses::FreeNull::BoolWithTrace> traces;

static CompassAnalyses::FreeNull::BoolWithTrace expressionIsNull(
		SgExpression* expr) {
	if (traces.find(expr) != traces.end()) {
		return traces[expr];
	}
	traces[expr].first = false;
	CompassAnalyses::FreeNull::BoolWithTrace result(false,
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
				result.first = true;
				break;
			}
			CompassAnalyses::FreeNull::BoolWithTrace tr = expressionIsNull(def);
			if (tr.first) {
				result = tr;
				break;
			}
		}
		break;
	}
	case V_SgAddOp: {
		SgExpression* lhs = isSgAddOp(expr)->get_lhs_operand();
		result = expressionIsNull(lhs);
		break;
	}
	case V_SgCastExp: {
		SgExpression* op = isSgUnaryOp(expr)->get_operand();
		result = expressionIsNull(op);
		break;
	}
	case V_SgAssignInitializer: {
		SgExpression* op = isSgAssignInitializer(expr)->get_operand();
		result = expressionIsNull(op);
		break;
	}
	case V_SgFunctionCallExp: {
		SgFunctionCallExp* fc = isSgFunctionCallExp(expr);
		SgFunctionRefExp* fr = isSgFunctionRefExp(fc->get_function());
		if (!fr) {
			result.first = true;
		} else {
			string name =
					fr->get_symbol()->get_declaration()->get_name().getString();
			if (name == "malloc") {
				result.first = true; // Check the result of malloc
			} else if (name == "xmalloc") {
				result.first = false; // For testing
			} else {
				result.first = true;
			}
		}
		break;
	}
	case V_SgNewExp: {
		result.first = false;
		break;
	}
	case V_SgThisExp: {
		result.first = false;
		break;
	}
	case V_SgAddressOfOp: {
		result.first = false;
		break;
	}
	case V_SgIntVal: {
		result.first = (isSgIntVal(expr)->get_value() == 0);
		break;
	}
	default: {
		result.first = true;
		break;
	}
	}
	if (result.first) {
		result.second.push_back(expr);
	}
	traces[expr] = result;
	return result;
}

void CompassAnalyses::FreeNull::Traversal::visit(SgNode* node) {
	SgFunctionCallExp* func = isSgFunctionCallExp(node);
	if (func) {
		SgFunctionRefExp *fref = isSgFunctionRefExp(func->get_function());
		SgMemberFunctionRefExp *fmem = isSgMemberFunctionRefExp(
				func->get_function());
		std::string func_name_str("");
		if (fref)
			func_name_str = fref->get_symbol()->get_name().getString();
		if (fmem)
			func_name_str = fmem->get_symbol()->get_name().getString();
		if (func_name_str.compare("free") == 0) {
			SgExprListExp* list = func->get_args();
			Rose_STL_Container<SgExpression*> plist = list->get_expressions();
			if (plist.size() > 1)
				return;
			SgExpression* argument = *plist.begin();
			while (isSgUnaryOp(argument) != NULL)
				argument = isSgUnaryOp(argument)->get_operand();
			BoolWithTrace tr = expressionIsNull(argument);
			if (tr.first) {
				std::string trace =
						"Argument of free() could be NULL: stack is:\n";
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
				output->addOutput(new CheckerOutput(trace, argument));
			}
		}
	}
} //End of the visit function.

// Checker main run function and metadata

static void run(Compass::Parameters params, Compass::OutputObject* output) {
	CompassAnalyses::FreeNull::Traversal(params, output).run(
			Compass::projectPrerequisite.getProject());
}

// Remove this function if your checker is not an AST traversal
static Compass::AstSimpleProcessingWithRunFunction* createTraversal(
		Compass::Parameters params, Compass::OutputObject* output) {
	return new CompassAnalyses::FreeNull::Traversal(params, output);
}

static Compass::PrerequisiteList getPrerequisites() {
	Compass::PrerequisiteList defusePre;
	defusePre.push_back(&Compass::projectPrerequisite);
	defusePre.push_back(&Compass::sourceDefUsePrerequisite);
	return defusePre;
}

extern const Compass::Checker* const freeNullChecker =
		new Compass::CheckerUsingAstSimpleProcessing("FreeNull",
				// Descriptions should not include the newline character "\n".
				"Short description not written yet!",
				"Long description not written yet!", Compass::C | Compass::Cpp,
				getPrerequisites(), run, createTraversal);


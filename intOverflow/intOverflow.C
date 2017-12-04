// -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*-
// vim: expandtab:shiftwidth=2:tabstop=2

// Int Overflow Analysis
// Author: Ubuntu-ROSE-Demo-V,,,
// Date: 06-November-2017

#include "rose.h"
#include "compass.h"

#define IS_ILLEGAL_CONSTANT(x) (x && is_illegal_constant(x))
#define IS_ZERO(x) (x && 0 == x->get_value())

extern const Compass::Checker* const intOverflowChecker;

using namespace std;

namespace CompassAnalyses {
namespace IntOverflow {

typedef std::pair<bool, std::vector<SgExpression*> > BoolWithTrace;

bool is_zero_value(SgValueExp* n);
bool is_illegal_constant(SgInitializer* n);
bool is_illegal_constant(SgValueExp* n);
bool is_illegal_constant(SgExpression* n);
bool is_illegal_constant(SgCastExp* n);
bool is_illegal_constant(SgAssignInitializer* n);
bool is_illegal_constant(SgBinaryOp* n);
bool has_illegal_constant(SgAggregateInitializer* n);
bool is_constant(SgExpression* n);
bool is_pointer(SgType* n);

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

bool CompassAnalyses::IntOverflow::is_zero_value(SgValueExp* value_node) {
	return (IS_ZERO(isSgBoolValExp(value_node))
			|| IS_ZERO(isSgCharVal(value_node))
			// Complex
			|| IS_ZERO(isSgDoubleVal(value_node))
			// Enum
			|| IS_ZERO(isSgFloatVal(value_node))
			|| IS_ZERO(isSgIntVal(value_node))
			|| IS_ZERO(isSgLongDoubleVal(value_node))
			|| IS_ZERO(isSgLongIntVal(value_node))
			|| IS_ZERO(isSgLongLongIntVal(value_node))
			|| IS_ZERO(isSgShortVal(value_node))
			// String
			|| IS_ZERO(isSgUnsignedCharVal(value_node))
			|| IS_ZERO(isSgUnsignedIntVal(value_node))
			|| IS_ZERO(isSgUnsignedLongLongIntVal(value_node))
			|| IS_ZERO(isSgUnsignedLongVal(value_node))
			// UpcMyThread
			// UpcThreads
			|| IS_ZERO(isSgWcharVal(value_node)));
}

bool CompassAnalyses::IntOverflow::is_illegal_constant(SgInitializer* node) {
	return IS_ILLEGAL_CONSTANT(isSgAssignInitializer(node));
}

bool CompassAnalyses::IntOverflow::is_illegal_constant(SgValueExp* node) {
	return !is_zero_value(node);
}

bool CompassAnalyses::IntOverflow::is_constant(SgExpression* node) {
	return isSgValueExp(node)
			|| (isSgCastExp(node)
					&& is_constant(isSgCastExp(node)->get_operand()));
}

bool CompassAnalyses::IntOverflow::is_illegal_constant(SgBinaryOp* node) {
	// This won't catch things like 2 + 0, e.g.
	//    int* bad = ((int*) 2) + 0;
	SgExpression* l_op = node->get_lhs_operand();
	SgExpression* r_op = node->get_rhs_operand();

	return is_constant(l_op) && is_constant(r_op)
			&& (is_illegal_constant(l_op) || is_illegal_constant(r_op));
}

bool CompassAnalyses::IntOverflow::is_illegal_constant(SgExpression* node) {
	return (IS_ILLEGAL_CONSTANT(isSgValueExp(node))
			|| IS_ILLEGAL_CONSTANT(isSgCastExp(node))
			|| IS_ILLEGAL_CONSTANT(isSgBinaryOp(node)));
}

bool CompassAnalyses::IntOverflow::is_illegal_constant(SgCastExp* node) {
	return is_illegal_constant(node->get_operand());
}

bool CompassAnalyses::IntOverflow::is_illegal_constant(
		SgAssignInitializer* node) {
	return is_illegal_constant(node->get_operand());
}

bool CompassAnalyses::IntOverflow::has_illegal_constant(
		SgAggregateInitializer* node) {
	Rose_STL_Container<SgExpression*>::iterator i;
	Rose_STL_Container<SgExpression*> list =
			node->get_initializers()->get_expressions();

	for (i = list.begin(); i != list.end(); ++i) {
		if (IS_ILLEGAL_CONSTANT(isSgAssignInitializer(*i)))
			return true;
	}

	return false;
}

CompassAnalyses::IntOverflow::CheckerOutput::CheckerOutput(std::string problem,
		SgNode* node) :
		OutputViolationBase(node, ::intOverflowChecker->checkerName,
				::intOverflowChecker->shortDescription) {
}

CompassAnalyses::IntOverflow::Traversal::Traversal(
		Compass::Parameters inputParameters, Compass::OutputObject* output) :
		output(output) {
}

template<typename T>
static std::string ToString(T t) {
	std::ostringstream myStream; //creates an ostringstream object
	myStream << t << std::flush;
	return myStream.str(); //returns the string form of the stringstream object
}

static std::map<SgExpression*, CompassAnalyses::IntOverflow::BoolWithTrace> traces;

static CompassAnalyses::IntOverflow::BoolWithTrace expressionIsDirty(
		SgNode* node) {
	CompassAnalyses::IntOverflow::BoolWithTrace result(false,
			vector<SgExpression*>());
	if (!isSgExpression(node)) {
		return result;
	}

	SgExpression* expr = isSgExpression(node);
	if (traces.find(expr) != traces.end()) {
		return traces[expr];
	}
	traces[expr].first = false;

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
			CompassAnalyses::IntOverflow::BoolWithTrace tr = expressionIsDirty(
					def);
			if (tr.first) {
				result = tr;
				break;
			}
		}
		break;
	}
	case V_SgAddOp: {
		SgExpression* lhs = isSgAddOp(expr)->get_lhs_operand();
		CompassAnalyses::IntOverflow::BoolWithTrace tr = expressionIsDirty(lhs);
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

static CompassAnalyses::IntOverflow::BoolWithTrace dirtyBinaryOp(SgNode* node) {
	std::string nameNode = node->class_name();
	if (isSgAddOp(node) || isSgSubtractOp(node) || isSgDivideOp(node)
			|| isSgMultiplyOp(node)) {
		std::vector<SgNode*> childrenRec =
				node->get_traversalSuccessorContainer();
		if (childrenRec.size() > 0) {
			bool foundConstValue = false;
			for (unsigned int i = 0; i < childrenRec.size(); i++) {
				SgNode* node = childrenRec[i];
				if (isSgAddOp(node) || isSgSubtractOp(node)
						|| isSgDivideOp(node) || isSgMultiplyOp(node)) {
					return dirtyBinaryOp(node);
				}
				if (isSgDoubleVal(node) || isSgFloatVal(node)
						|| isSgIntVal(node)) {
					if (foundConstValue) {
						CompassAnalyses::IntOverflow::BoolWithTrace br =
								expressionIsDirty(node);
						if (br.first)
							return br;
					}
					foundConstValue = true;
				}
				if (isSgCastExp(node)) {
					std::vector<SgNode*> childrenCast =
							node->get_traversalSuccessorContainer();
					node = childrenCast[0];
					if (isSgAddOp(node) || isSgSubtractOp(node)
							|| isSgDivideOp(node) || isSgMultiplyOp(node)) {
						return dirtyBinaryOp(node);
					}
				}
				if (isSgVarRefExp(node)) {
					SgVarRefExp* refexp = isSgVarRefExp(node);
					SgVariableSymbol* symbol = refexp->get_symbol();
					SgType* type = symbol->get_type();
					if (isSgTypeInt(type->findBaseType())
							|| isSgTypeDouble(type->findBaseType())
							|| isSgTypeFloat(type->findBaseType())) {
						CompassAnalyses::IntOverflow::BoolWithTrace br =
								expressionIsDirty(node);
						if (br.first)
							return br;
						foundConstValue = true;
					}
				}
				if (isSgPointerDerefExp(node)) {
					SgPointerDerefExp* deref = isSgPointerDerefExp(node);
					std::vector<SgNode*> children =
							deref->get_traversalSuccessorContainer();
					for (unsigned int i = 0; i < children.size(); i++) {
						SgNode* nodeChild = children[i];
						if (isSgVarRefExp(nodeChild)) {
							SgVarRefExp* refexp = isSgVarRefExp(nodeChild);
							SgVariableSymbol* symbol = refexp->get_symbol();
							SgType* type = symbol->get_type();
							if (isSgTypeInt(type->findBaseType())
									|| isSgTypeDouble(type->findBaseType())
									|| isSgTypeFloat(type->findBaseType())) {
								CompassAnalyses::IntOverflow::BoolWithTrace br =
										expressionIsDirty(node);
								if (br.first)
									return br;
								foundConstValue = true;
							}
						}
					}
				}
				if (isSgPntrArrRefExp(node)) {
					SgPntrArrRefExp* deref = isSgPntrArrRefExp(node);
					std::vector<SgNode*> childrenExp =
							deref->get_traversalSuccessorContainer();
					for (unsigned int i = 0; i < childrenExp.size(); i++) {
						SgNode* nodeChild2 = childrenExp[i];
						if (isSgVarRefExp(nodeChild2)) {
							SgVarRefExp* refexp = isSgVarRefExp(nodeChild2);
							SgVariableSymbol* symbol = refexp->get_symbol();
							SgType* type = symbol->get_type();
							if (isSgTypeInt(type->findBaseType())
									|| isSgTypeDouble(type->findBaseType())
									|| isSgTypeFloat(type->findBaseType())) {
								CompassAnalyses::IntOverflow::BoolWithTrace br =
										expressionIsDirty(node);
								if (br.first)
									return br;
								foundConstValue = true;
							}
						}
					}
				}
			}
		}
	}
	CompassAnalyses::IntOverflow::BoolWithTrace br;
	br.first = false;
	return br;
}

static void dumpNodeInfo(SgNode* TheNode) {
	static std::set<SgNode*> DoneSet;

	std::string ClassName = TheNode->class_name();
	if (ClassName.compare("SgProject") == 0
			|| ClassName.compare("SgFileList") == 0
			|| ClassName.compare("SgSourceFile") == 0)
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

void CompassAnalyses::IntOverflow::Traversal::visit(SgNode* node) {
//	dumpNodeInfo(node);
//	return;

//	SgAddOp* AOp = isSgAddOp(node);
//	if (AOp) {
//		std::cerr << "Found add operator" << std::endl;
//		if (isSgBinaryOp(AOp))
//			std::cerr << "Found binary operator" << std::endl;
//	}

	BoolWithTrace br = dirtyBinaryOp(node);
	if (br.first) {
		std::string trace = "Argument of free() could be NULL: stack is:\n";
		for (size_t i = br.second.size(); i > 0; --i) {
			std::string classname = (br.second[i - 1]->class_name());
			std::string unparsed = (br.second[i - 1]->unparseToString());
			int line = (br.second[i - 1]->get_startOfConstruct()->get_line());
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
		output->addOutput(new CheckerOutput(trace, node));
	}
} //End of the visit function.

static void run(Compass::Parameters params, Compass::OutputObject* output) {
	CompassAnalyses::IntOverflow::Traversal(params, output).run(
			Compass::projectPrerequisite.getProject());
}

// Remove this function if your checker is not an AST traversal
static Compass::AstSimpleProcessingWithRunFunction* createTraversal(
		Compass::Parameters params, Compass::OutputObject* output) {
	return new CompassAnalyses::IntOverflow::Traversal(params, output);
}

static Compass::PrerequisiteList getPrerequisites() {
	Compass::PrerequisiteList defusePre;
	defusePre.push_back(&Compass::projectPrerequisite);
	defusePre.push_back(&Compass::sourceDefUsePrerequisite);
	return defusePre;
}

extern const Compass::Checker* const intOverflowChecker =
		new Compass::CheckerUsingAstSimpleProcessing("IntOverflow",
				// Descriptions should not include the newline character "\n".
				"Integer overflow may happen", "Integer overflow may happen",
				Compass::C | Compass::Cpp, getPrerequisites(), run,
				createTraversal);


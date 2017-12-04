// -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*-
// vim: expandtab:shiftwidth=2:tabstop=2

// Test Checker Analysis
// Author: Ubuntu-ROSE-Demo-V,,,
// Date: 12-October-2017

#include "rose.h"
#include "compass.h"

extern const Compass::Checker* const testCheckerChecker;

// DQ (1/17/2009): Added declaration to match external defined in file:
// rose/projects/compass/extensions/prerequisites/ProjectPrerequisite.h
// I can't tell that it is defined anywhere in compass except the extern 
// declaration in ProjectPrerequisite.h
Compass::ProjectPrerequisite Compass::projectPrerequisite;

namespace CompassAnalyses
   { 
     namespace TestChecker
        { 
        /*! \brief Test Checker: Add your description here 
         */

       // Specification of Checker Output Implementation
          class CheckerOutput: public Compass::OutputViolationBase
             { 
               public:
                    CheckerOutput(SgNode* node);
             };

       // Specification of Checker Traversal Implementation

          class Traversal
             : public Compass::AstSimpleProcessingWithRunFunction 
             {
                    Compass::OutputObject* output;
            // Checker specific parameters should be allocated here.

               public:
                    Traversal(Compass::Parameters inputParameters, Compass::OutputObject* output);

                 // Change the implementation of this function if you are using inherited attributes.
                    void *initialInheritedAttribute() const { return NULL; }

                 // The implementation of the run function has to match the traversal being called.
                 // If you use inherited attributes, use the following definition:
                 // void run(SgNode* n){ this->traverse(n, initialInheritedAttribute()); }
                    void run(SgNode* n){ this->traverse(n, preorder); }

                 // Change this function if you are using a different type of traversal, e.g.
                 // void *evaluateInheritedAttribute(SgNode *, void *);
                 // for AstTopDownProcessing.
                    void visit(SgNode* n);
             };
        }
   }

CompassAnalyses::TestChecker::
CheckerOutput::CheckerOutput ( SgNode* node )
   : OutputViolationBase(node,::testCheckerChecker->checkerName,::testCheckerChecker->shortDescription)
   {}

CompassAnalyses::TestChecker::Traversal::
Traversal(Compass::Parameters inputParameters, Compass::OutputObject* output)
   : output(output)
   {
  // Initalize checker specific parameters here, for example: 
  // YourParameter = Compass::parseInteger(inputParameters["TestChecker.YourParameter"]);


   }

void
CompassAnalyses::TestChecker::Traversal::
visit(SgNode* node)
   { 
  // Implement your traversal here.  

   } //End of the visit function.

// Checker main run function and metadata

static void run(Compass::Parameters params, Compass::OutputObject* output) {
  CompassAnalyses::TestChecker::Traversal(params, output).run(Compass::projectPrerequisite.getProject());
}

// Remove this function if your checker is not an AST traversal
static Compass::AstSimpleProcessingWithRunFunction* createTraversal(Compass::Parameters params, Compass::OutputObject* output) {
  return new CompassAnalyses::TestChecker::Traversal(params, output);
}

extern const Compass::Checker* const testCheckerChecker =
  new Compass::CheckerUsingAstSimpleProcessing(
        "TestChecker",
     // Descriptions should not include the newline character "\n".
        "Short description not written yet!",
        "Long description not written yet!",
        Compass::C | Compass::Cpp,
        Compass::PrerequisiteList(1, &Compass::projectPrerequisite),
        run,
        createTraversal);
   

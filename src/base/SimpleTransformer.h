//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2014 The MINOTAUR Team.
//

/**
 * \file SimpleTransformer.h
 * \brief Declare class for simple transformation of problem.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURSIMPLETRANSFORMER_H
#define MINOTAURSIMPLETRANSFORMER_H

#include "Types.h"
#include "Transformer.h"

namespace Minotaur {
class CxQuadHandler;
class CxUnivarHandler;
class CGraph;
class CNode;
class Environment;
class LinearHandler;
class Problem;
class Solution;
class YEqCGs;
class YEqLFs;
class YEqVars;
typedef boost::shared_ptr<CxQuadHandler> CxQuadHandlerPtr;
typedef boost::shared_ptr<CxUnivarHandler> CxUnivarHandlerPtr;
typedef boost::shared_ptr<CGraph> CGraphPtr;
typedef boost::shared_ptr<Environment> EnvPtr;
typedef boost::shared_ptr<LinearHandler> LinearHandlerPtr;
typedef boost::shared_ptr<Problem> ProblemPtr;
typedef boost::shared_ptr<Solution> SolutionPtr;
typedef boost::shared_ptr<const Solution> ConstSolutionPtr;


/**
 * \brief Class for reformulating a problem using simple rules so that
 * handlers can be applied to it.
 *
 * No multilinear terms are created. QuadHandler is used only for terms
 * \f$y=x_1x_2\f$. Squares etc. are handled by PowerHandler. ExpHandler takes
 * care of exponential functions, and LogHandler handles logarithms.
 * TrigHandler is used for trigonometric functions. Mainly used to teach Ashu
 * some global optimization.
 */
class SimpleTransformer : public Transformer {
public:

  /// Default Constructor.
  SimpleTransformer();

  /// Constructor.
  SimpleTransformer(EnvPtr env);

  /// Destroy.
  ~SimpleTransformer();

  // base class method.
  SolutionPtr getSolOrig(ConstSolutionPtr sol, Int &err);

  // base class method.
  SolutionPtr getSolTrans(ConstSolutionPtr sol, Int &err);

  // base class method.
  void reformulate(ConstProblemPtr oldp, ProblemPtr &newp, 
                   HandlerVector &handlers, Int &status);


private:
  static const std::string me_;

  YEqCGs *yBiVars_;

  ProblemPtr newp_;

  void absRef_(LinearFunctionPtr lfl, VariablePtr vl, Double dl,
               VariablePtr &v, Double &d);

  void bilRef_(LinearFunctionPtr lfl, VariablePtr vl, Double dl,
               LinearFunctionPtr lfr, VariablePtr vr, Double dr,
               LinearFunctionPtr &lf, VariablePtr &v, Double &d);

  VariablePtr newBilVar_(VariablePtr vl, VariablePtr vr);

  void powKRef_(LinearFunctionPtr lfl,
                VariablePtr vl, Double dl, Double k,
                LinearFunctionPtr &lf, VariablePtr &v,
                Double &d);

  /**
   * \brief Reformulate the nonlinear constraints of the problem.
   *
   * \param [in] oldp Original problem.
   */
  void refNonlinCons_(ConstProblemPtr oldp);
    
  /**
   * \brief Reformulate the nonlinear objective of the problem.
   *
   * \param [in] oldp Original problem.
   */
  void refNonlinObj_(ConstProblemPtr oldp);
    
  /**
   * TODO
   */
  void recursRef_(const CNode *node, LinearFunctionPtr &lf, VariablePtr &v,
                  Double &d);

  void trigRef_(OpCode op, LinearFunctionPtr lfl, VariablePtr vl,
                Double dl, VariablePtr &v, Double &d);

  void uniVarRef_(const CNode *n0, LinearFunctionPtr lfl, 
                  VariablePtr vl, Double dl, 
                  LinearFunctionPtr &lf, VariablePtr &v, Double &d);

};

typedef boost::shared_ptr<SimpleTransformer> SimpTranPtr;
typedef boost::shared_ptr<const SimpleTransformer> ConstSimpTranPtr;

}

#endif


// Local Variables: 
// mode: c++ 
// eval: (c-set-style "k&r") 
// eval: (c-set-offset 'innamespace 0) 
// eval: (setq c-basic-offset 2) 
// eval: (setq fill-column 78) 
// eval: (auto-fill-mode 1) 
// eval: (setq column-number-mode 1) 
// eval: (setq indent-tabs-mode nil) 
// End:

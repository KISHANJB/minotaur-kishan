// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2017 The MINOTAUR Team.
// 

/**
 * \file QGHandlerAdvance.h
 * \Briefly declare a derived class of Handler with enhanced feastures for the Quesada-Grossmann algorithm.
 * \Author Meenarli Sharma, Indian Institute of Techonology Bombay.
 */

#ifndef MINOTAURQGHANDLERADVANCE_H
#define MINOTAURQGHANDLERADVANCE_H

#include <stack>

#include "Handler.h"
#include "Engine.h"
#include "Problem.h"
#include "Function.h"
#include "Linearizations.h"

#include "Solution.h"

namespace Minotaur {

struct QGStats {
  size_t nlpS;      /// Number of nlps solved.
  size_t nlpF;      /// Number of nlps feasible.
  size_t nlpI;      /// Number of nlps infeasible.
  size_t nlpIL;     /// Number of nlps hits engine iterations limit.
  size_t cuts;    /// Number of cuts at int feas nodes.
  size_t fracCuts;    /// Number of cuts at int feas nodes.
}; 


/**
 * \brief Handler for convex constraints, based on quesada-grossmann
 * algorithm.
 *
 * QGHandlerAdvance is a derived class of Handler. It adds cuts generated by solving
 *  an NLP whenever an integer (but infeasible) solution of LP relaxation is 
 *  found. It considers nonlinear constraints in the form f(x) <= b.
 */
class QGHandlerAdvance : public Handler {

private: 
  /// Pointer to environment.
  EnvPtr env_;

  /// Tolerance for checking integrality (should be obtained from env).
  double intTol_;

  /// Log.
  LoggerPtr logger_;

  /// For log:
  static const std::string me_;

  /// Pointer to original problem.
  ProblemPtr minlp_;
  
  Timer *timer_;

  /// Vector of nonlinear constraints.
  std::vector<ConstraintPtr> nlCons_;
  
  std::vector<ConstraintPtr> highDualCons_;
  
  /// NLP/QP Engine used to solve the NLP/QP relaxations.
  EnginePtr nlpe_;
  
  EnginePtr lpe_;

  /// Modifications done to NLP before solving it.
  std::stack<Modification *> nlpMods_;

  /// Status of the NLP/QP engine.
  EngineStatus nlpStatus_;

  double * solC_;
  /**
   * The variable corresponding to the objective function. It is a part of
   * all linearizations of the objective function and it appears in the
   * objective.
   */
  VariablePtr objVar_;

  /// Nonlinearity status of objective function. 1 if nonlinear 0 otherwise.
  bool oNl_;

  /// Pointer to relaxation of the problem.
  RelaxationPtr rel_;

  /// Value of objective in relaxation solution
  double relobj_; 

  /// Absolute tolerance for constraint feasibility.
  double solAbsTol_;

  /// Relative tolerance for constraint feasibility.
  double solRelTol_;

  /// Absolute tolerance for pruning a node.
  double objATol_;

  /// Relative tolerance for pruning a node.
  double objRTol_;
  
  LinearizationsPtr extraLin_;

  int rs3_;
  
  double maxVioPer_;
  
  std::string cutMethod_;

  double maxDist_;
  
  //double lpdist_;
  //EngineStatus shortestNlpStatus_;
  
  int lastNodeId_;

  /// Statistics.
  QGStats *stats_;

public:
  /**
   * \brief Default Constructor.
   *
   * \param [in] env Environment pointer.
   * \param [in] minlp The minlp for which cuts are generated (Not the
   * relaxation.)
   * \param [in] nlpe The engine to solve nonlinear continuous problem.
   */
  QGHandlerAdvance(EnvPtr env, ProblemPtr minlp, EnginePtr nlpe); 
  
  /// Destroy.
  ~QGHandlerAdvance();
   
  /// Does nothing.
  Branches getBranches(BrCandPtr, DoubleVector &, RelaxationPtr,
                       SolutionPoolPtr)
  {return Branches();};

  /// Does nothing.
  void getBranchingCandidates(RelaxationPtr, 
                              const DoubleVector &, ModVector &,
                              BrVarCandSet &, BrCandVector &, bool &) {};

  /// Does nothing.
  ModificationPtr getBrMod(BrCandPtr, DoubleVector &, RelaxationPtr,
                           BranchDirection)
  {return ModificationPtr();};

       
  // Base class method. 
  std::string getName() const;

  // Base class method. Check if x is feasible. x has to satisfy integrality
  // and also nonlinear constraints.
  bool isFeasible(ConstSolutionPtr sol, RelaxationPtr relaxation, 
                  bool & should_prune, double &inf_meas);

  /// Does nothing.
  SolveStatus presolve(PreModQ *, bool *) {return Finished;};

  /// Does nothing.
  bool presolveNode(RelaxationPtr, NodePtr, SolutionPoolPtr, ModVector &,
                    ModVector &)
  {return false;};

  /// Does nothing.
  void postsolveGetX(const double *, UInt, DoubleVector *) {};

  /// Base class method. calls relax_().
  void relaxInitFull(RelaxationPtr rel, bool *is_inf);

  /// Base class method. calls relax_().
  void relaxInitInc(RelaxationPtr rel, bool *is_inf);

  /// Base class method. Does nothing.
  void relaxNodeFull(NodePtr node, RelaxationPtr rel, bool *is_inf);

  /// Base class method. Does nothing.
  void relaxNodeInc(NodePtr node, RelaxationPtr rel, bool *is_inf);

 
  /// Base class method. Find cuts.
  void separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel, 
                CutManager *cutman, SolutionPoolPtr s_pool, ModVector &p_mods,
                ModVector &r_mods, bool *sol_found, SeparationStatus *status);
 

  void setLpEngine(EnginePtr lpe) {lpe_ = lpe;};

  /// Show statistics.
  void writeStats(std::ostream &out) const;

  ///MS: delete after use
  //ConstSolutionPtr getRootNLPSol_() {return rootNLPSol_;}

private:
  /**
   * Add linearization of nonlinear constraints and objective at point x* 
   * to the relaxation only (not to the lp engine)
   */

  void addInitLinearX_(ConstSolutionPtr sol);

  /**
   * Solve NLP by fixing integer variables at LP solution and add 
   * outer-approximation cuts to constraints and/or objective.
   */
  //void cutIntSol_(ConstSolutionPtr sol, CutManager *cutMan, 
  void cutIntSol_(const double *lpx, CutManager *cutMan, 
                  SolutionPoolPtr s_pool, bool *sol_found, 
                  SeparationStatus *status);


  void ESHTypeCut_(const double *lpx, CutManager *cutMan,
                             ConstraintPtr con);

  void findCenter_();
  /**
   * Fix integer constrained variables to integer values in x. Called
   * before solving NLP.
   */
  void fixInts_(const double *x);
  
  /**
   * Solve the NLP relaxation of the MINLP and add linearizations about
   * the optimal point. isInf is set to true if the relaxation is found
   * infeasible. Throw an assert if the relaxation is unbounded.
   */
  void initLinear_(bool *isInf);

  /**
   * Obtain the linear function (lf) and constant (c) from the
   * linearization of function f at point x.
   */
  void linearAt_(FunctionPtr f, double fval, const double *x, 
                 double *c, LinearFunctionPtr *lf, int *error);

  /** 
   * When the objective function is nonlinear, we need to replace it with
   * a single variable.
   */
  void linearizeObj_();

  /**
   * Check which nonlinear constraints are violated at the LP solution and
   * add OA cuts. Return number of OA cuts added.
   */
  void cutToCons_(const double *nlpx, const double *lpx, CutManager *,
                    SeparationStatus *status);
  
  /// Add OA cut to a violated constraint.   
  void addCut_(const double *nlpx, const double *lpx, ConstraintPtr con, 
               CutManager *cutman, SeparationStatus *status);
  
  void ECPTypeCut_(const double *x, CutManager *, ConstraintPtr con, double act);

  void consCutsAtLpSol_(const double *lpx, CutManager *cutman,
                    SeparationStatus *status);


  bool isIntFeas_(const double* x);

  void maxVio_(const double *x, NodePtr node, bool *sol_found,
                               SolutionPoolPtr s_pool, CutManager *cutMan,
                               SeparationStatus *status);

  void objCutAtLpSol_(const double *lpx, CutManager *,
                                  SeparationStatus *status);
  //void objCutAtLpSol_(const double *lpx, CutManager *cutman,
                    //SeparationStatus *status);

      
  /**
   * Check if objective is violated at the LP solution and
   * add OA cut.
   */
  void cutToObj_(const double *nlpx, const double *lpx, CutManager *,
                   SeparationStatus *status);

  /**
   * Create the initial relaxation. It is called from relaxInitFull and
   * relaxInitInc functions.
   */
  void relax_(bool *is_inf);

  void solveCenterNLP_();

  /// Solve the nlp.
  void solveNLP_();

  /// Undo the changes done in fixInts_().
  void undoMods_();

  /**
   * Update the upper bound. XXX: Needs proper integration with
   * Minotaur's Handler design. 
   */
  void updateUb_(SolutionPoolPtr s_pool, double nlpval, bool *sol_found);

  //void shortestDist_(ConstSolutionPtr sol);

  };

  typedef QGHandlerAdvance* QGHandlerAdvancePtr;
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

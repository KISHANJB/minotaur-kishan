// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

/**
 * \file QPDRelaxer.h
 * \brief Declare the QPDRelaxer class. 
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */


#ifndef MINOTAURQPDRELAXER_H
#define MINOTAURQPDRELAXER_H

#include "NodeRelaxer.h"
#include "Types.h"

namespace Minotaur {

  //class Engine;
  class Logger;
  //class Problem;


  /**
   * QPDRelaxer creates ``relaxation'' by 
   * creating a QP approximation. It does not really create a relaxation.
   * However, the Relaxation class suffices for its purposes. If we dived on a
   * node, then we don't create another QP, but just change the bounds.
   */
  class QPDRelaxer : public NodeRelaxer {
  public:
    /// Default constructor.
    QPDRelaxer(EnvPtr env, ProblemPtr p, EnginePtr qe, EnginePtr e);

    /// Destroy.
    ~QPDRelaxer();

    // Implement NodeRelaxer::CreateRootRelaxation().
    RelaxationPtr createRootRelaxation(NodePtr rootNode, bool &prune);

    // Implement NodeRelaxer::CreateNodeRelaxation().
    RelaxationPtr createNodeRelaxation(NodePtr node, bool dived, bool &prune); 

    // Implement NodeRelaxer::reset().
    void reset(NodePtr node, bool diving);

    // get the relaxation pointer, qp_.
    RelaxationPtr getRelaxation();

  private:
    /// Environment
    EnvPtr env_;

    /// Original Problem.
    ProblemPtr p_;

    /**
     * We only keep one QP formulation. We will clear all constraints (but
     * not variables) and rebuild them if needed.
     */
    RelaxationPtr qp_;

    /// Engine used to solve QP.
    EnginePtr qpe_;

    /// Engine used to solve NLP.
    EnginePtr e_;

    /// Logger.
    Logger *logger_;

  };

  typedef QPDRelaxer* QPDRelaxerPtr;
}
#endif


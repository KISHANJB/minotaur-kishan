// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2025 The Minotaur Team.
// 

/**
 * \file QPDRelaxer.h
 * \brief Define two functions of QPDRelaxer class. 
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */


#include "MinotaurConfig.h"
#include "Engine.h"
#include "Logger.h"
#include "Node.h"
#include "QPDRelaxer.h"
#include "Relaxation.h"
#include "SolutionPool.h"
#include "WarmStart.h"

using namespace Minotaur;

QPDRelaxer::QPDRelaxer(EnvPtr env, ProblemPtr p, EnginePtr qe, 
    EnginePtr e)
: env_(env),
  p_(p),
  qpe_(qe),
  e_(e),
  logger_(0)
{
  logger_ = new Logger(LogDebug2);
}


QPDRelaxer::~QPDRelaxer()
{
  //qp_.reset();
  //if (qp_) {
  //  qp_.reset();
  //}
  //p_.reset();
  //env_.reset();
  env_ = 0;
  p_ = 0;
  if (qp_){
    qp_ = 0;
  }
  if (logger_) {
    delete logger_;
  }
}


RelaxationPtr QPDRelaxer::createRootRelaxation(NodePtr, bool &prune)
{
  prune = false;
  qp_ = (RelaxationPtr) new Relaxation(env_); // empty, but not NULL
  qp_->setProblem(p_);

  if (e_) {
    p_->setNativeDer();
    e_->load(p_);
  }
  return qp_;
}


RelaxationPtr QPDRelaxer::createNodeRelaxation(NodePtr node, bool dived, 
                                               bool &prune)
{
  NodePtr t_node; 
  WarmStartPtr ws;

  prune = false;
  if (!dived) {
    // traceback to root and put in all modifications that need to go into the
    // relaxation and the engine.
    std::stack<NodePtr> predecessors;
    t_node = node->getParent();

    while (t_node) {
      predecessors.push(t_node);
      t_node = t_node->getParent();
    }

    // starting from the top, put in modifications made at each node to the
    // engine
    while (!predecessors.empty()) {
      t_node = predecessors.top();
      t_node->applyMods(qp_, p_);
      predecessors.pop();
    }
  } 

  // put in the modifications that were used to create this node from
  // its parent.
  node->applyMods(qp_, p_);

  // give the warm start info to the engine.
  ws = node->getWarmStart();
  if (ws) {
    qpe_->loadFromWarmStart(ws);
    node->removeWarmStart();
  }
  return qp_;
}


RelaxationPtr QPDRelaxer::getRelaxation()
{
  return qp_;
}


void QPDRelaxer::reset(NodePtr node, bool diving)
{
  if (!diving) {
    NodePtr t_node = node;
    while (t_node) {
      t_node->undoMods(qp_, p_);
      t_node = t_node->getParent();
    }
  }
}



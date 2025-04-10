// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

/**
 * \file NodeFullRelaxer.cpp
 * \brief Define the class NodeFullRelaxer.
 * \author Jeff Linderoth, UW-Madison
 */

#include <iostream>
#include <stack>

#include "MinotaurConfig.h"
#include "Engine.h"
#include "Function.h"
#include "Handler.h"
#include "LinearFunction.h"
#include "Node.h"
#include "NodeFullRelaxer.h"
#include "Objective.h"
#include "Relaxation.h"
#include "Variable.h"
#include "VarBoundMod.h"

// #define DEBUG_NODEFULLRELAXER


using namespace Minotaur;


NodeFullRelaxer::NodeFullRelaxer (EnvPtr env, HandlerVector handlers) 
  : env_(env),
    rel_(RelaxationPtr()), // NULL
    engine_(EnginePtr()),  // NULL
    handlers_(handlers)
{
  updateBoundsTol_ = 1.0e-6;
}

NodeFullRelaxer::NodeFullRelaxer (EnvPtr env, EnginePtr e, HandlerVector handlers) 
  : env_(env),
    rel_(RelaxationPtr()), // NULL
    engine_(e),  
    handlers_(handlers)
{
  updateBoundsTol_ = 1.0e-6;
}

NodeFullRelaxer::NodeFullRelaxer () 
  : env_(EnvPtr()),
    rel_(RelaxationPtr()), // NULL
    engine_(EnginePtr()),  // NULL
    handlers_(0)
{
  updateBoundsTol_ = 1.0e-6;
}


NodeFullRelaxer::~NodeFullRelaxer ()
{
  handlers_.clear();
  //rel_.reset();
  rel_ = 0;
  //env_.reset();
  env_ = 0;
}


RelaxationPtr NodeFullRelaxer::createRootRelaxation(NodePtr root,
                                                    SolutionPool *sp,
                                                    bool &prune)
{

  prune = false;
  rel_ = (RelaxationPtr) new Relaxation(env_);
  for (HandlerIterator h = handlers_.begin(); h != handlers_.end() && !prune; 
      ++h) {
    (*h)->relaxInitFull(rel_, sp, &prune);
  }
  if (!prune) {
    rel_->calculateSize();
  }

  engine_->load(rel_);  

  //Here we do strong pre-processing, and make changes to root
  if (!prune) {
    bool changedNode = strongBoundsTighten_(root);
    if (changedNode) {
      // Delete relaxation: Not sure how.  Or will smart pointer do it for you?
      //delete(rel_);
      std::cout << "HOORAY: We were able to strengthen bounds" << std::endl;
      rel_ = (RelaxationPtr) new Relaxation(env_);      
      rel_ = createNodeRelaxation(root, false, prune);      
    }
  }

#if defined(DEBUG_NODEFULLRELAXER)
    std::cout << "After relaxing (root) node.  Relaxation is: " << std::endl;
    rel_->write(std::cout);
#endif
  
  return rel_;
}


// Set and load the engine that is used to solve the relaxation.
void NodeFullRelaxer::setEngine(EnginePtr e)
{
  engine_ = e;
  engine_->load(rel_);
}


RelaxationPtr NodeFullRelaxer::createNodeRelaxation(NodePtr node, bool, 
                                                    bool &prune)
{

#if defined(DEBUG_NODEFULLRELAXER)
    std::cout << "Before relaxing node.  Relaxation is: " << std::endl;
    rel_->write(std::cout);
#endif

  prune = false;
  rel_ = (RelaxationPtr) new Relaxation(env_);
  for (HandlerIterator h = handlers_.begin(); h != handlers_.end() && !prune; 
      ++h) { 
    (*h)->relaxNodeFull(node, rel_, &prune);
  }

  if (!prune) {
    rel_->calculateSize();
    engine_->clear();
    engine_->load(rel_);

    bool changedNode = strongBoundsTighten_(node);
    if (changedNode) {
      // Delete relaxation: Not sure how.  Or will smart pointer do it for you?
      //delete(rel_);
      std::cout << "HOORAY: We were able to strengthen bounds" << std::endl;
      rel_ = (RelaxationPtr) new Relaxation(env_);  
      prune = false;
      for (HandlerIterator h = handlers_.begin(); h != handlers_.end() && !prune; 
           ++h) { 
        (*h)->relaxNodeFull(node, rel_, &prune);
      }

      rel_->calculateSize();
      engine_->clear();
      engine_->load(rel_);

    }    
  }

#if defined(DEBUG_NODEFULLRELAXER)
    std::cout << "After relaxing node.  Relaxation is: " << std::endl;
    rel_->write(std::cout);
#endif
  

  return rel_;

}


void NodeFullRelaxer::reset(NodePtr , bool )
{
}

RelaxationPtr NodeFullRelaxer::getRelaxation()
{
  return rel_;
}


void NodeFullRelaxer::setRelaxation(RelaxationPtr rel)
{
  rel_ = rel;
}

bool NodeFullRelaxer::isOriginalVariable_(ConstVariablePtr /* rv */,
                                          ConstVariablePtr &/* ov */)
{
  bool retval = false;

  for (HandlerIterator h = handlers_.begin(); h != handlers_.end(); ++h) {
    assert(!"Need to implement findOriginalVariable in Relaxation class");
    /*
    if ((*h)->findOriginalVariable(rv,ov)) {
      retval = true;
      break;
    }      
    */
  }

  return retval;
}


bool NodeFullRelaxer::strongBoundsTighten_(NodePtr node)
{
  bool retval = false;
  VariableConstIterator v_iter;
  VariablePtr rv, ov;
  double z = 0.0;
  VarBoundModPtr m;
  ObjectivePtr originalObj = rel_->getObjective();
  
  // Solve each problem
  for (v_iter = rel_->varsBegin(); v_iter != rel_->varsEnd(); ++v_iter) {
    rv = (*v_iter);
    bool isOriginal = isOriginalVariable_(rv, ov);

    if (isOriginal) {

#if defined(DEBUG_NODEFULLRELAXER)
      ov->write(std::cout);
#endif
      LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
      lf->addTerm(rv,1.0);

      FunctionPtr f = (FunctionPtr) new Function(lf);
      rel_->changeObj(f, 0.0);

#if defined(DEBUG_NODEFULLRELAXER)
      std::cout << "Bounds tightening.  Solving problem: " << std::endl;
      rel_->write(std::cout);
#endif

      EngineStatus s = engine_->solve();
      switch(s) {
      case (ProvenOptimal):
        z = engine_->getSolutionValue();
#if defined(DEBUG_NODEFULLRELAXER)
        std::cout << "Min value: " << z << std::endl;
#endif

        if (z > ov->getLb() + updateBoundsTol_) {
          retval = true;
          m = (VarBoundModPtr) new VarBoundMod(ov,Lower,z);
          node->addPMod(m);
        }
        break;
      default:
        std::cout << "strongBoundsTighten: Unknown Status String: " 
                  << engine_->getStatusString() << std::endl;
        assert(0);
      }
            
      
      lf->incTerm(rv,-2.0);
      f = (FunctionPtr) new Function(lf);
      rel_->changeObj(f, 0.0);
      
      //Solve
      s = engine_->solve();
      switch(s) {
      case (ProvenOptimal):
        z = -engine_->getSolutionValue();
#if defined(DEBUG_NODEFULLRELAXER)
        std::cout << "Max value: " << z << std::endl;
#endif
        if (z < ov->getUb() - updateBoundsTol_) {
          retval = true;
          m = (VarBoundModPtr) new VarBoundMod(ov,Upper,z);
          node->addPMod(m);
        }
        break;
      default:
        std::cout << "strongBoundsTighten: Unknown Status String: " 
                  << engine_->getStatusString() << std::endl;
        assert(0);
      }
    }
  }
   
  rel_->changeObj(originalObj->getFunction(), originalObj->getConstant());

  return retval;
}



//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2014 The MINOTAUR Team.
//

/**
 * \file SimpleTransformer.cpp
 * \brief Define class for simple reformulations a problem to make it suitable
 * for handlers.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <cmath>
#include <iostream>

#include "MinotaurConfig.h"

#include "Environment.h"
#include "CGraph.h"
#include "CNode.h"
#include "Constraint.h"
#include "CxQuadHandler.h"
#include "CxUnivarHandler.h"
#include "Function.h"
#include "IntVarHandler.h"
#include "LinearFunction.h"
#include "LinearHandler.h"
#include "Logger.h"
#include "NonlinearFunction.h"
#include "Option.h"
#include "Objective.h"
#include "Problem.h"
#include "ProblemSize.h"
#include "QuadraticFunction.h"
#include "SimpleTransformer.h"
#include "Solution.h"
#include "Variable.h"
#include "YEqCGs.h"
#include "YEqLFs.h"
#include "YEqUCGs.h"
#include "YEqVars.h"

// #define SPEW 1

using namespace Minotaur;
const std::string SimpleTransformer::me_ = "SimpleTransformer: ";


SimpleTransformer::SimpleTransformer()
  : Transformer(),
    yBiVars_(0)
{
}


SimpleTransformer::SimpleTransformer(EnvPtr env)
  : Transformer(env),
    yBiVars_(0)
{
}


SimpleTransformer::~SimpleTransformer() 
{
}


void SimpleTransformer::absRef_(LinearFunctionPtr lfl, VariablePtr vl,
                                Double dl, VariablePtr &v, Double &d)
{
  if (lfl) {
    vl = newVar_(lfl, dl, newp_);
  } else if (vl && fabs(dl)>zTol_) {
    vl = newVar_(vl, dl, newp_);
  }
  if (vl) {
    CGraphPtr cg = (CGraphPtr) new CGraph();
    CNode *n1 = cg->newNode(vl);
    CNode *n2 = 0;

    n1 = cg->newNode(OpAbs, n1, n2);
    cg->setOut(n1);
    cg->finalize();
    v = newVar_(cg, newp_);
  } else {
    d = fabs(dl);
  }
}


void SimpleTransformer::bilRef_(LinearFunctionPtr lfl, VariablePtr vl,
                                Double dl, LinearFunctionPtr lfr,
                                VariablePtr vr, Double dr,
                                LinearFunctionPtr &lf, VariablePtr &v,
                                Double &d)
{
  if (lfl) {
    vl = newVar_(lfl, dl, newp_);
    if (vr) {
      vr = newVar_(vr, dr, newp_);
    } else if (lfr) {
      vr = newVar_(lfr, dr, newp_);
    } 
    if (vr) {
      lf.reset();
      d = 0;
      v = newBilVar_(vl, vr);
    } else {
      lf = lfl;
      lf->multiply(dr);
      d = dl*dr;
      v.reset();
    }
  } else if (vl) {
    vl = newVar_(vl, dl, newp_);
    if (lfr) {
      vr = newVar_(lfr, dr, newp_);
    } else if (vr) {
      vr = newVar_(vr, dr, newp_);
    } 
    if (vr) {
      v = newBilVar_(vl, vr);
      lf.reset();
      d = 0;
    } else {
      lf = (LinearFunctionPtr) new LinearFunction();
      lf->addTerm(vl, dr);
      v.reset();
      d = 0;
    }
  } else if (lfr) {
    lf = lfr;
    lf->multiply(dl);
    d = dl*dr;
    v.reset();
  } else if (vr) {
    lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(vr, dl);
    v.reset();
    d = 0;
  } else {
    lf.reset();
    v.reset();
    d = dl*dr;
  }
}


SolutionPtr SimpleTransformer::getSolOrig(ConstSolutionPtr, Int &err )
{
  err = 1;
  return SolutionPtr();
}


SolutionPtr SimpleTransformer::getSolTrans(ConstSolutionPtr, Int &err )
{
  err = 1;
  return SolutionPtr();
}


VariablePtr SimpleTransformer::newBilVar_(VariablePtr vl, VariablePtr vr)
{
  CGraphPtr cg = (CGraphPtr) new CGraph();
  CNode *n1 = cg->newNode(vl);
  CNode *n2 = cg->newNode(vr);
  VariablePtr ov = VariablePtr();
  LinearFunctionPtr lf;
  FunctionPtr f;
  ConstraintPtr cnew;

  n2 = cg->newNode(OpMult, n1, n2);
  cg->setOut(n2);
  cg->finalize();

  ov = yBiVars_->findY(cg);
  if (!ov) {
    ov = newp_->newVariable();
    lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(ov, -1.0);
    f = (FunctionPtr) new Function(lf, cg);
    cnew = newp_->newConstraint(f, 0.0, 0.0);
    qHandler_->addConstraint(cnew);
    yBiVars_->insert(ov, cg);
  }
  return ov;
}


void SimpleTransformer::powKRef_(LinearFunctionPtr lfl,
                                 VariablePtr vl, Double dl, Double k,
                                 LinearFunctionPtr &lf, VariablePtr &v,
                                 Double &d)
{
  CNode *n1, *n2;
  if (fabs(k-floor(k+0.5))>zTol_) {
   assert(!"fractional powers can not be handled yet!");
  } else if (k<-zTol_) {
   assert(!"negative powers can not be handled yet!");
  } else if (fabs(k/2 - floor(k/2+0.5))>zTol_) {
   logger_->ErrStream() << "odd powers can not be handled yet!" << std::endl;
  }

  if (lfl) {
    vl = newVar_(lfl, dl, newp_);
  }
  if (vl) {
    CGraphPtr cg = (CGraphPtr) new CGraph();
    n1 = cg->newNode(vl);
    n2 = cg->newNode(k);
    n2 = cg->newNode(OpPowK, n1, n2);
    cg->setOut(n2);
    cg->finalize();
    v.reset();
    lf.reset();
    v = newVar_(cg, newp_);
    d = 0;
  } else {
    d = pow(dl, k);
  }
}


// Returns one of the following four:
// #1 lf + d, 
// #2  v + d, or
// d may be zero, lf and v may simultaneously be NULL. 
void SimpleTransformer::recursRef_(const CNode *node, LinearFunctionPtr &lf,
                                   VariablePtr &v, Double &d)
{
  Double dl = 0;
  Double dr = 0;
  FunctionPtr f;
  LinearFunctionPtr lfl = LinearFunctionPtr();
  LinearFunctionPtr lfr = LinearFunctionPtr();
  VariablePtr vl = VariablePtr();
  VariablePtr vr = VariablePtr();
  CNode *n1 = 0;

  lf = LinearFunctionPtr(); // NULL
  v = VariablePtr();
  d = 0.0;

  switch (node->getOp()) {
  case (OpAbs):
  case (OpAcos):
  case (OpAcosh):
  case (OpAsin):
  case (OpAsinh):
  case (OpAtan):
  case (OpAtanh):
  case (OpCeil):
  case (OpCos):
  case (OpCosh):
  case (OpCPow):
    recursRef_(node->getL(), lfl, vl, dl);
    uniVarRef_(node, lfl, vl, dl, lf, v, d);
    break;
  case (OpDiv):
    assert(!"OpDiv not implemented!");
    break;
  case (OpExp):
  case (OpFloor):
  case (OpInt):
  case (OpIntDiv):
  case (OpLog):
  case (OpLog10):
    recursRef_(node->getL(), lfl, vl, dl);
    uniVarRef_(node, lfl, vl, dl, lf, v, d);
    break;
  case (OpMinus):
    recursRef_(node->getL(), lfl, vl, dl);
    recursRef_(node->getR(), lfr, vr, dr);
    d = dl - dr;
    if (!vr && !lfr) {
      v = vl;
      lf = lfl;
    } else if (!vl && !lfl) {
      if (lfr) {
        lf = lfr;
        lf->multiply(-1.0);
      } else if (vr) {
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(vr, -1.0);
      }
    } else {
      lf = (LinearFunctionPtr) new LinearFunction();
      if (lfl) {
        lf->add(lfl);
      } else if (vl) {
        lf->incTerm(vl, 1.0);
      }
      if (lfr) {
        lfr->multiply(-1.0);
        lf->add(lfr);
      } else if (vr) {
        lf->incTerm(vr, -1.0);
      }
      v.reset();
    }
    break;
  case (OpMult):
    recursRef_(node->getL(), lfl, vl, dl);
    recursRef_(node->getR(), lfr, vr, dr);
    bilRef_(lfl, vl, dl, lfr, vr, dr, lf, v, d);
  case (OpNone):
    break;
  case (OpNum):
    d = node->getVal();
    break;
  case (OpPlus):
    recursRef_(node->getL(), lfl, vl, dl);
    recursRef_(node->getR(), lfr, vr, dr);
    d = dl + dr;
    if (!vl && !lfl) {
      v = vr;
      lf = lfr;
    } else if (!vr && !lfr) {
      v = vl;
      lf = lfl;
    } else {
      lf = (LinearFunctionPtr) new LinearFunction();
      if (lfl) {
        lf->add(lfl);
      } else if (vl) {
        lf->incTerm(vl, 1.0);
      }
      if (lfr) {
        lf->add(lfr);
      } else if (vr) {
        lf->incTerm(vr, 1.0);
      }
      v.reset();
    }
    break;
  case (OpPow):
    assert(!"not implemented!");
    break;
  case (OpPowK):
    recursRef_(node->getL(), lfl, vl, dl);
    powKRef_(lfl, vl, dl, node->getR()->getVal(), lf, v, d);
    break;
  case (OpRound):
  case (OpSin):
  case (OpSinh):
  case (OpSqr):
    recursRef_(node->getL(), lfl, vl, dl);
    uniVarRef_(node, lfl, vl, dl, lf, v, d);
    break;
  case (OpSqrt):
    recursRef_(node->getL(), lfl, vl, dl);
    uniVarRef_(node, lfl, vl, dl, lf, v, d);
    break;
  case (OpSumList):
    d = 0;
    lf = (LinearFunctionPtr) new LinearFunction();
    for (CNode **it=node->getListL(); it!=node->getListR(); ++it) {
      n1 = *it;
      lfl.reset(); vl.reset(); dl = 0;
      recursRef_(n1, lfl, vl, dl);
      d += dl;
      if (lfl) {
        lf->add(lfl);
      } else if (vl) {
        lf->incTerm(vl, 1.0);
      }
    }
    break;
  case (OpTan):
  case (OpTanh):
    recursRef_(node->getL(), lfl, vl, dl);
    uniVarRef_(node, lfl, vl, dl, lf, v, d);
    break;
  case (OpUMinus):
    recursRef_(node->getL(), lfl, vl, dl);
    d = -1.0*dl;
    if (lfl) {
      lf = lfl;
      lf->multiply(-1.0);
    } else if (vl) {
      lf = (LinearFunctionPtr) new LinearFunction();
      lf->addTerm(vl, -1.0);
    }
    break;
  case (OpVar):
    v = newp_->getVariable(node->getV()->getId());
    break;
  default:
    assert(!"cannot evaluate!");
  }

  assert(!lf || !v);
  if (lf && lf->getNumTerms()==1 &&
      fabs(lf->termsBegin()->second-1.0)<zTol_) { // return v, not lf
    v = lf->termsBegin()->first;
    lf.reset();
  }
}


void SimpleTransformer::refNonlinCons_(ConstProblemPtr oldp)
{
  ConstraintPtr c, cnew;
  FunctionPtr f, f2;
  CGraphPtr cg;
  LinearFunctionPtr lf;
  Double d, lb, ub;
  VariablePtr v = VariablePtr();

  assert (oldp && newp_);

  for (ConstraintConstIterator it=oldp->consBegin(); it!=oldp->consEnd();
       ++it) {
    c = *it;
    f = c->getFunction();
    if (f->getType()!=Constant && f->getType()!=Linear) {
      cg = boost::dynamic_pointer_cast <CGraph> (f->getNonlinearFunction());
      assert(cg);
      lf.reset(); v.reset(); d = 0.0;
      recursRef_(cg->getOut(), lf, v, d);
      if (v) {
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(v, 1.0);
      } 
      if (lf) {
        lf->add(f->getLinearFunction());
        if (lf->getNumTerms()>1) {
          f = (FunctionPtr) new Function(lf);
          cnew = newp_->newConstraint(f, c->getLb()-d, c->getUb()-d);
          lHandler_->addConstraint(cnew);
        } else {
          v = lf->termsBegin()->first;
          d = lf->termsBegin()->second;
          if (d>0) {
            lb = c->getLb()/d;
            ub = c->getUb()/d;
          } else {
            lb = c->getUb()/d;
            ub = c->getLb()/d;
          }
          if (lb>v->getLb()) {
            newp_->changeBound(v, Lower, lb);
          }
          if (ub<v->getUb()) {
            newp_->changeBound(v, Upper, ub);
          }
        }
      } else {
        assert(!"empty constraint?");
      }
    }
  }
}


void SimpleTransformer::refNonlinObj_(ConstProblemPtr oldp) 
{
  ObjectivePtr obj;
  FunctionPtr f, f2;
  Double d = 0;
  VariablePtr v = VariablePtr();
  LinearFunctionPtr lf, lf2;
  CGraphPtr cg;

  assert(newp_);
  assert(oldp);

  obj = oldp->getObjective();
  if (!obj) {
    return;
  }
  
  f = obj->getFunction();
  if (f->getType()!=Constant && f->getType()!=Linear) {
    cg = boost::dynamic_pointer_cast <CGraph> (f->getNonlinearFunction());
    assert(cg);
    recursRef_(cg->getOut(), lf, v, d);
    if (lf) {
      f2 = (FunctionPtr) new Function(lf);
      obj = newp_->newObjective(f2, d, Minimize);
    } else if (v) {
      lf2 = (LinearFunctionPtr) new LinearFunction();
      lf2->addTerm(v, 1.0);
      f2 = (FunctionPtr) new Function(lf);
      obj = newp_->newObjective(f2, d, Minimize);
    } else {
      f2.reset();
      obj = newp_->newObjective(f2, d, Minimize);
      logger_->MsgStream(LogInfo)
        << "Problem objective reduced to a constant" << std::endl;
    }
  }
}


void SimpleTransformer::reformulate(ConstProblemPtr oldp, ProblemPtr &newp, 
                                    HandlerVector &handlers, Int &status)
{
  assert(oldp);

  newp = (ProblemPtr) new Problem();
  newp_ = newp;
  yLfs_ = new YEqLFs(2*oldp->getNumVars());
  yUniExprs_ = new YEqUCGs();
  yBiVars_ = new YEqCGs();
  copyVars_(oldp, newp);

  // create handlers.
  if (oldp->getSize()->bins > 0 || oldp->getSize()->ints > 0) {
    IntVarHandlerPtr ihandler = (IntVarHandlerPtr)
                                new IntVarHandler(env_, newp);
    handlers.push_back(ihandler);
  }
  lHandler_ = (LinearHandlerPtr) new LinearHandler(env_, newp);
  handlers.push_back(lHandler_);
  qHandler_ = (CxQuadHandlerPtr) new CxQuadHandler(env_, newp);
  handlers.push_back(qHandler_);
  uHandler_ = (CxUnivarHandlerPtr) new CxUnivarHandler(env_, newp);
  handlers.push_back(uHandler_);

  copyLinear_(oldp, newp);
  refNonlinCons_(oldp);
  refNonlinObj_(oldp);

  if (!(allConsAssigned_(newp, handlers))) {
    status = 1;
    return;
  }
  clearUnusedHandlers_(handlers);
  status = 0;
  newp->write(std::cout);
}


void SimpleTransformer::trigRef_(OpCode op, LinearFunctionPtr lfl,
                                 VariablePtr vl, Double dl, VariablePtr &v,
                                 Double &d)
{
  if (lfl) {
    vl = newVar_(lfl, dl, newp_);
  } else if (vl && fabs(dl)>zTol_) {
    vl = newVar_(vl, dl, newp_);
  }
  if (vl) {
    CGraphPtr cg = (CGraphPtr) new CGraph();
    CNode *n1 = cg->newNode(vl);
    CNode *n2 = 0;

    n1 = cg->newNode(op, n1, n2);
    cg->setOut(n1);
    cg->finalize();
    v = newVar_(cg, newp_);
  } else {
    d = fabs(dl);
  }
}


void SimpleTransformer::uniVarRef_(const CNode *n0, LinearFunctionPtr lfl,
                                   VariablePtr vl, Double dl, 
                                   LinearFunctionPtr &lf, VariablePtr &v,
                                   Double &d)
{
  CNode *n1, *n2;
  Int err = 0;
  if (lfl) {
    vl = newVar_(lfl, dl, newp_);
  }
  if (vl) {
    CGraphPtr cg = (CGraphPtr) new CGraph();
    n1 = cg->newNode(vl);
    n2 = 0;
    n2 = cg->newNode(n0->getOp(), n1, n2);
    cg->setOut(n2);
    cg->finalize();
    v.reset();
    lf.reset();
    v = newVar_(cg, newp_);
    d = 0;
  } else {
    d = n0->eval(dl, &err);
    assert(0==err);
  }
}


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

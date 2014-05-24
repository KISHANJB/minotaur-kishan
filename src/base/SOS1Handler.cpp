//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2014 The MINOTAUR Team.
//

/**
 * \file SOS1Handler.cpp
 * \brief Declare the SOS1Handler class for handling SOS type I constraints.
 * It checks integrality and provides branching candidates. Does
 * not do any presolving and cut-generation.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include <cmath>
#include <iomanip>

#include "MinotaurConfig.h"
#include "BrCand.h"
#include "Branch.h"
#include "Constraint.h"
#include "Environment.h"
#include "Function.h"
#include "Logger.h"
#include "LinearFunction.h"
#include "LinMods.h"
#include "Operations.h"
#include "Option.h"
#include "ProblemSize.h"
#include "Relaxation.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "SOS.h"
#include "SOS1Handler.h"
#include "SOSBrCand.h"
#include "Variable.h"

//#define SPEW 1

using namespace Minotaur;
const std::string SOS1Handler::me_ = "SOS1Handler: ";

SOS1Handler::SOS1Handler(EnvPtr env, ProblemPtr problem)
  : env_(env)
{
  logger_ = (LoggerPtr) new Logger((LogLevel) env_->getOptions()->
                                   findInt("handler_log_level")->getValue());
  zTol_   = 1e-6;

  if (false == env_->getOptions()->findBool("modify_rel_only")->getValue()
      &&  (0 < problem_->getNumSOS1() + problem_->getNumSOS2())) {
    logger_->ErrStream() << "Can not handle SOS constraints if "
                         << "modify_rel_only flag is false" 
                         << std::endl;
    assert(!"error initializing sos constraint");
  }
  problem_  = problem;
}


SOS1Handler::~SOS1Handler()
{
  problem_.reset();
  env_.reset();
  logger_.reset();
}


Bool SOS1Handler::isFeasible(ConstSolutionPtr sol, RelaxationPtr rel, Bool &)
{
  SOSPtr sos;
  Bool isfeas = true;
  int nz;
  const Double* x = sol->getPrimal();

  for (SOSConstIterator siter=rel->sos1Begin(); siter!=rel->sos1End();
       ++siter) {
    sos = *siter;
    nz = 0;
    for (VariableConstIterator viter = sos->varsBegin(); viter!=sos->varsEnd();
         ++viter) {
      if (x[(*viter)->getIndex()]>zTol_) {
        ++nz;
        if (nz>1) {
          isfeas = false;
          break;
        }
      }
    }
    if (false == isfeas) {
      break;
    }
  }

#if SPEW
  logger_->MsgStream(LogDebug) << me_ << "isFeasible = " << isfeas
                               << std::endl;
#endif
  return isfeas;
}


Branches SOS1Handler::getBranches(BrCandPtr cand, DoubleVector &, 
                                  RelaxationPtr, SolutionPoolPtr)
{
  SOSBrCandPtr scand = boost::dynamic_pointer_cast <SOSBrCand> (cand);
  LinModsPtr mod;
  VarBoundModPtr bmod;

  BranchPtr branch1, branch2;
  Branches branches = (Branches) new BranchPtrVector();

  mod = (LinModsPtr) new LinMods();
  for (VariableConstIterator vit=scand->lVarsBegin();vit!=scand->lVarsEnd();
       ++vit) {
    bmod = (VarBoundModPtr) new VarBoundMod(*vit, Upper, 0.0);
    mod->insert(bmod);
  }
  branch1 = (BranchPtr) new Branch();
  branch1->addMod(mod);
  branch1->setActivity(scand->getLSum());

  mod = (LinModsPtr) new LinMods();
  for (VariableConstIterator vit=scand->rVarsBegin();vit!=scand->rVarsEnd();
       ++vit) {
    bmod = (VarBoundModPtr) new VarBoundMod(*vit, Upper, 0.0);
    mod->insert(bmod);
  }
  branch2 = (BranchPtr) new Branch();
  branch2->addMod(mod);
  branch2->setActivity(scand->getRSum());

  branches->push_back(branch1);
  branches->push_back(branch2);
  return branches;
}


void SOS1Handler::getBranchingCandidates(RelaxationPtr rel, 
                                         const std::vector< Double > &x,
                                         ModVector &, BrCandSet & cands,
                                         Bool & is_inf)
{
  SOSConstIterator siter;
  VariableConstIterator viter;
  VariablePtr var;
  SOSPtr sos;
  Double parsum;
  VarVector lvars, rvars;
  int nz;
  Double nzsum;
  Double nzval;
  SOSBrCandPtr br_can;
  bool is_new;

  for (siter=rel->sos1Begin(); siter!=rel->sos1End(); ++siter) {
    sos = *siter;
    getNzNumSum_(sos, x, &nz, &nzsum);
    if (nz>1) {
      parsum = 0.0;
      lvars.clear();
      rvars.clear();
      viter = sos->varsBegin();
      for (; viter!=sos->varsEnd(); ++viter) {
        var = *viter;
        if (var->getUb()-var->getLb()>zTol_) {
          nzval = x[var->getIndex()];
          if ((parsum + nzval) < 0.5*nzsum) {
            lvars.push_back(var);
            parsum += nzval;
          } else if (nzval+parsum - nzsum/2 < nzsum/2 - parsum) {
            lvars.push_back(var);
            parsum += nzval;
            ++viter;
            break;
          } else {
            break;
          }
        }
      } 
      for (; viter!=sos->varsEnd(); ++viter) {
        var = *viter;
        if (var->getUb()-var->getLb()>zTol_) {
          rvars.push_back(*viter);
        }
      }
      br_can = (SOSBrCandPtr) new SOSBrCand(sos, lvars, rvars, parsum,
                                            nzsum-parsum);
      br_can->setDir(DownBranch);
      br_can->setScore(20.0*(lvars.size()-1)*(rvars.size()-1));
      
      is_new = cands.insert(br_can).second;
      assert(true==is_new);

#if SPEW
      logger_->MsgStream(LogDebug) << me_ << sos->getName() << " is a "
        << " branching candidate." << std::endl
        << me_ << "left branch has variables ";
      for (viter = lvars.begin(); viter!=lvars.end(); ++viter) {
        logger_->MsgStream(LogDebug) << (*viter)->getName() << " ";
      }
      logger_->MsgStream(LogDebug) << std::endl
                                   << me_ << "left sum = " << parsum
                                   << std::endl
                                   << me_ << "right branch has variables ";
      for (viter = rvars.begin(); viter!=rvars.end(); ++viter) {
        logger_->MsgStream(LogDebug) << (*viter)->getName() << " ";
      }
      logger_->MsgStream(LogDebug) << std::endl
                                   << me_ << "right sum = " << nzsum - parsum
                                   << std::endl;
#endif

    } else {
#if SPEW
      logger_->MsgStream(LogDebug) << me_ << sos->getName() << " is not a "
                                   << " branching candidate." << std::endl;
#endif
    }
  }
  is_inf = false;
}


ModificationPtr SOS1Handler::getBrMod(BrCandPtr cand, DoubleVector &,
                                      RelaxationPtr , BranchDirection dir) 
{
  LinModsPtr mod = (LinModsPtr) new LinMods();
  SOSBrCandPtr scand = boost::dynamic_pointer_cast <SOSBrCand> (cand);
  VarBoundModPtr bmod;

  if (dir==DownBranch) {
    for (VariableConstIterator vit = scand->lVarsBegin();
         vit!=scand->lVarsEnd(); ++vit) {
      bmod = (VarBoundModPtr) new VarBoundMod((*vit), Upper, 0.0);
      mod->insert(bmod);
    }
  } else {
    for (VariableConstIterator vit = scand->rVarsBegin();
         vit!=scand->rVarsEnd(); ++vit) {
      bmod = (VarBoundModPtr) new VarBoundMod((*vit), Upper, 0.0);
      mod->insert(bmod);
    }
  }
  return mod;
}


std::string SOS1Handler::getName() const
{
  return "SOS1Handler (Handling SOS1 constraints).";
}


void SOS1Handler::getNzNumSum_(SOSPtr sos, const DoubleVector x, int *nz,
                               double *nzsum)
{
  Double xval;
  *nz = 0;
  *nzsum = 0.0;
  for (VariableConstIterator viter = sos->varsBegin(); viter!=sos->varsEnd();
       ++viter) {
    xval = x[(*viter)->getIndex()];
    if (xval>zTol_) {
      *nzsum += xval;
      ++(*nz);
    }
  }
}


Double SOS1Handler::getTol() const
{
  return zTol_;
}


Bool SOS1Handler::isGUB(SOS *sos)
{
  VariablePtr var;
  Int nz = sos->getNz();
  ConstConstraintPtr c;
  FunctionPtr f;
  LinearFunctionPtr lf;
  Bool isgub = false;


  for (VariableConstIterator viter = sos->varsBegin(); viter!=sos->varsEnd();
       ++viter) {
    var = *viter;
    if (Binary!=var->getType() || ImplBin!=var->getType()) {
      return false;
    }
  }

  // If we are here, then it means all variables are binary. Check if their sum
  // is <= 1.
  var = *(sos->varsBegin());
  for (ConstrSet::iterator it=var->consBegin();
       it!=var->consEnd() && false==isgub; ++it) {
    c = *it;
    f = c->getFunction();
    isgub = true;
    if (Linear!=f->getType()) {
      isgub = false;
    } else if ((UInt) nz==f->getNumVars()) {
      isgub = false;
    } else if (fabs(c->getUb()-1.0)>zTol_) {
      isgub = false;
    } else {
      lf = f->getLinearFunction();
      for (VariableConstIterator viter = sos->varsBegin();
           viter!=sos->varsEnd(); ++viter) {
        if ((lf->getWeight(*viter)-1.0)>zTol_) {
          isgub = false;
          break;
        }
      }
    }
  }

  return isgub;
}


Bool SOS1Handler::isNeeded()
{
  Bool is_needed = false;
  if (problem_ &&
      false == env_->getOptions()->findBool("ignore_SOS1")->getValue()) {
    problem_->calculateSize();
    if (problem_->getSize()->SOS1Cons > 0) {
      for (SOSConstIterator siter=problem_->sos1Begin();
           siter!=problem_->sos1End(); ++siter) {
        if (false==isGUB(*siter)) {
          is_needed = true;
          break;
        }
      }
    }
  }
  return is_needed;
}


void SOS1Handler::relaxInitFull(RelaxationPtr, Bool *is_inf)
{
  *is_inf = false;
}


void SOS1Handler::relaxInitInc(RelaxationPtr , Bool *is_inf)
{
  *is_inf = false;
}


void SOS1Handler::relaxNodeFull(NodePtr , RelaxationPtr, Bool *is_inf)
{
  *is_inf = false;
}


void SOS1Handler::relaxNodeInc(NodePtr , RelaxationPtr , Bool *is_inf)
{
  *is_inf = false;
}


void SOS1Handler::separate(ConstSolutionPtr, NodePtr , RelaxationPtr,
                           CutManager *, SolutionPoolPtr, Bool *,
                           SeparationStatus *status)
{
  *status = SepaNone;
}


void SOS1Handler::setTol(Double tol)
{
  zTol_ = tol;
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

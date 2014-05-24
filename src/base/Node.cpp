// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2014 The MINOTAUR Team.
// 

/**
 * \file Node.cpp
 * \brief Define class Node for storing information about nodes.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <cmath>
#include <iostream>

#include "MinotaurConfig.h"
#include "Branch.h"
#include "Modification.h"
#include "Node.h"
#include "Relaxation.h"

using namespace Minotaur;
using namespace std;

Node::Node()
  : branch_(BranchPtr()),
    depth_(0),
    id_(0),
    lb_(-INFINITY),
    mods_(0), 
    parent_(boost::shared_ptr<Node>()),
    status_(NodeNotProcessed),
    tbScore_(0)
{
}


Node::Node(NodePtr parentNode, BranchPtr branch)
  : branch_(branch),
    depth_(0),
    id_(0),
    mods_(0), 
    parent_(parentNode),
    status_(NodeNotProcessed),
    tbScore_(0)
{
  lb_ = parentNode->getLb();
}


Node::~Node()
{
  mods_.clear();
  children_.clear();
}


void Node::addChild(NodePtr childNode)
{
  children_.push_back(childNode);
}


void Node::applyMods(ProblemPtr p)
{
  ModificationConstIterator mod_iter;
  ModificationPtr mod;
  // first apply the mods that created this node from its parent
  if (branch_) {
    for (mod_iter=branch_->modsBegin(); mod_iter!=branch_->modsEnd(); 
        ++mod_iter) {
      mod = *mod_iter;
      mod->applyToProblem(p);
    }
  }
  // now apply any other mods that were added while processing it.
  for (mod_iter=mods_.begin(); mod_iter!=mods_.end(); ++mod_iter) {
    mod = *mod_iter;
    mod->applyToProblem(p);
  }
}


void Node::applyMods(RelaxationPtr rel, ProblemPtr p)
{
  ModificationConstIterator mod_iter;
  ModificationPtr mod;
  ModificationPtr mod2;
  
  if (branch_) {
    for (mod_iter=branch_->modsBegin(); mod_iter!=branch_->modsEnd(); 
        ++mod_iter) {
      mod = *mod_iter;
      mod2 = mod->toRel(p, rel);
      mod->applyToProblem(p);
      mod2->applyToProblem(rel);
    }
  }
 
  for (mod_iter=mods_.begin(); mod_iter!=mods_.end(); ++mod_iter) {
    mod = *mod_iter;
    mod2 = mod->toRel(p, rel);
    mod->applyToProblem(p);
    mod2->applyToProblem(rel);
  }
}


void Node::removeChild(NodePtrIterator childNodeIter)
{
  children_.erase(childNodeIter);
}


void Node::removeChildren()
{
  children_.clear();
}


void Node::removeParent()
{
  parent_.reset();
}


void Node::setDepth(UInt depth)
{
  depth_ = depth;
}


void Node::setId(UInt id)
{
  id_ = id;
}


void Node::setLb(Double value)
{
  lb_ = value;
}


void Node::setWarmStart (WarmStartPtr ws) 
{ 
  ws_ = ws; 
}


void Node::undoMods(ProblemPtr p)
{
  ModificationRConstIterator mod_iter;
  ModificationPtr mod;

  // explicitely request the const_reverse_iterator for rend():
  // for bug in STL C++ standard
  // http://stackoverflow.com/questions/2135094/gcc-reverse-iterator-comparison-operators-missing
  ModificationRConstIterator rend = mods_.rend();  
  // first undo the mods that were added while processing the node.
  for (mod_iter=mods_.rbegin(); mod_iter!= rend; ++mod_iter) {
    mod = *mod_iter;
    mod->undoToProblem(p);
  } 

  // now undo the mods that were used to create this node from its parent.
  if (branch_) {
    for (mod_iter=branch_->modsRBegin(); mod_iter!=branch_->modsREnd(); 
        ++mod_iter) {
      mod = *mod_iter;
      mod->undoToProblem(p);
    }
  }
}


void Node::undoMods(RelaxationPtr rel, ProblemPtr p)
{
  ModificationRConstIterator mod_iter;
  ModificationPtr mod;
  ModificationPtr mod2;
  ModificationRConstIterator rend = mods_.rend();  

  
  for (mod_iter=mods_.rbegin(); mod_iter!= rend; ++mod_iter) {
    mod = *mod_iter;
    mod2 = mod->toRel(p, rel);
    mod->undoToProblem(p);
    mod2->undoToProblem(rel);
  } 

 
  if (branch_) {
    for (mod_iter=branch_->modsRBegin(); mod_iter!=branch_->modsREnd(); 
        ++mod_iter) {
      mod = *mod_iter;
      mod2 = mod->toRel(p, rel);
      mod->undoToProblem(p);
      mod2->undoToProblem(rel);
    }
  }
}


void Node::write(std::ostream &out) const
{
  out << "Node ID: " << id_ << " at depth: " << depth_;
  if (parent_) { 
    out << " has parent ID: " << parent_->getId()
        << " lb = " << lb_ << " tbScore_ = " << tbScore_ << std::endl;
  }
  out << std::endl;
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

// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2017 The MINOTAUR Team.
// 

/**
 * \file Variable.h
 * \brief Define the Variable class
 * \author Ashutosh Mahajan, Argonne National Laboratory
 * 
 * Methods for handling variables in a problem.
 */


#ifndef MINOTAURVARIABLE_H
#define MINOTAURVARIABLE_H

#include <string>

#include "Types.h"

namespace Minotaur {

/**
 * Variable class contains methods for defining and changing variables of a
 * Problem. A variable has bounds (lb, ub), a type (binary, integer,
 * continuous), a name, and an id which is unique for the variable and is
 * usually generated by the Problem.
 */
class Variable {
public:
  /// Only Problem class can modify a Variable. All modification methods
  /// are private.
  friend class Problem;
  friend class PerspCon;

  /// Default constructor.
  Variable();

  /// Construct a variable with an id, lb, ub, type and name.
  Variable(UInt id, UInt index, double lb, double ub, VariableType vtype, 
           std::string name);

  /// Destroy.
  virtual ~Variable();

  /// Create a copy.
  boost::shared_ptr<Variable> clone(UInt id) const;

  /// Get the unique id.
  UInt getId() const { return id_; }

  /**
   * Many operations rely on arrays of values. index_ is the index of
   * variable in such arrays. Thus x[index_] will give the
   * value of this variable in an array x, provided x was populated that
   * way (Most engines etc. will populate it on the basis of index_).
   */
  UInt getIndex() const { return index_; }

  /// Get lowerbound.
  double getLb() const { return lb_; }

  /// Get upperbound.
  double getUb() const { return ub_; }

  /// Get the type (binary, integer, continuous).
  VariableType getType() const { return vtype_; }

  /// Get the function type of constraints that this variable is in.
  FunctionType getFunType() const { return ftype_; }

  /// Get the type of origin of this variable
  VarSrcType getSrcType() const { return stype_; }

  /// Get the state (fixed, deleted, ...).
  VarState getState() const { return state_; }

  UInt getNumCons() const;

  /// First iterator of constraints where this variable appears.
  ConstrSet::iterator consBegin();

  /// Last iterator of constraints where this variable appears.
  ConstrSet::iterator consEnd();

  /// Get name of the variable.
  const std::string getName() const;

  /**
   * \brief Write the full description of the variable including name, bounds,
   * type etc.
   */
  void write(std::ostream &out) const;

  /// Write the list of constraints in which this variable occurs.
  void writeConstraintMap(std::ostream & out) const;

protected:
  /// Change the id to a new value.
  void setId_(UInt n) { id_ = n; }

  /// Change the state to a new value.
  void setState_(VarState state) { state_ = state; return; }

  /// Change the lowerbound to a new value.
  void setLb_(double newLb) { lb_ = newLb; }

  /// Change the upperbound to a new value.
  void setUb_(double newUb) { ub_ = newUb; }

  /// Change the index to a new value.
  void setIndex_(UInt n) { index_ = n; }

  /// Change the name to a new value.
  void setName_(std::string newName) { name_ = newName; }

  /// Change the type to a new value.
  void setType_(VariableType vtype) { vtype_ = vtype; return; }

  /// Change the function type to a new value.
  void setFunType_(FunctionType ftype) { ftype_ = ftype; return; }

  /// Change the type of the origin of this variable
  void setSrcType(VarSrcType stype) { stype_ = stype; }

  /**
   * \brief Remove the constraint from the list of constraints this variable
   * belongs to.
   */
  void outOfConstraint_(ConstraintPtr cPtr);

  /**
   * \brief Add the constraint to the list of constraints this variable
   * belongs to.
   */
  void inConstraint_(ConstraintPtr c);

  /// Clear the list of constraints this variable belongs to.
  void clearConstraints_();

private:
  /// unique id for this variable
  UInt id_;

  /// index for this variable
  UInt index_;

  /// lower bound
  double lb_;

  /// upper bound
  double ub_;

  /// type: integer, binary, continuous etc
  VariableType vtype_;

  /// What kind of constraints does it appear in? Linear, Quadratic, ...
  FunctionType ftype_;

  /// state: fixed, freed etc
  VarState state_;

  /// Where did this variable originate? Original problem, transformer, etc.
  VarSrcType stype_;

  /// name
  std::string name_;

  /// Set of constraints in which this variable appears.
  ConstrSet cons_;

  /// Copy constructor is not allowed.
  Variable(const Variable &v);

  /// Copy by assignment is no allowed.
  Variable & operator = (const Variable &v);
};

typedef std::set<std::pair<ConstraintPtr, FunctionType> >::const_iterator 
ConstraintFunIterator;
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

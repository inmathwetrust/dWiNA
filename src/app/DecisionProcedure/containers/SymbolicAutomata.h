/*****************************************************************************
 *  gaston - We pay homage to Gaston, an Africa-born brown fur seal who
 *    escaped the Prague Zoo during the floods in 2002 and made a heroic
 *    journey for freedom of over 300km all the way to Dresden. There he
 *    was caught and subsequently died due to exhaustion and infection.
 *    Rest In Piece, brave soldier.
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *      Notable mentions: Ondrej Lengal <ondra.lengal@gmail.com>
 *
 *  Description:
 *      Symbolic Automata representing the formulae. The computation is
 *      done on this representation according to the latest paper.
 *****************************************************************************/


#ifndef WSKS_SYMBOLICAUTOMATA_H
#define WSKS_SYMBOLICAUTOMATA_H

#include "../mtbdd/apply1func.hh"
#include "../mtbdd/apply2func.hh"
#include "../mtbdd/void_apply1func.hh"
#include "../mtbdd/ondriks_mtbdd.hh"
#include "../utils/Symbol.h"
#include "../environment.hh"
#include "../containers/SymbolicCache.hh"
#include "../../Frontend/ident.h"
#include "../../Frontend/ast.h"
#include "../containers/VarToTrackMap.hh"
#include <vector>
#include <vata/util/binary_relation.hh>

// <<< FORWARD CLASS DECLARATIONS >>>
class Term;
extern VarToTrackMap varMap;

using namespace Gaston;

/**
 * Base class for Symbolic Automata. Each symbolic automaton contains the
 * pointer to the formula it corresponds to (for further informations),
 * its initial and final states, free variables and caches.
 */
class SymbolicAutomaton {
public:
    // <<< PUBLIC MEMBERS >>>
    static StateType stateCnt;
    AutType type;

protected:
    // <<< PRIVATE MEMBERS >>>
    Formula_ptr _form;
    Term_ptr _initialStates;
    Term_ptr _finalStates;
    ResultCache _resCache;          // Caches (states, symbol) = (fixpoint, bool)
    SubsumptionCache _subCache;     // Caches (term, term) = bool
    VarList _freeVars;

    // <<< PRIVATE FUNCTIONS >>>
    virtual void _InitializeAutomaton() = 0;
    virtual void _InitializeInitialStates() = 0;
    virtual void _InitializeFinalStates() = 0;
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term_ptr&, bool) = 0;

    // <<< MEASURES >>>
    unsigned int _falseCounter = 0;
    unsigned int _trueCounter = 0;
    unsigned int _contUnfoldingCounter = 0;
    unsigned int _contCreationCounter = 0;
    unsigned int _projectIterationCounter = 0;
    unsigned int _projectSymbolEvaluationCounter = 0;

public:
    // <<< CONSTRUCTORS >>>
    SymbolicAutomaton(Formula_ptr form);

    // <<< PUBLIC API >>>
    virtual Term_ptr GetInitialStates();
    virtual Term_ptr GetFinalStates();
    virtual Term_ptr Pre(Symbol*, Term_ptr, bool) = 0;
    ResultType IntersectNonEmpty(Symbol*, Term_ptr&, bool);

    // <<< DUMPING FUNCTIONS >>>
    virtual void DumpAutomaton() = 0;
    virtual void DumpCacheStats() = 0;
    virtual void DumpStats() = 0;
};

/**
 * BinaryOpAutomaton corresponds to Binary Operations of Intersection and
 * Union of subautomata. It further contains the links to left and right
 * operands and some additional functions for evaluation of results
 */
class BinaryOpAutomaton : public SymbolicAutomaton {
protected:
    // <<< PRIVATE MEMBERS >>>
    SymbolicAutomaton_ptr _lhs_aut;
    SymbolicAutomaton_ptr _rhs_aut;
    ProductType _productType;
    bool (*_eval_result)(bool, bool, bool);     // Boolean function for evaluation of left and right results
    bool (*_eval_early)(bool, bool);            // Boolean function for evaluating early evaluation
    bool (*_early_val)(bool);                   // Boolean value of early result

    // <<< PRIVATE FUNCTIONS >>>
    virtual void _InitializeAutomaton();
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term_ptr&, bool);

public:
    BinaryOpAutomaton(SymbolicAutomaton_raw lhs, SymbolicAutomaton_raw rhs, Formula_ptr form);

    // <<< PUBLIC API >>>
    virtual Term_ptr Pre(Symbol*, Term_ptr, bool);

    // <<< DUMPING FUNCTIONS >>>
    virtual void DumpAutomaton();
    virtual void DumpStats();
    virtual void DumpCacheStats();
};

/**
 * Automaton corresponding to the formula: phi and psi
 */
class IntersectionAutomaton : public BinaryOpAutomaton {
public:
    IntersectionAutomaton(SymbolicAutomaton* lhs, SymbolicAutomaton* rhs, Formula_ptr form);
};

/**
 * Automaton corresponding to the formula: phi or psi
 */
class UnionAutomaton : public BinaryOpAutomaton {
public:
    UnionAutomaton(SymbolicAutomaton* lhs, SymbolicAutomaton* rhs, Formula_ptr form);
};

/**
 * Automaton corresponding to the formulae: not phi
 */
class ComplementAutomaton : public SymbolicAutomaton {
protected:
    // <<< PRIVATE MEMBERS >>>
    std::shared_ptr<SymbolicAutomaton> _aut;

    // <<< PRIVATE FUNCTIONS >>>
    virtual void _InitializeAutomaton();
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term_ptr&, bool);

public:
    // <<< CONSTRUCTORS >>>
    ComplementAutomaton(SymbolicAutomaton *aut, Formula_ptr form);

    // <<< PUBLIC API >>>
    virtual Term_ptr Pre(Symbol*, Term_ptr, bool);

    // <<< DUMPING FUNCTIONS >>>
    virtual void DumpAutomaton();
    virtual void DumpStats();
    virtual void DumpCacheStats();
};

/**
 * Automaotn corresponding to the formulae: Exists X. phi
 */
class ProjectionAutomaton : public SymbolicAutomaton {
protected:
    // <<< PRIVATE MEMBERS >>>
    std::shared_ptr<SymbolicAutomaton> _aut;
    IdentList* _projected_vars;

    // <<< PRIVATE FUNCTIONS >>>
    virtual void _InitializeAutomaton();
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term_ptr&, bool);

public:
    /// <<< CONSTRUCTORS >>>
    ProjectionAutomaton(SymbolicAutomaton* aut, Formula_ptr form);

    // <<< PUBLIC API >>>
    virtual Term_ptr Pre(Symbol*, Term_ptr, bool);

    // <<< DUMPING FUNCTIONS >>>
    virtual void DumpAutomaton();
    virtual void DumpStats();
    virtual void DumpCacheStats();
};

/**
 * Symbolic automaton corresponding to base automata
 */
class BaseAutomaton : public SymbolicAutomaton {
protected:
    /// <<< PRIVATE MEMBERS >>>
    std::shared_ptr<BaseAutomatonType> _base_automaton;
    unsigned int _stateSpace;               // Number of states in automaton
    unsigned int _stateOffset;              // Offset of states used for mask

    /// <<< PRIVATE FUNCTIONS >>>
    virtual void _InitializeAutomaton();
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term_ptr&, bool);
    void _RenameStates();

public:
    // <<< CONSTRUCTORS >>>
    BaseAutomaton(BaseAutomatonType* aut, Formula_ptr form) : SymbolicAutomaton(form), _base_automaton(aut){ this->_InitializeAutomaton();type = AutType::BASE; }

    // <<< PUBLIC API >>>
    virtual Term_ptr Pre(Symbol*, Term_ptr, bool);

    // <<< DUMPING FUNCTIONS >>>
    virtual void BaseAutDump();
    virtual void DumpStats();
    virtual void DumpCacheStats();
};

class GenericBaseAutomaton : public BaseAutomaton {
public:
    GenericBaseAutomaton(BaseAutomatonType* aut, Formula_ptr form) : BaseAutomaton(aut, form) { }
    virtual void DumpAutomaton();
};

class SubAutomaton : public BaseAutomaton {
public:
    SubAutomaton(BaseAutomatonType* aut, Formula_ptr form) : BaseAutomaton(aut, form) { }
    virtual void DumpAutomaton();
};

class TrueAutomaton : public BaseAutomaton {
public:
    TrueAutomaton(BaseAutomatonType* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void DumpAutomaton();
};

class FalseAutomaton : public BaseAutomaton {
public:
    FalseAutomaton(BaseAutomatonType* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void DumpAutomaton();
};

class InAutomaton : public BaseAutomaton {
public:
    InAutomaton(BaseAutomatonType* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void DumpAutomaton();
};

class FirstOrderAutomaton : public BaseAutomaton {
public:
    FirstOrderAutomaton(BaseAutomatonType* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void DumpAutomaton();
};

class EqualFirstAutomaton : public BaseAutomaton {
public:
    EqualFirstAutomaton(BaseAutomatonType* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void DumpAutomaton();
};

class EqualSecondAutomaton : public BaseAutomaton {
public:
    EqualSecondAutomaton(BaseAutomatonType* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void DumpAutomaton();
};

class LessAutomaton : public BaseAutomaton {
public:
    LessAutomaton(BaseAutomatonType* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void DumpAutomaton();
};

class LessEqAutomaton : public BaseAutomaton {
public:
    LessEqAutomaton(BaseAutomatonType* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void DumpAutomaton();
};

#endif //WSKS_SYMBOLICAUTOMATA_H

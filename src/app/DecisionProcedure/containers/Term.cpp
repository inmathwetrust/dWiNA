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
 *      Representation of Terms that are computed during the decision procedure
 *****************************************************************************/

#include "Term.h"
#include <boost/functional/hash.hpp>

namespace Gaston {
    size_t hash_value(Term* s) {
        return boost::hash_value(s->MeasureStateSpace());
    }
}

// <<< STATIC MEMBER INITIALIZATION >>>
int TermProduct::instances = 0;
int TermBaseSet::instances = 0;
int TermList::instances = 0;
int TermFixpoint::instances = 0;
int TermContinuation::instances = 0;

// <<< TERM CONSTRUCTORS >>>
TermEmpty::TermEmpty() {
    this->_inComplement = false;
    this->type = TERM_EMPTY;
}

/**
 * Constructor of Term Product---construct intersecting product of
 * @p lhs and @p rhs
 *
 * @param[in] lhs:  left operand of term intersection
 * @param[in] rhs:  right operand of term intersection
 */
TermProduct::TermProduct(Term_ptr lhs, Term_ptr rhs) : left(lhs), right(rhs) {
    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermProduct::";
    this->dump();
    std::cout << "\n";
    #endif
    #if (MEASURE_STATE_SPACE == true)
    ++TermProduct::instances;
    #endif
    type = TermType::TERM_PRODUCT;
    subtype = ProductType::E_INTERSECTION;
    this->_inComplement = false;
}

/**
 * Constructor of Term Product---other type
 */
TermProduct::TermProduct(Term_ptr lhs, Term_ptr rhs, ProductType pt) : left(lhs), right(rhs) {
    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermProduct::";
    this->dump();
    std::cout << "\n";
    #endif
    #if (MEASURE_STATE_SPACE == true)
    ++TermProduct::instances;
    #endif
    type = TermType::TERM_PRODUCT;
    subtype = pt;
    this->_inComplement = false;
}

TermBaseSet::TermBaseSet() : states(), stateMask(0) {
    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermBaseSet::";
    this->dump();
    std::cout << "\n";
    #endif
    #if (MEASURE_STATE_SPACE == true)
    ++TermBaseSet::instances;
    #endif
    type = TERM_BASE;
    this->_inComplement = false;
}

TermBaseSet::TermBaseSet(VATA::Util::OrdVector<unsigned int>& s, unsigned int offset, unsigned int stateNo) : states(), stateMask(stateNo)  {
    #if (MEASURE_STATE_SPACE == true)
    ++TermBaseSet::instances;
    #endif
    type = TERM_BASE;
    for(auto state : s) {
        this->states.push_back(state);
        this->stateMask.set(state-offset, true);
    }
    this->_inComplement = false;
    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermBaseSet::";
    this->dump();
    std::cout << "\n";
    #endif
}

TermContinuation::TermContinuation(std::shared_ptr<SymbolicAutomaton> a, Term_ptr t, std::shared_ptr<SymbolType> s, bool b) : aut(a), term(t), symbol(s), underComplement(b) {
    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermContinuation::";
    this->dump();
    std::cout << "\n";
    #endif
    #if (MEASURE_STATE_SPACE == true)
    ++TermContinuation::instances;
    #endif
    this->type = TERM_CONTINUATION;
    #if (DEBUG_CONTINUATIONS == true)
    std::cout << "Postponing computation as [";
    t->dump();
    std::cout << "]\n";
    #endif
    this->_nonMembershipTesting = b;
    this->_inComplement = false;
}

TermList::TermList() {
    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermList::";
    this->dump();
    std::cout << "\n";
    #endif
    #if (MEASURE_STATE_SPACE == true)
    ++TermList::instances;
    #endif
    type = TERM_LIST;
    this->_inComplement = false;
}

TermList::TermList(Term_ptr first, bool isCompl) {
    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermList::";
    this->dump();
    std::cout << "\n";
    #endif
    #if (MEASURE_STATE_SPACE == true)
    ++TermList::instances;
    #endif
    this->type = TERM_LIST;
    this->list.push_back(first);
    this->_nonMembershipTesting = isCompl;
    this->_inComplement = false;
}

TermList::TermList(Term_ptr f, Term_ptr s, bool isCompl) {
    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermList::";
    this->dump();
    std::cout << "\n";
    #endif
    #if (MEASURE_STATE_SPACE == true)
    ++TermList::instances;
    #endif
    this->type = TERM_LIST;
    this->list.push_back(f);
    this->list.push_back(s);
    this->_nonMembershipTesting = isCompl;
    this->_inComplement = false;
}

TermFixpoint::TermFixpoint(std::shared_ptr<SymbolicAutomaton> aut, Term_ptr startingTerm, Symbols symList, bool inComplement, bool initbValue)
        : _sourceTerm(nullptr), _sourceIt(nullptr), _aut(aut), _bValue(initbValue) {
    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermFixpoint::";
    this->dump();
    std::cout << "\n";
    #endif
    #if (MEASURE_STATE_SPACE == true)
    ++TermFixpoint::instances;
    #endif

    // Initialize the aggregate function
    this->_InitializeAggregateFunction(inComplement);
    this->_nonMembershipTesting = inComplement;
    this->_inComplement = false;
    this->type = TERM_FIXPOINT;

    // Initialize the fixpoint as [nullptr, startingTerm]
    this->_fixpoint.push_front(startingTerm);
    this->_fixpoint.push_front(nullptr);

    // Push symbols to worklist
    for(auto symbol : symList) {
        this->_symList.push_back(symbol);
        this->_worklist.insert(this->_worklist.cbegin(), std::make_pair(startingTerm, symbol));
    }
}

TermFixpoint::TermFixpoint(std::shared_ptr<SymbolicAutomaton> aut, Term_ptr sourceTerm, Symbols symList, bool inComplement)
        : _sourceTerm(sourceTerm), _sourceIt(reinterpret_cast<TermFixpoint*>(sourceTerm.get())->GetIteratorDynamic()),
        _aut(aut), _worklist(), _bValue(inComplement) {
    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermFixpoint::";
    this->dump();
    std::cout << "\n";
    #endif
    #if (MEASURE_STATE_SPACE == true)
    ++TermFixpoint::instances;
    #endif
    assert(sourceTerm->type == TERM_FIXPOINT);

    // Initialize the aggregate function
    this->_InitializeAggregateFunction(inComplement);
    this->type = TERM_FIXPOINT;
    this->_nonMembershipTesting = inComplement;
    this->_inComplement = false;

    // Initialize the fixpoint
    this->_fixpoint.push_front(nullptr);
    // Push things into worklist
    for(auto symbol : symList) {
        this->_symList.push_back(symbol);
    }
}

/**
 * Tests the Term subsumption
 *
 * @param[in] t:    term we are testing subsumption against
 * @return:         true if this is subsumed by @p t
 */
bool Term::IsSubsumed(Term *t) {
    // TODO: Add Caching
    #if (DEBUG_TERM_SUBSUMPTION == true)
    this->dump();
    std::cout << " <?= ";
    t->dump();
    std::cout << "\n";
    #endif
    // unfold the continuation
    if(t->type == TERM_CONTINUATION) {
        // TODO: We should check that maybe we have different continuations
        TermContinuation *continuation = reinterpret_cast<TermContinuation *>(t);
        if (this->type == TERM_CONTINUATION) {
            TermContinuation *thisCont = reinterpret_cast<TermContinuation *>(this);
            assert(continuation->underComplement == thisCont->underComplement);
        }
        auto unfoldedContinuation = (continuation->aut->IntersectNonEmpty(continuation->symbol.get(),
                                                                          continuation->term,
                                                                          continuation->underComplement)).first;
        return this->IsSubsumed(unfoldedContinuation.get());
    } else if(this->type == TERM_CONTINUATION) {
        TermContinuation *continuation = reinterpret_cast<TermContinuation *>(this);
        auto unfoldedContinuation = (continuation->aut->IntersectNonEmpty(continuation->symbol.get(),
                                                                          continuation->term,
                                                                          continuation->underComplement)).first;
        return unfoldedContinuation->IsSubsumed(t);
    } else {
        if(this->_inComplement) {
            if(this->type == TERM_EMPTY) {
                return t->type == TERM_EMPTY;
            } else {
                return t->_IsSubsumedCore(this);
            }
        } else {
            if(t->type == TERM_EMPTY) {
                return this->type == TERM_EMPTY;
            } else {
                return this->_IsSubsumedCore(t);
            }
        }
    }
}

bool TermEmpty::_IsSubsumedCore(Term *t) {
    // Empty term is subsumed by everything (probably)
    return true;
}

bool TermProduct::_IsSubsumedCore(Term* t) {
    assert(t->type == TERM_PRODUCT);

    // Retype and test the subsumption component-wise
    TermProduct *rhs = reinterpret_cast<TermProduct*>(t);
    return (this->left->IsSubsumed(rhs->left.get())) && (this->right->IsSubsumed(rhs->right.get()));
}

bool TermBaseSet::_IsSubsumedCore(Term* term) {
    assert(term->type == TERM_BASE);

    // Test component-wise, not very efficient though
    // TODO: Change to bit-vectors if possible
    TermBaseSet *t = reinterpret_cast<TermBaseSet*>(term);
    if(t->states.size() < this->states.size()) {
        return false;
    } else {
        // TODO: Maybe we could exploit that we have ordered vectors
        for(auto state : this->states) {
            auto isIn = std::find(t->states.begin(), t->states.end(), state);
            if(isIn == t->states.end()) {
                // Not in, false
                return false;
            }
        }
        return true;
    }
}

bool TermContinuation::_IsSubsumedCore(Term *t) {
    // TODO: How to do this smartly?
    // TODO: Maybe if we have {} we can answer sooner, without unpacking

    // We unpack this term
    auto unfoldedTerm = (this->aut->IntersectNonEmpty(this->symbol.get(), this->term, this->underComplement)).first;
    return unfoldedTerm->IsSubsumed(t);
}

bool TermList::_IsSubsumedCore(Term* t) {
    assert(t->type == TERM_LIST);

    // Reinterpret
    TermList* tt = reinterpret_cast<TermList*>(t);
    // Do the item-wise subsumption check
    for(auto item : this->list) {
        bool subsumes = false;
        for(auto tt_item : tt->list) {
            if(item->IsSubsumed(tt_item.get())) {
                subsumes = true;
                break;
            }
        }
        if(!subsumes) return false;
    }

    return true;
}

bool TermFixpoint::_IsSubsumedCore(Term* t) {
    assert(t->type == TERM_FIXPOINT);

    // Reinterpret
    TermFixpoint* tt = reinterpret_cast<TermFixpoint*>(t);

    // Do the piece-wise comparison
    for(auto item : this->_fixpoint) {
        // Skip the nullptr
        if(item == nullptr) continue;
        bool subsumes = false;
        for(auto tt_item : tt->_fixpoint) {
            if(tt_item == nullptr) continue;
            if(item->IsSubsumed(tt_item.get())) {
                subsumes = true;
                break;
            }
        }
        if(!subsumes) return false;
    }

    return true;
}

/**
 * Tests the subsumption over the list of terms
 *
 * @param[in] fixpoint:     list of terms contained as fixpoint
 */
bool TermEmpty::IsSubsumedBy(std::list < Term_ptr > &fixpoint) {
    // Empty term is subsumed by everything
    return true;
}

bool TermProduct::IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
    // TODO: Add caching
    // Empty things is always subsumed
    if(this->IsEmpty()) {
        return true;
    }

    // For each item in fixpoint
    for(auto item : fixpoint) {
        // Nullptr is skipped
        if(item == nullptr) continue;

        // Test the subsumption
        if(this->IsSubsumed(item.get())) {
            return true;
        }
    }

    return false;
}

bool TermBaseSet::IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
    // TODO: Add caching
    if(this->IsEmpty()) {
        return true;
    }

    // For each item in fixpoint
    for(auto item : fixpoint) {
        // Nullptr is skipped
        if(item == nullptr) continue;

        // Test the subsumption
        if(this->IsSubsumed(item.get())) {
            return true;
        }
    }

    return false;
}

bool TermContinuation::IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
    assert(false && "TermContSubset.IsSubsumedBy() is impossible to happen~!");
}

bool TermList::IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
    // TODO: Add caching
    #if (DEBUG_TERM_SUBSUMPTION == true)
    this->dump();
    std::cout << " <?= ";
    std::cout << "{";
    for(auto item : fixpoint) {
        if(item == nullptr) continue;
        item->dump();
        std::cout << ",";
    }
    std::cout << "}";
    std::cout << "\n";
    #endif
    if(this->IsEmpty()) {
        return true;
    }

    // For each item in fixpoint
    for(auto item : fixpoint) {
        // Nullptr is skipped
        if(item == nullptr) continue;

        return this->IsSubsumed(item.get());
    }

    return false;
}

bool TermFixpoint::IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
    // TODO: There should be unfolding of fixpoint probably
    #if (DEBUG_TERM_SUBSUMPTION == true)
    this->dump();
    std::cout << " <?= ";
    std::cout << "{";
    for(auto item : fixpoint) {
        if(item == nullptr) continue;
        item->dump();
        std::cout << ",";
    }
    std::cout << "}";
    std::cout << "\n";
    #endif

    // Component-wise comparison
    for(auto item : fixpoint) {
        if(item == nullptr) continue;
        if (this->IsSubsumed(item.get())) {
            return true;
        }
    }
    return false;
}

/**
 * Tests the emptiness of Term
 */
bool TermEmpty::IsEmpty() {
    return true;
}

bool TermProduct::IsEmpty() {
    return this->left->IsEmpty() && this->right->IsEmpty();
}

bool TermBaseSet::IsEmpty() {
    return this->states.size() == 0;
}

bool TermContinuation::IsEmpty() {
    return false;
}

bool TermList::IsEmpty() {
    if(this->list.size() == 0) {
        return true;
    } else {
        for(auto item : this->list) {
            if(!item->IsEmpty()) {
                return false;
            }
        }
        return false;
    }
}

bool TermFixpoint::IsEmpty() {
    return this->_worklist.empty();
}

/**
 * Measures the state space as number of states
 */
unsigned int TermEmpty::MeasureStateSpace() {
    return 0;
}

unsigned int TermProduct::MeasureStateSpace() {
    return this->left->MeasureStateSpace() + this->right->MeasureStateSpace() + 1;
}

unsigned int TermBaseSet::MeasureStateSpace() {
    return this->states.size();
}

unsigned int TermContinuation::MeasureStateSpace() {
    return 1;
}

unsigned int TermList::MeasureStateSpace() {
    unsigned int count = 0;
    // Measure state spaces of all subterms
    for(auto item : this->list) {
        count += item->MeasureStateSpace();
    }

    return count;
}

unsigned int TermFixpoint::MeasureStateSpace() {
    unsigned count = 1;
    for(auto item : this->_fixpoint) {
        if(item == nullptr) {
            continue;
        }
        assert(item != nullptr);
        count += item->MeasureStateSpace();
    }

    return count;
}
/**
 * Dumping functions
 */
std::ostream& operator <<(std::ostream& osObject, Term& z) {
    // TODO: Optimize this you lazy fuck!
#if (DEBUG_TERM_UNIQUENESS == true)
    osObject << "[" << &z <<"]";
#endif
    z.dump();
    return osObject;
}

void Term::dump() {
    if(this->_inComplement) {
        std::cout << "\033[1;31m{\033[0m";
    }
    this->_dumpCore();
    if(this->_inComplement) {
        std::cout << "\033[1;31m}\033[0m";
    }
}

void TermEmpty::_dumpCore() {
    std::cout << "\u2205";
}

void TermProduct::_dumpCore() {
    if(this->subtype == ProductType::E_INTERSECTION) {
        std::cout << "\033[1;32m";
    } else {
        std::cout << "\033[1;33m";
    }
    std::cout << "{\033[0m";
    left->dump();
    if(this->subtype == ProductType::E_INTERSECTION) {
        std::cout << "\033[1;32m \u2293 \033[0m";
    } else {
        //std::cout << " \u22C3 ";
        std::cout << "\033[1;33m \u2294 \033[0m";
    };
    right->dump();
    if(this->subtype == ProductType::E_INTERSECTION) {
        std::cout << "\033[1;32m";
    } else {
        std::cout << "\033[1;33m";
    }
    std::cout << "}\033[0m";
}

void TermBaseSet::_dumpCore() {
    std::cout << "\033[1;35m{";
    for (auto state : this->states) {
        std::cout << (state) << ",";
    }
    std::cout << "}\033[0m";
}

void TermContinuation::_dumpCore() {
    std::cout << "?";
    term->dump();
    std::cout << "?";
    std::cout << "'";
    if(symbol != nullptr) {
        std::cout << (*symbol);
    }
    std::cout << "'";
    std::cout << "?";
}

void TermList::_dumpCore() {
    std::cout << "\033[1;36m{\033[0m";
    for(auto state : this->list) {
        state->dump();
        std::cout  << ",";
    }
    std::cout << "\033[1;36m}\033[0m";
}

void TermFixpoint::_dumpCore() {
    std::cout << "\033[1;34m{\033[0m";
    for(auto item : this->_fixpoint) {
        if(item == nullptr) {
            continue;
        }
        item->dump();
        std::cout << "\033[1;34m,\033[0m";
    }
    std::cout << "\033[1;34m}\033[0m";
}
// <<< ADDITIONAL TERMBASESET FUNCTIONS >>>

bool TermBaseSet::Intersects(TermBaseSet* rhs) {
    for (auto lhs_state : this->states) {
        for(auto rhs_state : rhs->states) {
            if(lhs_state == rhs_state) {
                return true;
            }
        }
    }
    return false;
}

// <<< ADDITIONAL TERMFIXPOINT FUNCTIONS >>>

/**
 * Does the computation of the next fixpoint, i.e. the next iteration.
 */
void TermFixpoint::ComputeNextFixpoint() {
    assert(!_worklist.empty());

    // Pop the front item from worklist
    WorklistItemType item = _worklist.front();
    _worklist.pop_front();

    // Compute the results
    ResultType result = _aut->IntersectNonEmpty(&item.second, item.first, this->_nonMembershipTesting);

    // If it is subsumed by fixpoint, we don't add it
    if(result.first->IsSubsumedBy(_fixpoint)) {
        return;
    }

    // Push new term to fixpoint
    _fixpoint.push_back(result.first);
    // Aggregate the result of the fixpoint computation
    _bValue = this->_aggregate_result(_bValue,result.second);
    // Push new symbols from _symList
    for(auto symbol : _symList) {
        _worklist.insert(_worklist.cbegin(), std::make_pair(result.first, symbol));
    }
}

/**
 * Computes the pre of the fixpoint, i.e. it does pre on all the things
 * as we had already the fixpoint computed from previous step
 */
void TermFixpoint::ComputeNextPre() {
    assert(!_worklist.empty());

    // Pop item from worklist
    WorklistItemType item = _worklist.front();
    _worklist.pop_front();

    // Compute the results
    ResultType result = _aut->IntersectNonEmpty(&item.second, item.first, this->_nonMembershipTesting);

    // If it is subsumed we return
    if(result.first->IsSubsumedBy(_fixpoint)) {
        return;
    }

    // Push the computed thing and aggregate the result
    _fixpoint.push_back(result.first);
    _bValue = this->_aggregate_result(_bValue,result.second);
}

/**
 * When we are computing the fixpoint, we aggregate the results by giant
 * OR, as we are doing the epsilon check in the union. However, if we are
 * under the complement we have to compute the AND of the function, as we
 * are computing the epsilon check not in the union.
 *
 * @param[in] inComplement:     whether we are in complement
 */
void TermFixpoint::_InitializeAggregateFunction(bool inComplement) {
    if(!inComplement) {
        this->_aggregate_result = [](bool a, bool b) {return a || b;};
    } else {
        this->_aggregate_result = [](bool a, bool b) {return a && b;};
    }
}

/**
 * @return: result of fixpoint
 */
bool TermFixpoint::GetResult() {
    return this->_bValue;
}

/**
 * Returns whether we are computing the Pre of the already finished fixpoint,
 * or if we are computing the fixpoint from scratch
 *
 * @return: semantics of the fixpoint
 */
FixpointTermSem TermFixpoint::GetSemantics() const {
    return (nullptr == _sourceTerm) ? E_FIXTERM_FIXPOINT : E_FIXTERM_PRE;
}

// <<< EQUALITY CHECKING FUNCTIONS >>>
/**
 * Operation for equality checking, tests first whether the two pointers
 * are the same (this is for the (future) unique pointer cache), checks
 * if the types are the same to optimize the equality checking.
 *
 * Otherwise it calls the _eqCore() function for specific comparison of
 * terms
 *
 * @param[in] t:        tested term
 */
bool Term::operator==(const Term &t) {
    if(this == &t) {
        // Same thing
        return true;
    } else if (this->type != t.type) {
        // Terms are of different type
        return false;
    } else {
        return this->_eqCore(t);
    }
}

bool TermEmpty::_eqCore(const Term &t) {
    assert(t.type == TERM_EMPTY && "Testing equality of different term types");
    return true;
}

bool TermProduct::_eqCore(const Term &t) {
    assert(t.type == TERM_PRODUCT && "Testing equality of different term types");

    const TermProduct &tProduct = static_cast<const TermProduct&>(t);
    return (*tProduct.left == *this->left) && (*tProduct.right == *this->right);
}

bool TermBaseSet::_eqCore(const Term &t) {
    assert(t.type == TERM_BASE && "Testing equality of different term types");

    const TermBaseSet &tBase = static_cast<const TermBaseSet&>(t);
    if(this->states.size() != tBase.states.size()) {
        return false;
    } else {
        // check the things, should be sorted
        auto lhsIt = this->states.begin();
        auto rhsIt = tBase.states.begin();
        for(; lhsIt != this->states.end(); ++lhsIt, ++rhsIt) {
            if(*lhsIt != *rhsIt) {
                return false;
            }
        }
        return true;
    }
}

bool TermContinuation::_eqCore(const Term &t) {
    assert(t.type == TERM_CONTINUATION && "Testing equality of different term types");
    G_NOT_IMPLEMENTED_YET("TermContinuation::_eqCore");
}

bool TermList::_eqCore(const Term &t) {
    assert(t.type == TERM_LIST && "Testing equality of different term types");

    const TermList &tList = static_cast<const TermList&>(t);
    G_NOT_IMPLEMENTED_YET("TermList::_eqCore");
}

bool TermFixpoint::_eqCore(const Term &t) {
    assert(t.type == TERM_FIXPOINT && "Testing equality of different term types");

    const TermFixpoint &tFix = static_cast<const TermFixpoint&>(t);
    if(this->_fixpoint.size() != tFix._fixpoint.size()) {
        return false;
    } else {
        for(auto it = this->_fixpoint.begin(); it != this->_fixpoint.end(); ++it) {
            if(*it == nullptr)
                continue;
            bool found = false;
            for(auto tit = tFix._fixpoint.begin(); tit != tFix._fixpoint.end(); ++tit) {
                if(*tit == nullptr)
                    continue;
                if(**it == **tit) {
                    found = true;
                    break;
                }
            }
            if(!found) {
                return false;
            }
        }
        return true;
    }
}
#include "Workshops.h"
#include "Term.h"

namespace Workshops {
    TermWorkshop::TermWorkshop(SymbolicAutomaton* aut) : _aut(aut) { }

    void TermWorkshop::InitializeWorkshop() {
        switch(this->_aut->type) {
            case AutType::BASE:
                this->_bCache = new BaseCache();
                break;
            case AutType::BINARY:
            case AutType::INTERSECTION:
            case AutType::UNION:
                this->_pCache = new ProductCache();
                break;
            case AutType::COMPLEMENT:
                break;
            case AutType::PROJECTION:
                this->_lCache = new ListCache();
                break;
            default:
                assert(false && "Missing implementation for this type");
                break;
        }
    }

    /**
     * Checks if there is already created TermBaseSet in cache, in case there is not
     * it creates the new object and populates the cache.
     *
     * @param[in] states:       parameters used to create TermBaseSet
     * @param[in] offset:       offset for bitmask
     * @param[in] stateno:      state number for bitmask
     * @return:                 unique pointer for TermBaseSet
     */
    TermBaseSet* TermWorkshop::CreateBaseSet(VATA::Util::OrdVector<unsigned int>& states, unsigned int offset, unsigned int stateno) {
#if (OPT_GENERATE_UNIQUE_TERMS == true && UNIQUE_BASE == true)
        assert(this->_bCache != nullptr);

        Term* termPtr = nullptr;
        if(!this->_bCache->retrieveFromCache(states, termPtr)) {
            #if (DEBUG_WORKSHOPS == true && DEBUG_TERM_CREATION == true)
            std::cout << "[*] Creating BaseSet: ";
            #endif
            // The object was not created yet, so we create it and store it in cache
            termPtr = new TermBaseSet(states, offset, stateno);
            this->_bCache->StoreIn(states, termPtr);
        }
        assert(termPtr != nullptr);
        return reinterpret_cast<TermBaseSet*>(termPtr);
#else
        return new TermBaseSet(states, offset, stateno);
#endif
    }

    /**
     * Checks if there is already created TermProduct in cache, in case there is not
     * it creates the new object and populates the cache
     *
     * @param[in] lptr:     left term of product
     * @param[in] rptr:     right term of product
     * @param[in] type:     type of the product
     */
    TermProduct* TermWorkshop::CreateProduct(Term_ptr const&lptr, Term_ptr const&rptr, ProductType type) {
        // TODO: Can there be sets that have same lhs rhs, but different product type??
        // TODO: I don't think so actually, because this is on the node, so it cannot generate different things
        // TODO: And same thing goes for complements
#if (OPT_GENERATE_UNIQUE_TERMS == true && UNIQUE_PRODUCTS == true)
        assert(this->_pCache != nullptr);

        Term* termPtr = nullptr;
        auto productKey = std::make_pair(lptr.get(), rptr.get());
        if(!this->_pCache->retrieveFromCache(productKey, termPtr)) {
            #if (DEBUG_WORKSHOPS == true && DEBUG_TERM_CREATION == true)
            std::cout << "[*] Creating Product: ";
            std::cout << "from ["<< lptr << "] + [" << rptr << "] to ";
            #endif
            // The object was not created yet, so we create it and store it in cache
            termPtr = new TermProduct(lptr, rptr, type);
            this->_pCache->StoreIn(productKey, termPtr);
        }
        assert(termPtr != nullptr);
        return reinterpret_cast<TermProduct*>(termPtr);
#else
        return new TermProduct(lptr, rptr, type);
#endif
    }

    /**
     * Checks if there is already created list with one symbol, in case there is
     * not it creates the new object and populates the cache
     *
     * @param[in] startTerm:        first term in list
     * @param[in] inComplement:     whether the list is complemented
     * @return: unique pointer
     */
    TermList* TermWorkshop::CreateList(Term_ptr const& startTerm, bool inComplement) {
#if (OPT_GENERATE_UNIQUE_TERMS == true && UNIQUE_LISTS == true)
        assert(this->_lCache != nullptr);

        Term* termPtr = nullptr;
        auto productKey = startTerm.get();
        if(!this->_lCache->retrieveFromCache(productKey, termPtr)) {
            #if (DEBUG_WORKSHOPS == true && DEBUG_TERM_CREATION == true)
            std::cout << "[*] Creating List: ";
            std::cout << "from ["<< startTerm << "] to ";
            #endif
            termPtr = new TermList(startTerm, inComplement);
            this->_lCache->StoreIn(productKey, termPtr);
        }
        assert(termPtr != nullptr);
        return reinterpret_cast<TermList*>(termPtr);
#else
        return new TermList(startTerm, inComplement);
#endif
    }
    //TermFixpoint::TermFixpoint(std::shared_ptr<SymbolicAutomaton> aut, Term_ptr startingTerm, Symbols symList, bool inComplement, bool initbValue)
    //TermFixpoint::TermFixpoint(std::shared_ptr<SymbolicAutomaton> aut, Term_ptr sourceTerm, Symbols symList, bool inComplement)
    //TermContinuation::TermContinuation(std::shared_ptr<SymbolicAutomaton> a, Term_ptr t, std::shared_ptr<SymbolType> s, bool b)
    TermFixpoint* TermWorkshop::CreateFixpoint(Term_ptr const& source, SymbolList &symbols, bool inCompl, bool initValue) {
#if (OPT_GENERATE_UNIQUE_TERMS == true && UNIQUE_FIXPOINTS == true)

#else
        return new TermFixpoint(std::shared_ptr<SymbolicAutomaton>(this->_aut), source, symbols, inCompl, initValue);
#endif
    };

    TermFixpoint* TermWorkshop::CreateFixpointPre(Term_ptr const& source, SymbolList &symbols, bool inCompl) {
#if (OPT_GENERATE_UNIQUE_TERMS == true && UNIQUE_FIXPOINTS == true)

#else
        return new TermFixpoint(std::shared_ptr<SymbolicAutomaton>(this->_aut), source, symbols, inCompl);
#endif
    }

    TermContinuation* TermWorkshop::CreateContinuation(Term_ptr const&, Symbol_shared&, bool) {
        G_NOT_IMPLEMENTED_YET("CreateContinuation");
        return nullptr;
    }

    TermList* TermWorkshop::ExtendList(Term* list, Term* newTerm) {
        G_NOT_IMPLEMENTED_YET("ExtendList");
        return nullptr;
    }

    /***
     * Dump stats for Workshop. Dumps the cache if it is created.
     */
    void TermWorkshop::Dump() {
        if(this->_bCache != nullptr) {
            std::cout << "  \u2218 BaseCache stats -> ";
            this->_bCache->dumpStats();
        }
        if(this->_pCache != nullptr) {
            std::cout << "  \u2218 ProductCache stats -> ";
            this->_pCache->dumpStats();
        }
    }

    void dumpBaseKey(BaseKey const&s) {
        std::cout << s;
    }

    void dumpProductKey(ProductKey const&s) {
        std::cout << "<" << (*s.first) << ", " << (*s.second) << ">";
    }

    void dumpCacheData(Term *&s) {
        std::cout << s;
    }
}
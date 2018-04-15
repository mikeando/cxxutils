#pragma once

#include "cxxutils/result.hpp"
class ValidResultComparator {
public:
    template<typename T>
    bool
    ok( const Result<T> & result ) const {
        return result.isOK();
    }

    template<typename T>
    std::string
    describe_failure(const Result<T> & result) const {
        std::stringstream ss;
        ss<<"expected valid result but got failed with '" << result.getException().mesg<<"'";
        return ss.str();
    }
};

class FailedResultComparator {
public:
    template<typename T>
    bool
    ok( const Result<T> & result ) const {
        return !result.isOK();
    }

    template<typename T>
    std::string
    describe_failure(const Result<T> & result) const {
        return "expected failed result but got ok result";
    }
};

template<typename T>
class ResultWithComparator {
public:
    template<typename U>
    bool
    ok( const Result<U> & result ) const {
        return result.isOK() && comparator.ok(result.getValue());
    }

    template<typename U>
    std::string
    describe_failure(const Result<U> & result) const {
        if(!result.isOK())
          return "expected valid result but got failed result with '"+result.getException().mesg+"'";
        return "Expected Result with value satisfying : " + 
                comparator.describe_failure(result.getValue());
    }

    ResultWithComparator( T&& comparator ) : comparator(std::move(comparator)) 
    {
    }

    T comparator;
};

static inline ValidResultComparator isValidResult() {
    return ValidResultComparator();
}

static inline FailedResultComparator isFailedResult() {
    return FailedResultComparator();
}

template<typename T>
static inline ResultWithComparator<T> isResultWhereValue(T && comparator) {
    return ResultWithComparator<T>(std::move(comparator));
}

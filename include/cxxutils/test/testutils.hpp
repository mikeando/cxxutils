#pragma once
//
// Created by Michael Anderson on 26/11/2015.
//

#include "gtest/gtest.h"

#include "StringComparators.hpp"
//#include "MarkdownComparators.hpp"
#include "ResultComparators.hpp"

namespace testutils {

  template<typename T>
  struct IOWrapper;

  template<typename T>
  IOWrapper<T> wrap(const T & v);


  template<typename T>
  struct TestIOHelper {
    static std::ostream& output(std::ostream & os, const T & value) {
      return os<<value;
    }
  };

  template<>
  struct TestIOHelper<bool> {
    static std::ostream& output(std::ostream & os, bool value) {
      return os<<(value?"true":"false");
    }
  };


  template<typename T>
  struct TestIOHelper<std::vector<T>> {
    static std::ostream& output(std::ostream& os, const std::vector<T> & vec) {
      os<<"{";
      std::for_each(vec.begin(), vec.end(), [&os](const T & v){ os<<wrap(v)<<","; });
      os<<"}";
      return os;
    }
  };


  template<typename T>
  struct IOWrapper {
    const T& ref;
    IOWrapper(const T & v) : ref(v) {}
    friend std::ostream& operator<<(std::ostream & os, const IOWrapper<T> & w) {
      return TestIOHelper<T>::output(os,w.ref);
    }
  };

  template<typename T>
  IOWrapper<T> wrap(const T & v) {
    return IOWrapper<T>(v);
  }

}


class AssertionFailedException {
public:
    AssertionFailedException(const std::string & mesg_) :mesg(mesg_) {}
    const std::string mesg;
};

template<typename T>
class ValueComparator {
public:
    ValueComparator(const T& value) : value_(value) {
    }

    template<typename U>
    bool
    ok(const U & value) const {
        return value_==value;
    }

    template<typename U>
    std::string
    describe_failure(const U & value) const {
        std::stringstream ss;
        ss<<"expected : "<<testutils::wrap(value_) <<"\n"
          <<"actual   : "<<testutils::wrap(value);
        return ss.str();
    }
private:
    const T value_;
};

template<int N>
class ValueComparator<char[N]> {
public:
    ValueComparator(const char value[N]) : value_(value) {
    }

    template<typename U>
    bool
    ok(const U & value) const {
        return value_==value;
    }

    template<typename U>
    std::string
    describe_failure(const U & value) const {
        std::stringstream ss;
        ss<<"expected : "<<testutils::wrap(value_)<<"\n"
          <<"actual   : "<<testutils::wrap(value);
        return ss.str();
    }
private:
    const char* value_;
};

template<typename T>
class TypeComparator {
public:
    template<typename U>
    bool
    ok( const U* v) const {
        return dynamic_cast<const T*>(v)!=nullptr;
    }

    template<typename U, typename D>
    bool
    ok( const std::unique_ptr<U,D> & ptr) const {
        return ok(ptr.get());
    }

    template<typename U>
    bool
    ok( const std::shared_ptr<U> & ptr) const {
        return ok(ptr.get());
    };

    template<typename U>
    std::string describe_failure(const U* value) const {
        std::stringstream ss;
        ss<<"expected "<<typeid(T).name()<<" or derived class but got "<<((value!= nullptr)?typeid(*value).name():"null");
        return ss.str();
    }

    template<typename U, typename D>
    std::string describe_failure(const std::unique_ptr<U,D> & ptr) const {
        return describe_failure(ptr.get());
    }

    template<typename U>
    std::string describe_failure(const std::shared_ptr<U> & ptr) const {
        return describe_failure(ptr.get());
    }

};

class NullComparator {
public:
    template<typename T>
    bool
    ok( const T* ptr) const {
        return ptr== nullptr;
    }

    template<typename T>
    bool
    ok( const std::unique_ptr<T> & ptr) const {
        return !ptr;
    }

    template<typename T>
    std::string
    describe_failure(const T* value) const {
        std::stringstream ss;
        ss<<"expected null but got "<<testutils::wrap(value);
        return ss.str();
    }

    template<typename T>
    std::string
    describe_failure(const std::unique_ptr<T> & value) const {
        std::stringstream ss;
        ss<<"expected null but got "<<testutils::wrap(value.get());
        return ss.str();
    }
};

class NotNullComparator {
public:
    template<typename T>
    bool
    ok( const T* ptr) const {
        return ptr!= nullptr;
    }

    template<typename T, typename D>
    bool
    ok( const std::unique_ptr<T, D> & ptr) const {
        return ptr.get() != nullptr;
    };

    template<typename T>
    bool
    ok( const std::shared_ptr<T> & ptr) const {
        return ptr.get() != nullptr;
    };

    template<typename T>
    std::string
    describe_failure(const T* value) const {
        std::stringstream ss;
        ss<<"expected not-null but got null";
        return ss.str();
    }

    template<typename T, typename D>
    std::string
    describe_failure( const std::unique_ptr<T, D> & ptr) const {
        return "expected not-null but got null";
    };

    template<typename T>
    std::string
    describe_failure( const std::shared_ptr<T> & ptr) const {
        return "expected not-null but got null";
    };
};


template<typename V>
class ContainsComparator {
public:

    template<typename T>
    bool
    ok( const T & container ) const {
        for( auto v : container ) {
            if(v==value)
                return true;
        }
        return false;
    }

    template<typename T>
    std::string
    describe_failure(const T & container) const {
        std::stringstream ss;
        ss << "expected container containing " << testutils::wrap(value) 
           << " but got " << testutils::wrap(container);
        return ss.str();
    }

    V value;

    ContainsComparator(const V & value) : value(value) {}
};



template<typename T>
ValueComparator<T> is(const T & value) {
    return ValueComparator<T>(value);
}

template<typename T>
TypeComparator<T> isInstanceOf() {
    return TypeComparator<T>();
}

static inline NotNullComparator isNotNull() {
    return NotNullComparator();
}

static inline NullComparator isNull() {
    return NullComparator();
}

template<typename T>
static inline ContainsComparator<T> contains(const T & value) {
    return ContainsComparator<T>(value);
}

template<typename T1, typename T2>
class AndComparator {
private:
    T1 c1;
    T2 c2;
public:
    AndComparator(T1 && c1, T2 && c2): c1(c1), c2(c2) {}

    template<typename T>
    bool
    ok( const T & v) const {
        return c1.ok(v) && c2.ok(v);
    };

    template<typename T>
    std::string
    describe_failure(const T& v) const {
        if(!c1.ok(v)) {
            return c1.describe_failure(v);
        } else {
            return c2.describe_failure(v);
        }
    }

};

template <typename T1, typename T2>
auto and_(T1 && c1, T2 && c2) {
    return AndComparator<T1,T2>(std::move(c1),std::move(c2));
}



// #define EXPECT_FLOAT_EQ(expected, actual)\
//  EXPECT_PRED_FORMAT2(::testing::internal::CmpHelperFloatingPointEQ<float>, \
//                      expected, actual)

template<typename T>
class WrappedCondition {
public:

    T condition;
    const std::string objectStr;
    const std::string conditionStr;

    WrappedCondition(
        const std::string & objectStr,
        const std::string & conditionStr,
        const T & condition
        ) :
        objectStr(objectStr),
        conditionStr(conditionStr),
        condition(condition) {
    }

    template<typename W>
    testing::AssertionResult operator()( const char * valueStr, const W & value) {
        if(condition.ok(value)) {
            return testing::AssertionSuccess();
        } else {
            return testing::AssertionFailure() 
              << "assertion failed: "<<objectStr<<" "<<conditionStr<<std::endl
              << condition.describe_failure(value);
        }
    }
};

template<typename T>
WrappedCondition<T> wrapCondition(const std::string objectStr, const std::string conditionStr, const T condition) {
    return WrappedCondition<T>(objectStr, conditionStr, condition);
}

#define assertThat(object, condition) ASSERT_PRED_FORMAT1(::wrapCondition(#object, #condition, condition), object)

template<typename T, typename C>
void
assertThatImpl(const std::string & file,
               int line,
               const std::string & function,
               const std::string & value_expr,
               const std::string & test_expr,
               const T & value,
               const C & comparator) {
    if(comparator.ok(value))
        return;

    std::stringstream ss;
    ss<<file<<":"<<line<<":"<<function<<" assertion failed"<<std::endl;
    ss<<"failed   : assertThat("<<value_expr<<","<<test_expr<<") failed: "<<std::endl;
    ss<<comparator.describe_failure(value);
    throw AssertionFailedException(ss.str());
}

static inline void failImpl(const std::string & file,
              int line,
              const std::string & function,
              const std::string & message) {
    std::stringstream ss;
    ss<<file<<":"<<line<<":"<<function<<" assertion failed"<<std::endl;
    ss<<"failed   : "<<message<<std::endl;
    throw AssertionFailedException(ss.str());
}

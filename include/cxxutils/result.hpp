#pragma once

#include "cxxutils/cxx14shims.hpp"
#include <string>
#include <assert.h>

#include "optional.hpp"

class ResultException {
public:
    std::string component;
    std::string mesg;

    ResultException() : component("unknown component"), mesg("Unknown exception")  {}
    explicit ResultException(const std::string & component_in, const std::string & mesg_in) : component(component_in), mesg(mesg_in) {}
    ResultException(const ResultException & orig) : component(orig.component), mesg(orig.mesg) {}
    ResultException( ResultException && orig) {
        std::swap(component, orig.component);
        std::swap(mesg, orig.mesg);
    }
};

template<typename T>
class Result {

private:

    Result( optional<T> && value, optional<ResultException> && exception ) :
        value(std::move(value)),
        exception(std::move(exception)) {
    }

    optional<T> value;
    optional<ResultException> exception;

public:
    static Result<T> ok(const T & value) {
        return Result<T>( optional<T>(value), optional<ResultException>() );
    }

    static Result<T> ok(T && value) {
        return Result<T>( optional<T>(std::move(value)), optional<ResultException>() );
    }

    static Result<T> failed(const ResultException & e) {
        return Result<T>( optional<T>(), optional<ResultException>(e) );
    }

    static Result<T> failed(ResultException && e) {
        return Result<T>( optional<T>(), optional<ResultException>( std::move(e) ) );
    }

    void ensureValid() const {
        bool hasValue = value.hasValue();
        bool hasException = exception.hasValue();

        if(hasValue && hasException)
            throw std::runtime_error("Invalid Result: Result has both value and exception");
        if(!hasValue && !hasException)
            throw std::runtime_error("Invalid Result: Result has neither value or exception");
    }


    bool isOK() const {
        ensureValid();
        return value.hasValue();
    }

    const T& getValue() const {
        ensureValid();
        assert(value.hasValue());
        return value.getValue();
    }

    const T& getValueOrThrow() const {
        ensureValid();
        if(!isOK())
            throw exception.getValue();
        return value.getValue();
    }

    std::unique_ptr<T>&& takeValuePtr() {
        return std::move(value);
    }

    T&& takeValue() {
        return value.takeValue();
    }

    const ResultException& getException() const {
        ensureValid();
        return exception.getValue();
    }

    template<typename U>
    static Result<T> translateError(const Result<U> & u) {
        return Result<T>::failed(u.getException());
    }


   template<typename FN>
   auto map(FN f) -> Result<decltype(f(getValue()))> {
      using RESULT = Result<decltype(f(getValue()))>;
      if(isOK()) {
        return RESULT::ok(f(getValue()));
      }
      return RESULT::translateError( *this );
    }

    template<typename FN>
    auto take_map(FN f) -> Result<decltype(f(takeValue()))> {
        using RESULT = Result<decltype(f(takeValue()))>;
        if(isOK()) {
            return RESULT::ok(f(takeValue()));
        }
        return RESULT::translateError( *this );
    }

    template<typename FN>
    auto take_map_void(FN f) -> Result<void>;

   template<typename FN>
   auto flatmap(FN f) -> decltype(f(getValue())) {
       if(isOK()) {
         return f(getValue());
       }
       return decltype(f(getValue()))::translateError( *this );
     }

   template<typename FN>
   void on_failure(FN f) {
       if(!isOK()) {
         f(getException());
       }
   }

};

template<>
class Result<void> {

private:

    explicit Result( optional<ResultException> && exception ) :
        exception(std::move(exception)) {
    }

    optional<ResultException> exception;


public:
    static Result<void> ok() {
        return Result<void>( optional<ResultException>() );
    }

    static Result<void> failed(const ResultException & e) {
        return Result<void>( optional<ResultException>(e) );
    }

    static Result<void> failed(ResultException && e) {
        return Result<void>( optional<ResultException>(std::move(e)) );
    }

    bool isOK() const {
        return !exception.hasValue();
    }

    const ResultException& getException() const {
        return exception.getValue();
    }

    template<typename U>
    static Result<void> translateError(const Result<U> & u) {
        return Result<void>::failed(u.getException());
    }

   template<typename FN>
#define RESULT Result<decltype(f())>
   auto map(FN f) -> RESULT {
      if(isOK()) {
        return RESULT::ok(f());
      }
      return RESULT::translateError( *this );
    }
#undef RESULT

   template<typename FN>
   auto flatmap(FN f) -> decltype(f()) {
       if(isOK()) {
         return f();
       }
       return decltype(f())::translateError( *this );
     }

   template<typename FN>
   void on_failure(FN f) {
       if(!isOK()) {
         f(getException());
       }
   }

};

template<typename T, typename... Args>
Result<T> make_result_ok(Args&&... args) {
  return Result<T>::ok( T(std::forward<Args>(args)... ));
}

template<typename T, typename... Args>
Result<std::unique_ptr<T>> make_result_unique_ok(Args&&... args) {
  return Result<std::unique_ptr<T>>::ok( cxx14::make_unique<T>(std::forward<Args>(args)... ));
}

template<typename T, typename... Args>
Result<std::shared_ptr<T>> make_result_shared_ok(Args&&... args) {
  return Result<std::shared_ptr<T>>::ok( std::make_shared<T>(std::forward<Args>(args)... ));
}

template<typename T, typename... Args>
Result<T> make_result_failed(Args&&... args) {
  return Result<T>::failed( ResultException(std::forward<Args>(args)... ));
}


namespace ResultUtils {
}

template<typename T>
template<typename FN>
auto Result<T>::take_map_void(FN f) -> Result<void> {
    using RESULT = Result<void>;
    if(isOK()) {
        f(takeValue());
        return RESULT::ok();
    }
    return RESULT::translateError( *this );
}


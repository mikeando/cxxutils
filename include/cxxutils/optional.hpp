#pragma once

#include <exception>

struct MissingOptionalValue : std::runtime_error {
    MissingOptionalValue() : std::runtime_error("Optional value is not set") {}
};


template<typename T>
class optional {
private:
    std::unique_ptr<T> value;
public:
      optional() : value() {}

      optional( const optional<T> & v) : 
          value() {
          if(v.value) {
              value.reset(new T(*v.value.get()));
          }
      }

      optional<T>& operator=(const optional<T> & v) {
          if(v.value) {
              value.reset(new T(*v.value.get()));
          } else {
              value.clear();
          }
          return *this;
      }

      optional( optional<T> && v) :
          value(std::move(v.value)) {};

      optional<T>& operator=(optional<T> && v) {
          value=(std::move(v.value));
          return *this;
      }

      explicit optional( const T & v) :
          value( new T(v) ) {}

      explicit optional( T && v) :
          value( new T(std::move(v)) ) {}


      bool hasValue() const {
          return value.get()!=nullptr;
      }

      const T& getValue() const {
          T* v = value.get();
          if(v==nullptr)
              throw MissingOptionalValue();
          return *v;
      }

      T && takeValue() {
        return std::move(*value.release());
      }
};
          


#pragma once

#include <memory>

namespace cxx14 {

template<typename T, typename... Args>
static inline std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>( new T(std::forward<Args>(args)... ));
}

}

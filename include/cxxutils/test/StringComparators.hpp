#pragma once

struct StringStartsWithComparator {

  std::string prefix;

  explicit StringStartsWithComparator(const std::string & prefix) : prefix(prefix) {}

  bool ok(const std::string & input) const {
    return input.substr(0,prefix.size())==prefix;
  }

  std::string describe_failure(const std::string & input) {
    return "Expected string starting '"+prefix+"' but got string starting '"+input.substr(0,prefix.size())+"'";
  }


}
;
struct StringContainsComparator {

  std::string needle;

  explicit StringContainsComparator(const std::string & needle) : needle(needle) {}

  bool ok(const std::string & input) const {
    return input.find(needle)!=std::string::npos;
  }

  std::string describe_failure(const std::string & input) {
    return "Expected string to contain '"+needle+"' but got string '"+input+"'";
  }

};

static inline StringStartsWithComparator startsWith(const std::string & start) {
  return StringStartsWithComparator(start);
}

static inline StringContainsComparator contains(const std::string & needle) { 
    return StringContainsComparator(needle);
}


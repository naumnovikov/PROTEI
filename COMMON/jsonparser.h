#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <string>
#include <string_view>
#include <vector>

#include "common_types.h"

// To fix the problem with shared code,
// which causes including everything in CMakeLists for each of targets,
// I decided to do Facade-pattern

class CommonJsonValidator {
 public:
  bool isIPValid(std::string_view ip) const;
  bool isLocationValid(const position_vector& location) const noexcept;
};

#ifdef BUILD_UE
class UE;

class UEJsonParser {
 private:
  bool validateJSONOnRequiredFiledsForUE(const json& json_file) const;
  bool isIMEIValid(std::string_view imei) const noexcept;
  bool isIMSIValid(std::string_view imsi) const noexcept;

 public:
  void setValuesUE(const json& json_data, UE& ue) const;
};
#endif

#ifdef BUILD_SIMTEL
class SIMTEL;

class SimtelJsonParser {
 private:
  bool validateJSONOnRequiredFiledsForServer(const json& json_file) const;

 public:
  void setValuesSIMTEL(const json& json_data, SIMTEL& simtel) const;
};
#endif

class JSONParser {
 public:
#ifdef BUILD_UE
  void configurateUE(std::string json_filenameParam, UE& ue) const;
#endif
#ifdef BUILD_SIMTEL
  void configurateSIMTEL(std::string json_filenameParam, SIMTEL& simtel) const;
#endif
};

#endif  // JSONPARSER_H
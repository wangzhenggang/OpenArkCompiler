/*
 * Copyright (c) [2020] Huawei Technologies Co.,Ltd.All rights reserved.
 *
 * OpenArkCompiler is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *
 *     http://license.coscl.org.cn/MulanPSL
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
 * FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v1 for more details.
 */
#ifndef MAPLE_UTIL_INCLUDE_OPTION_PARSER_H
#define MAPLE_UTIL_INCLUDE_OPTION_PARSER_H
#include <map>
#include <string>
#include <vector>
#include "error_code.h"

namespace mapleOption {
enum BuildType {
  kBuildTypeAll,
  kBuildTypeDebug,
  kBuildTypeRelease
};

enum ArgCheckPolicy {
  kArgCheckPolicyUnknown,
  kArgCheckPolicyNone,
  kArgCheckPolicyOptional,
  kArgCheckPolicyRequired,
  kArgCheckPolicyNumeric,
  kArgCheckPolicyBool
};

struct ExtraOption {
  // Tool bin name
  const char * const exeName;

  // Key mapping with -/-- if needed.
  const char * const optionKey;

  // Can override the connector defined in Descriptor.
  const char * const connector;

  // Can override the delimiter defined in Descriptor
  const char * const delimiter;
};

constexpr unsigned int kMaxExtraOptions = 10;

struct Descriptor {
  // Unique option index
  const unsigned int index;

  // Unique option type
  const int type;

  // Short form option key
  const char * const shortOption;

  // Long form option key
  const char * const longOption;

#ifdef OPTION_PARSER_EXTRAOPT
  // Connector followed after key when passing args to target tool.
  const char * const connector;

  // There's default args for the option.
  // When user set it again, if true, append to the default args,
  // else replace it.
  const bool isCanAppend;

  // Delimiter when append.
  // if isCanAppend is false, it's meaningless.
  const char * const delimiter;
#endif

  // The option can be seen in which build type.
  const BuildType enableBuildType;

  // Should we have a parameter?
  const ArgCheckPolicy checkPolicy;

  // Help info
  const char * const help;

#ifdef OPTION_PARSER_EXTRAOPT
  const char * const exeName;

  // option key mapping to target tool
  const std::vector<ExtraOption> extras;
#endif

  bool IsEnabledForCurrentBuild() const {
    switch (enableBuildType) {
      case BuildType::kBuildTypeAll:
        return true;
      case BuildType::kBuildTypeDebug:
        return true;
      case BuildType::kBuildTypeRelease:
        return false;
      default:
        // should never reach
        return true;
    }
  }
};

class Option {
 public:
  Option(Descriptor desc, const std::string &optionKey, const std::string &args)
      : descriptor(desc), optionKey(optionKey), args(args) {}

  ~Option() = default;

  unsigned int Index() const {
    return descriptor.index;
  }

  unsigned int Type() const {
    return descriptor.type;
  }

  const std::string &Args() const {
    return args;
  }

  const std::string &OptionKey() const {
    return optionKey;
  }

#ifdef OPTION_PARSER_EXTRAOPT
  std::string ConnectSymbol(const std::string &exeName) const {
    auto extra = FindExtra(exeName);
    if (extra != nullptr && extra->connector != nullptr) {
      return extra->connector;
    }
    return descriptor.connector == nullptr ? "" : descriptor.connector;
  }

  std::string AppendSymbol(const std::string &exeName) const {
    auto extra = FindExtra(exeName);
    if (extra != nullptr && extra->delimiter != nullptr) {
      return extra->delimiter;
    }
    return descriptor.delimiter == nullptr ? "" : descriptor.delimiter;
  }

  bool HasExtra() const {
    return descriptor.extras[0].exeName != nullptr;
  }

  std::vector<ExtraOption> GetExtras() const {
    auto ret = std::vector<ExtraOption>();
    unsigned int index = 0;
    while (index < kMaxExtraOptions && descriptor.extras[index].exeName != nullptr) {
      ret.push_back(descriptor.extras[index++]);
    }
    return ret;
  }

  const ExtraOption *FindExtra(const std::string &exeName) const {
    unsigned int index = 0;
    while (index < kMaxExtraOptions && descriptor.extras[index].exeName != nullptr) {
      if (exeName == descriptor.extras[index].exeName) {
        return &descriptor.extras[index];
      }
      ++index;
    }
    return nullptr;
  }

#endif

 private:
  Descriptor descriptor;
  const std::string optionKey;
  const std::string args;
};

class OptionParser {
 public:
  explicit OptionParser(const Descriptor usage[]);

  ~OptionParser() = default;

  maple::ErrorCode Parse(int argc, char **argv);

  maple::ErrorCode HandleInputArgs(const std::vector<std::string> &inputArgs, const std::string &exeName,
                                   std::vector<mapleOption::Option> &inputOption);

  const std::vector<Option> &GetOptions() const {
    return options;
  }

  const std::vector<std::string> &GetNonOptions() const {
    return nonOptionsArgs;
  }

  int GetNonOptionsCount() const {
    return nonOptionsArgs.size();
  }

#ifdef OPTION_PARSER_EXTRAOPT
  void InsertExtraUsage(const Descriptor &usage);
#endif

  void CreateNoOption(const Descriptor &usage);

  void PrintUsage() const;

#ifdef OPTION_PARSER_EXTRAOPT
  bool SetOption(const std::string &key, const std::string &value, const std::string &exeName,
                 std::vector<mapleOption::Option> &exeOption);
  void PrintUsage(const std::string &helpTyoe) const;
#endif

 private:
  bool HandleKeyValue(const std::string &key, const std::string &value, bool isValueEmpty,
                      std::vector<mapleOption::Option> &inputOption, const std::string &exeName,
                      bool isAllOption = true);
  bool CheckOpt(const std::string option, std::string &lastKey, bool &isLastMatch,
                std::vector<mapleOption::Option> &inputOption, const std::string &exeName);
  void InsertOption(const std::string &opt, const Descriptor &usage) {
    if (usage.IsEnabledForCurrentBuild()) {
      usages.insert(make_pair(opt, usage));
    }
  }
  bool CheckSpecialOption(const std::string &option, std::string &key, std::string &value);
  std::vector<Descriptor> rawUsages;
  std::multimap<std::string, Descriptor> usages;
  std::vector<Option> options;
  std::vector<std::string> nonOptionsArgs;
};
enum MatchedIndex {
  kMatchNone,
  kMatchShortOpt,
  kMatchLongOpt
};
enum EnabledIndex {
  kDisable,
  kEnable
};
}  // namespace mapleOption

#endif  // MAPLE_UTIL_INCLUDE_OPTION_PARSER_H

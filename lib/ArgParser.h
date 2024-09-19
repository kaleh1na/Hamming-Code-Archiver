#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

namespace ArgumentParser {

class ArgParser {
 private:
  class IntArgument {
   public:
    IntArgument(const char short_arg, const std::string& long_arg,
                const std::string& description);

    IntArgument& Positional();

    IntArgument& MultiValue(size_t count = 0);

    IntArgument& Default(int value);

    IntArgument& StoreValue(int& value);

    IntArgument& StoreValues(std::vector<int>& values);

    void CreateHelp();

    char short_name;
    std::string long_name;
    std::string def;
    std::string help = "";
    bool is_filled = false;

    int arg;
    int* store_arg = nullptr;
    std::vector<int> args;
    std::vector<int>* store_args = nullptr;

    bool is_positional = false;

    bool is_repeated = false;
    size_t min_count;

    int default_value;
    bool has_default = false;
    size_t size = 0;
  };

  class StringArgument {
   public:
    StringArgument(const char short_arg, const std::string& long_arg,
                   const std::string& description);

    StringArgument& Positional();

    StringArgument& MultiValue(size_t count = 0);

    StringArgument& Default(const std::string& value);

    StringArgument& StoreValue(std::string& value);

    StringArgument& StoreValues(std::vector<std::string>& values);

    void CreateHelp();

    char short_name;
    std::string long_name;
    std::string def;
    std::string help = "";
    bool is_filled = false;

    std::string arg;
    std::string* store_arg = nullptr;
    std::vector<std::string> args;
    std::vector<std::string>* store_args = nullptr;

    bool is_repeated = false;
    size_t min_count = 0;

    bool is_positional = false;

    std::string default_value;
    bool has_default = false;
    size_t size = 0;
  };

  class FlagArgument {
   public:
    FlagArgument(const char short_arg, const std::string& long_arg,
                 const std::string& description);

    FlagArgument& Default(const bool value);

    FlagArgument& StoreValue(bool& value);

    void CreateHelp();

    char short_name;
    std::string long_name;
    std::string def;
    std::string help = "";

    bool arg = false;
    bool* store_arg = nullptr;

    bool has_default = false;
    bool default_value;
  };

  class HelpArgument {
   public:
    HelpArgument(const char short_arg, const std::string& long_arg,
                 const std::string& description);

    void CreateHelp();

    char short_name;
    std::string long_name;
    std::string def;
    std::string help = "";
  };

  std::string name_;
  std::vector<IntArgument> int_args_;
  std::vector<StringArgument> string_args_;
  std::vector<FlagArgument> flag_args_;
  std::vector<HelpArgument> help_args_;

  bool need_help_ = false;
  std::string help_ = "";

 public:
  ArgParser(const std::string_view& name);

  bool Parse(const std::vector<std::string>& argv);

  bool Parse(const int argc, char** argv);

  IntArgument& AddIntArgument(const char short_arg = ' ',
                              const std::string& long_arg = "",
                              const std::string def = "");

  IntArgument& AddIntArgument(const std::string& long_arg = "",
                              const std::string def = "");

  StringArgument& AddStringArgument(const char short_arg = ' ',
                                    const std::string& long_arg = "",
                                    const std::string def = "");

  StringArgument& AddStringArgument(const std::string& long_arg = "",
                                    const std::string def = "");

  FlagArgument& AddFlag(const char short_arg = ' ',
                        const std::string& long_arg = "",
                        const std::string def = "");

  FlagArgument& AddFlag(const std::string& long_arg = "",
                        const std::string def = "");

  void AddHelp(const char short_arg = ' ', const std::string& long_arg = "",
               const std::string def = "");

  void AddHelp(const std::string& long_arg = "", const std::string def = "");

  //

  int GetIntValue(const char arg_name, const int ind = -1);

  int GetIntValue(const std::string& arg_name, const int ind = -1);

  std::string GetStringValue(const char arg_name, const int ind = -1);

  std::string GetStringValue(const std::string& arg_name, const int ind = -1);

  bool GetFlag(const char arg_name);

  bool GetFlag(const std::string& arg_name);

  //

  bool Help();

  std::string& HelpDescription();

  void CreateHelp();
};

}  // namespace ArgumentParser
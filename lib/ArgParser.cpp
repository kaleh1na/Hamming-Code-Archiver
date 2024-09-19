#include "ArgParser.h"
using namespace ArgumentParser;

bool ArgParser::Parse(const std::vector<std::string>& argv) {
  std::string arg;
  for (size_t j = 1; j < argv.size(); ++j) {
    arg = argv[j];
    if (arg.size() > 1 && arg[0] == '-' && arg[1] != '-' && !isdigit(arg[1])) {
      bool is_processed = false;
      for (size_t i = 0; i < help_args_.size(); ++i) {
        if (help_args_[i].short_name == arg[1] && arg.size() == 2) {
          is_processed = true;
          need_help_ = true;
          break;
        }
      }
      if (!is_processed) {
        for (size_t i = 0; i < string_args_.size(); ++i) {
          if (string_args_[i].short_name == arg[1] && arg.size() > 3 &&
              arg[2] == '=') {
            is_processed = true;
            std::string value = arg.substr(3);
            if (!string_args_[i].is_repeated) {
              string_args_[i].arg = value;
              string_args_[i].is_filled = true;
            } else {
              if (string_args_[i].size < string_args_[i].args.size()) {
                string_args_[i].args[string_args_[i].size] = value;
              } else {
                string_args_[i].args.push_back(value);
              }
              if (string_args_[i].args.size() >= string_args_[i].min_count) {
                string_args_[i].is_filled = true;
              }
              string_args_[i].size++;
            }
            break;
          }
        }
      }
      if (!is_processed) {
        for (size_t i = 0; i < int_args_.size(); ++i) {
          if (int_args_[i].short_name == arg[1] && arg.size() > 3 &&
              arg[2] == '=') {
            is_processed = true;
            std::string num = arg.substr(3);
            const char* val = num.c_str();
            errno = 0;
            char* arg_end{};
            uint64_t value = strtoll(val, &arg_end, 10);
            if ((errno == ERANGE) || (arg_end != val + strlen(val))) {
              return false;
            }
            if (!int_args_[i].is_repeated) {
              int_args_[i].arg = value;
              int_args_[i].is_filled = true;
            } else {
              if (int_args_[i].size < int_args_[i].args.size()) {
                int_args_[i].args[int_args_[i].size] = value;
              } else {
                int_args_[i].args.push_back(value);
              }
              if (int_args_[i].args.size() >= int_args_[i].min_count) {
                int_args_[i].is_filled = true;
              }
              int_args_[i].size++;
            }
            break;
          }
        }
      }
      if (!is_processed) {
        bool is_marked;
        for (size_t k = 1; k < arg.size(); ++k) {
          is_marked = false;
          for (size_t i = 0; i < flag_args_.size(); ++i) {
            if (flag_args_[i].short_name == arg[k]) {
              flag_args_[i].arg = true;
              is_marked = true;
              break;
            }
          }
          if (!is_marked) {
            return false;
          }
        }
        if (is_marked) {
          is_processed = true;
        }
      }
      if (!is_processed) {
        return false;
      }
    } else if (arg.size() > 1 && arg[0] == '-' && arg[1] == '-') {
      bool is_processed = false;
      for (size_t i = 0; i < help_args_.size(); ++i) {
        if (help_args_[i].long_name == arg.substr(2)) {
          is_processed = true;
          need_help_ = true;
          break;
        }
      }
      if (!is_processed) {
        for (size_t i = 0; i < string_args_.size(); ++i) {
          if (string_args_[i].long_name ==
                  arg.substr(2, string_args_[i].long_name.size()) &&
              arg.size() > 3 + string_args_[i].long_name.size() &&
              arg[2 + string_args_[i].long_name.size()] == '=') {
            is_processed = true;
            std::string value =
                arg.substr(3 + string_args_[i].long_name.size());
            if (!string_args_[i].is_repeated) {
              string_args_[i].arg = value;
              string_args_[i].is_filled = true;
            } else {
              if (string_args_[i].size < string_args_[i].args.size()) {
                string_args_[i].args[string_args_[i].size] = value;
              } else {
                string_args_[i].args.push_back(value);
              }
              if (string_args_[i].args.size() >= string_args_[i].min_count) {
                string_args_[i].is_filled = true;
              }
              string_args_[i].size++;
            }
            break;
          }
        }
      }
      if (!is_processed) {
        for (size_t i = 0; i < int_args_.size(); ++i) {
          if (int_args_[i].long_name ==
                  arg.substr(2, int_args_[i].long_name.size()) &&
              arg.size() > 3 + int_args_[i].long_name.size() &&
              arg[2 + int_args_[i].long_name.size()] == '=') {
            is_processed = true;
            std::string num = arg.substr(3 + int_args_[i].long_name.size());
            const char* val = num.c_str();
            errno = 0;
            char* arg_end{};
            uint64_t value = strtoll(val, &arg_end, 10);
            if ((errno == ERANGE) || (arg_end != val + strlen(val))) {
              return false;
            }
            if (!int_args_[i].is_repeated) {
              int_args_[i].arg = value;
              int_args_[i].is_filled = true;
            } else {
              if (int_args_[i].size < int_args_[i].args.size()) {
                int_args_[i].args[int_args_[i].size] = value;
              } else {
                int_args_[i].args.push_back(value);
              }
              if (int_args_[i].args.size() >= int_args_[i].min_count) {
                int_args_[i].is_filled = true;
              }
              int_args_[i].size++;
            }
            break;
          }
        }
      }
      if (!is_processed) {
        for (size_t i = 0; i < flag_args_.size(); ++i) {
          if (flag_args_[i].long_name ==
                  arg.substr(2, flag_args_[i].long_name.size()) &&
              arg.size() == 2 + flag_args_[i].long_name.size()) {
            flag_args_[i].arg = true;
            is_processed = true;
            break;
          }
        }
      }
      if (!is_processed) {
        return false;
      }
    } else {
      bool is_processed = false;
      for (size_t i = 0; i < string_args_.size(); ++i) {
        if (string_args_[i].is_positional && !string_args_[i].is_repeated &&
            !string_args_[i].is_filled) {
          string_args_[i].arg = arg;
          string_args_[i].is_filled = true;
          is_processed = true;
          break;
        } else if (string_args_[i].is_positional &&
                   string_args_[i].is_repeated) {
          if (string_args_[i].size < string_args_[i].args.size()) {
            string_args_[i].args[string_args_[i].size] = arg;
          } else {
            string_args_[i].args.push_back(arg);
          }
          if (string_args_[i].args.size() >= string_args_[i].min_count) {
            string_args_[i].is_filled = true;
          }
          string_args_[i].size++;
          is_processed = true;
          break;
        }
      }
      if (!is_processed) {
        for (size_t i = 0; i < int_args_.size(); ++i) {
          if (int_args_[i].is_positional && !int_args_[i].is_repeated &&
              !int_args_[i].is_filled) {
            const char* val = arg.c_str();
            errno = 0;
            char* arg_end{};
            uint64_t value = strtoll(val, &arg_end, 10);
            if ((errno == ERANGE) || (arg_end != val + strlen(val))) {
              return false;
            }
            int_args_[i].arg = value;
            int_args_[i].is_filled = true;
            is_processed = true;
            break;
          } else if (int_args_[i].is_positional && int_args_[i].is_repeated) {
            const char* val = arg.c_str();
            errno = 0;
            char* arg_end{};
            uint64_t value = strtoll(val, &arg_end, 10);
            if ((errno == ERANGE) || (arg_end != val + strlen(val))) {
              return false;
            }
            if (int_args_[i].size < int_args_[i].args.size()) {
              int_args_[i].args[int_args_[i].size] = value;
            } else {
              int_args_[i].args.push_back(value);
            }
            if (int_args_[i].args.size() >= int_args_[i].min_count) {
              int_args_[i].is_filled = true;
            }
            int_args_[i].size++;
            is_processed = true;
            break;
          }
        }
      }
      if (!is_processed) {
        return false;
      }
    }
  }
  if (need_help_) {
    return true;
  }
  for (size_t i = 0; i < string_args_.size(); ++i) {
    if (!string_args_[i].is_filled) {
      return false;
    }
    if (!string_args_[i].is_repeated) {
      if (string_args_[i].store_arg != nullptr) {
        *string_args_[i].store_arg = string_args_[i].arg;
      }
    } else {
      if (string_args_[i].store_args != nullptr) {
        (*string_args_[i].store_args).clear();
        for (size_t j = 0; j < string_args_[i].args.size(); ++j) {
          (*string_args_[i].store_args).push_back(string_args_[i].args[j]);
        }
      }
    }
  }
  for (size_t i = 0; i < int_args_.size(); ++i) {
    if (!int_args_[i].is_filled) {
      return false;
    }
    if (!int_args_[i].is_repeated) {
      if (int_args_[i].store_arg != nullptr) {
        *int_args_[i].store_arg = int_args_[i].arg;
      }
    } else {
      if (int_args_[i].store_args != nullptr) {
        (*int_args_[i].store_args).clear();
        for (size_t j = 0; j < int_args_[i].args.size(); ++j) {
          (*int_args_[i].store_args).push_back(int_args_[i].args[j]);
        }
      }
    }
  }
  for (size_t i = 0; i < flag_args_.size(); ++i) {
    if (flag_args_[i].store_arg != nullptr) {
      *flag_args_[i].store_arg = flag_args_[i].arg;
    }
  }
  return true;
}

bool ArgParser::Parse(const int argc, char** argv) {
  return Parse(std::vector<std::string>(argv, argv + argc));
}

//

ArgParser::IntArgument::IntArgument(const char short_arg,
                                    const std::string& long_arg,
                                    const std::string& description) {
  short_name = short_arg;
  long_name = long_arg;
  def = description;
}

ArgParser::IntArgument& ArgParser::IntArgument::Positional() {
  is_positional = true;
  return *this;
}

ArgParser::IntArgument& ArgParser::IntArgument::MultiValue(size_t count) {
  is_repeated = true;
  min_count = count;
  return *this;
}

ArgParser::IntArgument& ArgParser::IntArgument::Default(int value) {
  has_default = true;
  is_filled = true;
  default_value = value;
  if (is_repeated) {
    for (size_t i = 0; i < min_count; ++i) {
      args.push_back(value);
    }
    if (store_args != nullptr) {
      for (size_t i = 0; i < min_count; ++i) {
        (*store_args).push_back(value);
      }
    }
  } else {
    arg = value;
    if (store_arg != nullptr) {
      *store_arg = value;
    }
  }
  return *this;
}

ArgParser::IntArgument& ArgParser::IntArgument::StoreValue(int& value) {
  store_arg = &value;
  return *this;
}

ArgParser::IntArgument& ArgParser::IntArgument::StoreValues(
    std::vector<int>& values) {
  store_args = &values;
  return *this;
}

void ArgParser::IntArgument::CreateHelp() {
  if (short_name != ' ') {
    help += "-" + std::string(1, short_name);
    if (long_name != "" || def != "") {
      help += ",  ";
    }
  } else if (long_name != "" || def != "") {
    help += "     ";
  }
  if (long_name != "") {
    help += "--" + long_name + "=<int>";
    if (def != "") {
      help += ",  ";
    }
  }
  if (def != "") {
    help += def;
  }
  if (is_positional) {
    help += " [positional]";
  }
  if (is_repeated) {
    help += " [repeated";
    if (min_count == 0) {
      help += "]";
    } else {
      help += ", min args = " + std::to_string(min_count) + "]";
    }
  }
  if (has_default) {
    help += " [default = " + std::to_string(default_value) + "]";
  }
}

//

ArgParser::StringArgument::StringArgument(const char short_arg,
                                          const std::string& long_arg,
                                          const std::string& description) {
  short_name = short_arg;
  long_name = long_arg;
  def = description;
}

ArgParser::StringArgument& ArgParser::StringArgument::Positional() {
  is_positional = true;
  return *this;
}

ArgParser::StringArgument& ArgParser::StringArgument::MultiValue(size_t count) {
  is_repeated = true;
  min_count = count;
  if (count == 0) {
    is_filled = true;
  }
  return *this;
}

ArgParser::StringArgument& ArgParser::StringArgument::Default(
    const std::string& value) {
  has_default = true;
  is_filled = true;
  default_value = value;
  if (is_repeated) {
    for (size_t i = 0; i < min_count; ++i) {
      args.push_back(value);
    }
    if (store_args != nullptr) {
      for (size_t i = 0; i < min_count; ++i) {
        (*store_args).push_back(value);
      }
    }
  } else {
    arg = value;
    if (store_arg != nullptr) {
      *store_arg = value;
    }
  }
  return *this;
}

ArgParser::StringArgument& ArgParser::StringArgument::StoreValue(
    std::string& value) {
  store_arg = &value;
  return *this;
}

ArgParser::StringArgument& ArgParser::StringArgument::StoreValues(
    std::vector<std::string>& values) {
  store_args = &values;
  return *this;
}

void ArgParser::StringArgument::CreateHelp() {
  if (short_name != ' ') {
    help += "-" + std::string(1, short_name);
    if (long_name != "" || def != "") {
      help += ",  ";
    }
  } else if (long_name != "" || def != "") {
    help += "     ";
  }
  if (long_name != "") {
    help += "--" + long_name + "=<string>";
    if (def != "") {
      help += ",  ";
    }
  }
  if (def != "") {
    help += def;
  }
  if (is_positional) {
    help += " [positional]";
  }
  if (is_repeated) {
    help += " [repeated";
    if (min_count == 0) {
      help += "]";
    } else {
      help += ", min args = " + std::to_string(min_count) + "]";
    }
  }
  if (has_default) {
    help += " [default = " + default_value + "]";
  }
}

//

ArgParser::FlagArgument::FlagArgument(const char short_arg,
                                      const std::string& long_arg,
                                      const std::string& description) {
  short_name = short_arg;
  long_name = long_arg;
  def = description;
}

ArgParser::FlagArgument& ArgParser::FlagArgument::StoreValue(bool& value) {
  store_arg = &value;
  *store_arg = arg;
  return *this;
}

ArgParser::FlagArgument& ArgParser::FlagArgument::Default(const bool value) {
  has_default = true;
  default_value = value;
  arg = value;
  if (store_arg != nullptr) {
    *store_arg = value;
  }
  return *this;
}

void ArgParser::FlagArgument::CreateHelp() {
  if (short_name != ' ') {
    help += "-" + std::string(1, short_name);
    if (long_name != "" || def != "") {
      help += ",  ";
    }
  } else if (long_name != "" || def != "") {
    help += "     ";
  }
  if (long_name != "") {
    help += "--" + long_name;
    if (def != "") {
      help += ",  ";
    }
  }
  if (def != "") {
    help += def;
  }
  if (has_default) {
    if (default_value) {
      help += " [default = true]";
    } else {
      help += " [default = false]";
    }
  }
}

//

ArgumentParser::ArgParser::ArgParser(const std::string_view& name) {
  name_ = name;
}

ArgParser::IntArgument& ArgParser::AddIntArgument(const std::string& long_arg,
                                                  const std::string def) {
  IntArgument arg(' ', long_arg, def);
  int_args_.push_back(arg);
  return int_args_.back();
}

ArgParser::IntArgument& ArgParser::AddIntArgument(const char short_arg,
                                                  const std::string& long_arg,
                                                  const std::string def) {
  IntArgument arg(short_arg, long_arg, def);
  int_args_.push_back(arg);
  return int_args_.back();
}

ArgParser::StringArgument& ArgParser::AddStringArgument(
    const std::string& long_arg, const std::string def) {
  StringArgument arg(' ', long_arg, def);
  string_args_.push_back(arg);
  return string_args_.back();
}

ArgParser::StringArgument& ArgParser::AddStringArgument(
    const char short_arg, const std::string& long_arg, const std::string def) {
  StringArgument arg(short_arg, long_arg, def);
  string_args_.push_back(arg);
  return string_args_.back();
}

ArgParser::FlagArgument& ArgParser::AddFlag(const std::string& long_arg,
                                            const std::string def) {
  FlagArgument arg(' ', long_arg, def);
  flag_args_.push_back(arg);
  return flag_args_.back();
}

ArgParser::FlagArgument& ArgParser::AddFlag(const char short_arg,
                                            const std::string& long_arg,
                                            const std::string def) {
  FlagArgument arg(short_arg, long_arg, def);
  flag_args_.push_back(arg);
  return flag_args_.back();
}

void ArgParser::AddHelp(const std::string& long_arg, const std::string def) {
  HelpArgument arg(' ', long_arg, def);
  help_args_.push_back(arg);
}

void ArgParser::AddHelp(const char short_arg, const std::string& long_arg,
                        const std::string def) {
  HelpArgument arg(short_arg, long_arg, def);
  help_args_.push_back(arg);
}

//

int ArgParser::GetIntValue(const char arg_name, const int ind) {
  for (size_t i = 0; i < int_args_.size(); ++i) {
    if (int_args_[i].short_name == arg_name) {
      if (ind == -1) {
        return int_args_[i].arg;
      }
      return int_args_[i].args[ind];
    }
  }
  return 0;
}

int ArgParser::GetIntValue(const std::string& arg_name, const int ind) {
  for (size_t i = 0; i < int_args_.size(); ++i) {
    if (int_args_[i].long_name == arg_name) {
      if (ind == -1) {
        return int_args_[i].arg;
      }
      return int_args_[i].args[ind];
    }
  }
  return 0;
}

std::string ArgParser::GetStringValue(const char arg_name, const int ind) {
  for (size_t i = 0; i < string_args_.size(); ++i) {
    if (string_args_[i].short_name == arg_name) {
      if (ind == -1) {
        return string_args_[i].arg;
      }
      return string_args_[i].args[ind];
    }
  }
  return "";
}

std::string ArgParser::GetStringValue(const std::string& arg_name,
                                      const int ind) {
  for (size_t i = 0; i < string_args_.size(); ++i) {
    if (string_args_[i].long_name == arg_name) {
      if (ind == -1) {
        return string_args_[i].arg;
      }
      return string_args_[i].args[ind];
    }
  }
  return "";
}

bool ArgParser::GetFlag(const char arg_name) {
  for (size_t i = 0; i < flag_args_.size(); ++i) {
    if (flag_args_[i].short_name == arg_name) {
      return flag_args_[i].arg;
    }
  }
  return false;
}

bool ArgParser::GetFlag(const std::string& arg_name) {
  for (size_t i = 0; i < flag_args_.size(); ++i) {
    if (flag_args_[i].long_name == arg_name) {
      return flag_args_[i].arg;
    }
  }
  return false;
}

//

bool ArgParser::Help() { return need_help_; }

std::string& ArgParser::HelpDescription() {
  CreateHelp();
  return help_;
}

ArgParser::HelpArgument::HelpArgument(const char short_arg,
                                      const std::string& long_arg,
                                      const std::string& description) {
  short_name = short_arg;
  long_name = long_arg;
  def = description;
}

void ArgParser::HelpArgument::CreateHelp() {
  if (short_name != ' ') {
    help += "-" + std::string(1, short_name);
    if (long_name != "" || def != "") {
      help += ",  ";
    }
  } else if (long_name != "" || def != "") {
    help += "     ";
  }
  if (long_name != "") {
    help += "--" + long_name;
    if (def != "") {
      help += ",  ";
    }
  }
  if (def != "") {
    help += def;
  }
}

void ArgParser::CreateHelp() {
  help_ += name_ + "\n";
  help_ += "\n";

  for (size_t i = 0; i < int_args_.size(); ++i) {
    int_args_[i].CreateHelp();
    help_ += int_args_[i].help;
    help_ += "\n";
  }
  for (size_t i = 0; i < string_args_.size(); ++i) {
    string_args_[i].CreateHelp();
    help_ += string_args_[i].help;
    help_ += "\n";
  }
  for (size_t i = 0; i < flag_args_.size(); ++i) {
    flag_args_[i].CreateHelp();
    help_ += flag_args_[i].help;
    help_ += "\n";
  }
  for (size_t i = 0; i < help_args_.size(); ++i) {
    help_args_[i].CreateHelp();
    help_ += help_args_[i].help;
    help_ += "\n";
  }
}
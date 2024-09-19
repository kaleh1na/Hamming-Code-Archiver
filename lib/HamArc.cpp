#include "HamArc.h"

bool CheckFileExistence(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    return false;
  }
  file.close();
  return true;
}

bool Parse(int argc, char **argv, Options &opt) {
  ArgParser parser("HamArcParser");
  parser.AddHelp('h', "help", "description about program");
  parser.AddFlag('c', "create", "create new archive")
      .StoreValue(opt.need_creation);
  parser.AddStringArgument('f', "file", "name of archive file")
      .StoreValue(opt.filename);
  parser.AddFlag('l', "list", "shows a list of files in the archive")
      .StoreValue(opt.need_list);
  parser
      .AddFlag('x', "extract",
               "extracts file from archive(all, if not specified)")
      .StoreValue(opt.need_extraction);
  parser.AddFlag('d', "delete", "deletes file from archive")
      .StoreValue(opt.need_deletion);
  parser.AddFlag('a', "append", "add file to rhe archive")
      .StoreValue(opt.need_addition);
  parser.AddFlag('A', "concatenate", "merge 2 archives")
      .StoreValue(opt.need_concatenation);
  parser.AddStringArgument("files", "names of files for work")
      .MultiValue()
      .Positional()
      .StoreValues(opt.filenames);
  parser.AddIntArgument('s', "size", "size of block in hamming code")
      .StoreValue(opt.block_size)
      .Default(16);
  if (!parser.Parse(argc, argv)) {
    std::cout << "Wrong argument" << '\n';
    std::cout << parser.HelpDescription();
    return false;
  }
  if (parser.Help()) {
    std::cout << parser.HelpDescription();
  }
  if (!(opt.need_list || opt.need_extraction) && opt.filenames.size() == 0) {
    return false;
  }
  if (opt.need_addition || opt.need_deletion || opt.need_extraction ||
      opt.need_list) {
    if (!CheckFileExistence(opt.filename)) {
      return false;
    }
  }
  if (opt.need_creation || opt.need_addition || opt.need_concatenation) {
    for (int i = 0; i < opt.filenames.size(); ++i) {
      if (!CheckFileExistence(opt.filenames[i])) {
        return false;
      }
    }
  }
  return true;
}

//

size_t CountEncodedBlockSize(const size_t block_size) {
  size_t i = 0;
  size_t j = 0;
  size_t k = 1;
  while (i < block_size) {
    if ((j + 1) == k) {
      k *= 2;
    } else {
      ++i;
    }
    ++j;
  }
  return j;
}

size_t CountBlockSize(const size_t encoded_block_size) {
  size_t i = 0;
  size_t j = 0;
  size_t k = 1;
  while (i < encoded_block_size) {
    if ((j + 1) == k) {
      k *= 2;
    } else {
      ++j;
    }
    ++i;
  }
  return j;
}

size_t CountEncodedSize(const size_t file_size, const size_t block_size) {
  size_t full_blocks = file_size / block_size;
  size_t part_block_len = file_size % block_size;
  size_t size = full_blocks * CountEncodedBlockSize(block_size * 8) +
                CountEncodedBlockSize(part_block_len * 8);
  size = (size + 7) / 8;
  return size;
}

size_t GetFileSize(const std::string &filename) {
  std::ifstream file(filename, std::ios::binary);
  file.seekg(0, std::ios::end);
  size_t file_size = file.tellg();
  file.close();
  return file_size;
}

//

size_t HamCode::FindMistake() const {
  int ind_mistake = 0;
  for (int i = 1; i < encoded_data_.size() + 1; i *= 2) {
    int control_bit = 0;
    int j = i;
    while (j < encoded_data_.size()) {
      if ((j + 1) % i == 0) {
        j += i;
      }
      if (j >= encoded_data_.size()) {
        break;
      }
      control_bit ^= encoded_data_[j];
      ++j;
    }
    if (control_bit != encoded_data_[i - 1]) {
      ind_mistake += i;
    }
  }
  return ind_mistake;
}

HamCode::HamCode(uint8_t bytes[], const size_t block_size) {
  for (int i = 0; i < block_size; ++i) {
    uint8_t item = bytes[i];
    for (int j = 128; j >= 1; j = j / 2) {
      data_.push_back(item / j);
      item %= j;
    }
  }
}

HamCode::HamCode(Block &block, const size_t block_size) {
  size_t encoded_block_size = CountEncodedBlockSize(block_size * 8);
  size_t ind = 0;
  uint8_t item = block.bytes[ind];
  uint8_t x = 128;
  for (int i = 0; i < block.bit; ++i) {
    item %= x;
    x /= 2;
  }
  for (int i = 0; i < encoded_block_size; ++i) {
    encoded_data_.push_back(item / x);
    item %= x;
    x /= 2;
    if (x == 0) {
      x = 128;
      if (ind + 1 < block.bytes.size()) {
        ++ind;
        item = block.bytes[ind];
      }
    }
  }
  block.byte = block.bytes[ind];
  size_t new_bit = 0;
  while (x != 128) {
    x *= 2;
    new_bit++;
  }
  block.bit = new_bit;
}

int HamCode::GetIndMistake() const { return ind_fixed_bit_; }

bool HamCode::FixMistake() {
  size_t ind_mistake = FindMistake();
  if (ind_mistake != 0) {
    encoded_data_[ind_mistake - 1] = encoded_data_[ind_mistake - 1] ^ 1;
    ind_fixed_bit_ = ind_mistake - 1;
  }
  if (FindMistake() != 0) {
    return false;
  }
  return true;
}

void HamCode::Coding() {
  size_t i = 0;
  size_t j = 1;
  size_t k = 1;
  while (i < data_.size()) {
    if (j == k) {
      encoded_data_.push_back(0);
      k *= 2;
    } else {
      encoded_data_.push_back(data_[i]);
      ++i;
    }
    ++j;
  }
  for (int i = 1; i < encoded_data_.size() + 1; i *= 2) {
    int j = i + 1;
    while (j <= encoded_data_.size()) {
      if (j % i == 0) {
        j += i;
      }
      if (j > encoded_data_.size()) {
        break;
      }
      encoded_data_[i - 1] = encoded_data_[i - 1] ^ encoded_data_[j - 1];
      ++j;
    }
  }
}

void HamCode::Decoding() {
  int i = 1;
  for (int j = 0; j < encoded_data_.size(); ++j) {
    if (j + 1 == i) {
      i *= 2;
    } else {
      data_.push_back(encoded_data_[j]);
    }
  }
}

void HamCode::FillBytes(Block &block) const {
  uint8_t x = 128;
  for (int i = 0; i < block.bit; ++i) {
    x /= 2;
  }
  for (int i = 0; i < encoded_data_.size(); ++i) {
    if (encoded_data_[i] == 1) {
      block.byte += x;
    }
    x /= 2;
    if (x == 0) {
      block.bytes.push_back(block.byte);
      block.byte = 0;
      x = 128;
    }
  }
  size_t new_bit = 0;
  while (x != 128) {
    x *= 2;
    new_bit++;
  }
  block.bit = new_bit;
}

void HamCode::GetBytes(uint8_t bytes[]) const {
  size_t x = 128;
  size_t ind = 0;
  bytes[ind] = 0;
  for (int i = 0; i < data_.size(); ++i) {
    if (data_[i]) {
      bytes[ind] += x;
    }
    x /= 2;
    if (x == 0) {
      x = 128;
      if (ind + 1 < data_.size() / 8) {
        ++ind;
        bytes[ind] = 0;
      }
    }
  }
}

//

void FillBlock(std::ofstream &arc, uint8_t buffer[], size_t block_size,
               Block &block) {
  HamCode code(buffer, block_size);
  code.Coding();
  code.FillBytes(block);
  for (int k = 0; k < block.bytes.size(); ++k) {
    arc << block.bytes[k];
  }
  block.bytes.clear();
}

void SizeCoding(std::ofstream &arc, const Options &opt, const size_t i) {
  Block block;
  size_t file_size = GetFileSize(opt.filenames[i]);
  size_t encoded_file_size = CountEncodedSize(file_size, opt.block_size);
  uint8_t size[4];
  size[0] = encoded_file_size % 256;
  size[1] = (encoded_file_size >> 8) % 256;
  size[2] = ((encoded_file_size >> 8) >> 8) % 256;
  size[3] = (((encoded_file_size >> 8) >> 8) >> 8) % 256;
  uint8_t size_buffer[opt.block_size];
  size_t ind = 0;
  for (int j = 0; j < 4 / opt.block_size; ++j) {
    for (int k = 0; k < opt.block_size; ++k) {
      size_buffer[k] = size[ind];
      ++ind;
    }
    FillBlock(arc, size_buffer, opt.block_size, block);
  }
  if (4 % opt.block_size != 0) {
    uint8_t size_buffer[4 % opt.block_size];
    for (int k = 0; k < 4 % opt.block_size; ++k) {
      size_buffer[k] = size[ind];
      ++ind;
    }
    FillBlock(arc, size_buffer, 4 % opt.block_size, block);
  }
  if (block.bit != 0) {
    arc << block.byte;
  }
}

void FileCoding(std::ofstream &arc, const Options &opt, const size_t i) {
  Block block;
  size_t file_size = GetFileSize(opt.filenames[i]);
  std::ifstream file;
  file.open(opt.filenames[i], std::ios::binary);
  uint8_t buffer[opt.block_size];
  for (int j = 0; j < file_size / opt.block_size; ++j) {
    file.read(reinterpret_cast<char *>(&buffer), opt.block_size);
    FillBlock(arc, buffer, opt.block_size, block);
  }
  if (file_size % opt.block_size != 0) {
    uint8_t buffer[file_size % opt.block_size];
    file.read(reinterpret_cast<char *>(&buffer), file_size % opt.block_size);
    FillBlock(arc, buffer, file_size % opt.block_size, block);
  }
  if (block.bit != 0) {
    arc << block.byte;
  }
  file.close();
}

void NameCoding(std::ofstream &arc, const Options &opt, const size_t i) {
  size_t name_size = CountEncodedSize(opt.filenames[i].size(), opt.block_size);
  Block block;
  uint8_t name_size_buffer[1];
  name_size_buffer[0] = name_size;
  FillBlock(arc, name_size_buffer, 1, block);
  if (block.bit != 0) {
    arc << block.byte;
  }
  block.bit = 0;
  block.byte = 0;
  std::vector<uint8_t> name;
  for (int j = 0; j < opt.filenames[i].size(); ++j) {
    name.push_back(opt.filenames[i][j]);
  }
  size_t ind = 0;
  uint8_t name_buffer[opt.block_size];
  for (int j = 0; j < name.size() / opt.block_size; ++j) {
    for (int k = 0; k < opt.block_size; ++k) {
      name_buffer[k] = name[ind];
      ++ind;
    }
    FillBlock(arc, name_buffer, opt.block_size, block);
  }
  if (name.size() % opt.block_size != 0) {
    uint8_t name_buffer[name.size() % opt.block_size];
    for (int k = 0; k < name.size() % opt.block_size; ++k) {
      name_buffer[k] = name[ind];
      ++ind;
    }
    FillBlock(arc, name_buffer, name.size() % opt.block_size, block);
  }
  if (block.bit != 0) {
    arc << block.byte;
  }
}

//

void BlockDecoding(Block &block, size_t block_size, uint8_t bytes[]) {
  HamCode code(block, block_size);
  code.Decoding();
  code.GetBytes(bytes);
  size_t last_ind = block.indexes[block.indexes.size() - 1];
  block.bytes.clear();
  block.indexes.clear();
  if (block.bit != 0) {
    block.bytes.push_back(block.byte);
    block.indexes.push_back(last_ind);
  } else {
    block.byte = 0;
  }
}

void ReadBlock(std::ifstream &arc, Block &block, const size_t block_size) {
  size_t encoded_block_size = CountEncodedBlockSize(block_size * 8);
  size_t full_buffer_size = (encoded_block_size + 7) / 8;
  size_t part_buffer_size = encoded_block_size / 8;
  uint8_t full_buffer[full_buffer_size];
  uint8_t part_buffer[part_buffer_size];
  size_t ind = arc.tellg();
  if (block.bit == 0 ||
      ((encoded_block_size - (8 - block.bit)) + 7) / 8 == full_buffer_size) {
    arc.read(reinterpret_cast<char *>(&full_buffer), full_buffer_size);
    for (int k = 0; k < full_buffer_size; ++k) {
      block.indexes.push_back(ind + k);
      block.bytes.push_back(full_buffer[k]);
    }
  } else {
    arc.read(reinterpret_cast<char *>(&part_buffer), part_buffer_size);
    for (int k = 0; k < part_buffer_size; ++k) {
      block.indexes.push_back(ind + k);
      block.bytes.push_back(part_buffer[k]);
    }
  }
}

void HeaderDecoding(std::ifstream &arc, Block &block, const size_t block_size,
                    std::vector<uint8_t> &answer) {
  uint8_t bytes[block_size];
  ReadBlock(arc, block, block_size);
  BlockDecoding(block, block_size, bytes);
  for (int i = 0; i < block_size; ++i) {
    answer.push_back(bytes[i]);
  }
}

void DataDecoding(std::ifstream &arc, Block &block, const size_t block_size,
                  std::ofstream &file) {
  uint8_t bytes[block_size];
  ReadBlock(arc, block, block_size);
  BlockDecoding(block, block_size, bytes);
  for (int i = 0; i < block_size; ++i) {
    file << bytes[i];
  }
}

void HeaderFullDecoding(std::ifstream &arc, const size_t block_size,
                        const size_t size, std::vector<uint8_t> &answer) {
  Block block;
  size_t encoded_block_size = CountEncodedBlockSize(block_size * 8);
  for (int j = 0; (j + encoded_block_size) <= size * 8;
       j += encoded_block_size) {
    HeaderDecoding(arc, block, block_size, answer);
  }
  if (size * 8 % encoded_block_size != 0) {
    size_t last_block_size = CountBlockSize(size * 8 % encoded_block_size) / 8;
    if (last_block_size != 0) {
      HeaderDecoding(arc, block, last_block_size, answer);
    }
  }
}

void DataFullDecoding(std::ifstream &arc, const size_t block_size,
                      const size_t size, std::ofstream &file) {
  Block block;
  size_t encoded_block_size = CountEncodedBlockSize(block_size * 8);
  for (int j = 0; (j + encoded_block_size) <= size * 8;
       j += encoded_block_size) {
    DataDecoding(arc, block, block_size, file);
  }
  if (size * 8 % encoded_block_size != 0) {
    size_t last_block_size = CountBlockSize(size * 8 % encoded_block_size) / 8;
    if (last_block_size != 0) {
      DataDecoding(arc, block, last_block_size, file);
    }
  }
}

//

size_t ReadNameSize(std::ifstream &arc) {
  uint8_t name_size_buffer[2];
  Block block;
  block.indexes.push_back(arc.tellg());
  block.indexes.push_back(1 + arc.tellg());
  arc.read(reinterpret_cast<char *>(&name_size_buffer), 2);
  block.bytes.push_back(name_size_buffer[0]);
  block.bytes.push_back(name_size_buffer[1]);
  uint8_t name_size_bytes[1];
  BlockDecoding(block, 1, name_size_bytes);
  return name_size_bytes[0];
}

void ReadName(std::ifstream &arc, const Options &opt,
              std::vector<uint8_t> &name_bytes) {
  size_t name_size = ReadNameSize(arc);
  HeaderFullDecoding(arc, opt.block_size, name_size, name_bytes);
}

void ReadSize(std::ifstream &arc, const Options &opt, size_t &size) {
  size_t size_size = CountEncodedSize(4, opt.block_size);
  std::vector<uint8_t> size_bytes;
  HeaderFullDecoding(arc, opt.block_size, size_size, size_bytes);
  size = size_bytes[0];
  size += size_bytes[1] << 8;
  size += size_bytes[2] << 16;
  size += size_bytes[3] << 24;
}

bool FindFile(const Options &opt, const size_t ind, size_t position[]) {
  std::ifstream arc(opt.filename, std::ios::binary);
  size_t arc_size = GetFileSize(opt.filename);
  while (arc.tellg() != arc_size) {
    size_t pos = arc.tellg();
    std::vector<uint8_t> name_bytes;
    ReadName(arc, opt, name_bytes);
    bool found = false;
    if (opt.filenames[ind].size() == name_bytes.size()) {
      bool match = true;
      for (int i = 0; i < name_bytes.size(); ++i) {
        if (name_bytes[i] != opt.filenames[ind][i]) {
          match = false;
          break;
        }
      }
      if (match) {
        found = true;
        position[0] = pos;
        position[1] = arc.tellg();
      }
    }
    size_t size;
    ReadSize(arc, opt, size);
    arc.seekg(size, std::ios::cur);
    if (found) {
      position[2] = arc.tellg();
      arc.close();
      return true;
    }
  }
  arc.close();
  return false;
}

void ExtractAllFiles(const Options &opt) {
  std::ifstream arc(opt.filename, std::ios::binary);
  size_t arc_size = GetFileSize(opt.filename);
  while (arc.tellg() != arc_size) {
    std::vector<uint8_t> name_bytes;
    ReadName(arc, opt, name_bytes);
    std::string filename = "*";
    for (int i = 0; i < name_bytes.size(); ++i) {
      filename += name_bytes[i];
    }
    std::ofstream file(filename, std::ios::binary);
    size_t size;
    ReadSize(arc, opt, size);
    DataFullDecoding(arc, opt.block_size, size, file);
    file.close();
  }
  arc.close();
}

//

void RewriteFixedByte(const size_t ind, const size_t ind_fixed_bit, std::ofstream &arc,
                      Block &block) {
  block.bytes[(ind + ind_fixed_bit) / 8] =
      block.bytes[(ind + ind_fixed_bit) / 8] ^ (128 >> ((ind + ind_fixed_bit) % 8));
  arc.seekp(block.indexes[(ind + ind_fixed_bit) / 8], std::ios::beg);
  arc << block.bytes[(ind + ind_fixed_bit) / 8];
}

bool FixMistake(std::ofstream &arc, Block &block, size_t block_size) {
  size_t ind = block.bit;
  HamCode code(block, block_size);
  if (!code.FixMistake()) {
    return false;
  }
  size_t ind_fixed_bit = code.GetIndMistake();
  if (ind_fixed_bit != -1) {
    RewriteFixedByte(ind, ind_fixed_bit, arc, block);
  }
  size_t last_ind = block.indexes[block.indexes.size() - 1];
  size_t last_block = block.bytes[block.bytes.size() - 1];
  block.bytes.clear();
  block.indexes.clear();
  if (block.bit != 0) {
    block.bytes.push_back(last_block);
    block.indexes.push_back(last_ind);
  } else {
    block.byte = 0;
  }
  return true;
}

bool FixMistakeSize(std::ofstream &arc, Block &block, size_t block_size,
                    std::vector<uint8_t> &size_bytes) {
  size_t ind = block.bit;
  HamCode code(block, block_size);
  if (!code.FixMistake()) {
    return false;
  }
  uint8_t size_bytes_buffer[block_size];
  code.Decoding();
  code.GetBytes(size_bytes_buffer);
  for (int i = 0; i < block_size; ++i) {
    size_bytes.push_back(size_bytes_buffer[i]);
  }
  size_t ind_fixed_bit = code.GetIndMistake();
  if (ind_fixed_bit != -1) {
    RewriteFixedByte(ind, ind_fixed_bit, arc, block);
  }
  size_t last_ind = block.indexes[block.indexes.size() - 1];
  size_t last_block = block.bytes[block.bytes.size() - 1];
  block.bytes.clear();
  block.indexes.clear();
  if (block.bit != 0) {
    block.bytes.push_back(last_block);
    block.indexes.push_back(last_ind);
  } else {
    block.byte = 0;
  }
  return true;
}

bool FixBlockMistake(std::ifstream &arc_in, std::ofstream &arc_out,
                     Block &block, const size_t block_size) {
  ReadBlock(arc_in, block, block_size);
  if (!FixMistake(arc_out, block, block_size)) {
    return false;
  }
  return true;
}

bool FixMistakes(std::ifstream &arc_in, std::ofstream &arc_out,
                 const size_t block_size, const size_t size) {
  Block block;
  size_t encoded_block_size = CountEncodedBlockSize(block_size * 8);
  for (int j = 0; (j + encoded_block_size) <= size * 8;
       j += encoded_block_size) {
    if (!FixBlockMistake(arc_in, arc_out, block, block_size)) {
      return false;
    }
  }
  if (size * 8 % encoded_block_size != 0) {
    size_t last_block_size = CountBlockSize(size * 8 % encoded_block_size) / 8;
    if (last_block_size != 0) {
      if (!FixBlockMistake(arc_in, arc_out, block, last_block_size)) {
        return false;
      }
    }
  }
  return true;
}

bool FixNameSize(std::ifstream &arc_in, std::ofstream &arc_out,
                 size_t &name_size) {
  Block block;
  ReadBlock(arc_in, block, 1);
  uint8_t name_size_bytes[1];
  HamCode code(block, 1);
  if (!code.FixMistake()) {
    return false;
  }
  size_t ind_fixed_bit = code.GetIndMistake();
  if (ind_fixed_bit != -1) {
    RewriteFixedByte(0, ind_fixed_bit, arc_out, block);
  }
  code.Decoding();
  code.GetBytes(name_size_bytes);
  name_size = name_size_bytes[0];
  return true;
}

bool FixSize(std::ifstream &arc_in, std::ofstream &arc_out,
             const size_t block_size, size_t &size) {
  std::vector<uint8_t> size_bytes;
  size_t size_size = CountEncodedSize(4, block_size);
  Block block;
  size_t encoded_block_size = CountEncodedBlockSize(block_size * 8);
  for (int j = 0; (j + encoded_block_size) <= size_size * 8;
       j += encoded_block_size) {
    ReadBlock(arc_in, block, block_size);
    if (!FixMistakeSize(arc_out, block, block_size, size_bytes)) {
      return false;
    }
  }
  if (size_size * 8 % encoded_block_size != 0) {
    size_t last_block_size =
        CountBlockSize(size_size * 8 % encoded_block_size) / 8;
    if (last_block_size != 0) {
      ReadBlock(arc_in, block, last_block_size);
      if (!FixMistakeSize(arc_out, block, last_block_size, size_bytes)) {
        return false;
      }
    }
  }
  size = size_bytes[0];
  size += size_bytes[1] << 8;
  size += size_bytes[2] << 16;
  size += size_bytes[3] << 24;
  return true;
}

bool FixArchive(const std::string &name, const Options &opt) {
  std::ifstream arc_in(name, std::ios::binary);
  std::ofstream arc_out(name, std::ios::in | std::ios::binary);
  size_t arc_size = GetFileSize(name);
  while (arc_in.tellg() != arc_size) {
    size_t name_size;
    if (!FixNameSize(arc_in, arc_out, name_size)) {
      arc_in.close();
      arc_out.close();
      return false;
    }
    std::ifstream arc(name, std::ios::binary);
    if (!FixMistakes(arc_in, arc_out, opt.block_size, name_size)) {
      arc_in.close();
      arc_out.close();
      return false;
    }
    size_t size;
    if (!FixSize(arc_in, arc_out, opt.block_size, size)) {
      arc_in.close();
      arc_out.close();
      return false;
    }
    if (!FixMistakes(arc_in, arc_out, opt.block_size, size)) {
      arc_in.close();
      arc_out.close();
      return false;
    }
  }
  arc_in.close();
  arc_out.close();
  return true;
}

void BreakArchive(const std::string &name) {
  std::ifstream file1(name, std::ios::binary);
  std::ofstream file2(name, std::ios::in | std::ios::binary);
  size_t size = GetFileSize(name);
  uint8_t byte;
  for (int i = 0; i < size; ++i) {
    file1.read(reinterpret_cast<char *>(&byte), 1);
    if (i % 17 == 0) {
      file2.seekp(i, std::ios::beg);
      byte = byte ^ 1;
      file2 << byte;
    }
  }
  file1.close();
  file2.close();
}

//

bool Insert(const Options &opt) {
  if (!FixArchive(opt.filename, opt)) {
    return false;
  }
  std::ofstream arc(opt.filename, std::ios::app | std::ios::binary);
  for (int i = 0; i < opt.filenames.size(); ++i) {
    NameCoding(arc, opt, i);
    SizeCoding(arc, opt, i);
    FileCoding(arc, opt, i);
  }
  arc.close();
  return true;
}

void Create(const Options &opt) {
  std::ofstream arc(opt.filename, std::ios::binary);
  for (int i = 0; i < opt.filenames.size(); ++i) {
    NameCoding(arc, opt, i);
    SizeCoding(arc, opt, i);
    FileCoding(arc, opt, i);
  }
  arc.close();
}

bool Concatenate(const Options &opt) {
  for (int i = 0; i < opt.filename.size(); ++i) {
    if (!FixArchive(opt.filenames[i], opt)) {
      return false;
    }
  }
  std::ofstream arc(opt.filename, std::ios::binary);
  for (int i = 0; i < opt.filenames.size(); ++i) {
    std::ifstream file(opt.filenames[i], std::ios::binary);
    uint8_t byte;
    while (file.read(reinterpret_cast<char *>(&byte), 1)) {
      arc << byte;
    }
    file.close();
  }
  arc.close();
  return true;
}

bool Extract(const Options &opt) {
  if (!FixArchive(opt.filename, opt)) {
    return false;
  }
  if (opt.filenames.size() == 0) {
    ExtractAllFiles(opt);
    return true;
  }
  std::ifstream arc(opt.filename, std::ios::binary);
  for (int i = 0; i < opt.filenames.size(); ++i) {
    size_t position[3];
    if (!FindFile(opt, i, position)) {
      arc.close();
      return false;
    }
    std::ofstream file("*" + opt.filenames[i], std::ios::binary);
    arc.seekg(position[1], std::ios::beg);
    size_t size;
    ReadSize(arc, opt, size);
    DataFullDecoding(arc, opt.block_size, size, file);
    file.close();
  }
  arc.close();
  return true;
}

bool List(const Options &opt) {
  if (!FixArchive(opt.filename, opt)) {
    return false;
  }
  std::ifstream arc(opt.filename, std::ios::binary);
  size_t arc_size = GetFileSize(opt.filename);
  while (arc.tellg() != arc_size) {
    std::vector<uint8_t> name_bytes;
    ReadName(arc, opt, name_bytes);
    for (int i = 0; i < name_bytes.size(); ++i) {
      std::cout << name_bytes[i];
    }
    std::cout << '\n';
    size_t size;
    ReadSize(arc, opt, size);
    arc.seekg(size, std::ios::cur);
  }
  arc.close();
  return true;
}

bool Delete(const Options &opt) {
  if (!FixArchive(opt.filename, opt)) {
    return false;
  }
  for (int i = 0; i < opt.filenames.size(); ++i) {
    size_t position[3];
    if (!FindFile(opt, i, position)) {
      return false;
    }
    std::ifstream arc(opt.filename, std::ios::binary);
    arc.seekg(position[2]);
    std::ofstream new_arc(opt.filename, std::ios::in | std::ios::binary);
    new_arc.seekp(position[0]);
    uint8_t byte;
    while (arc.read(reinterpret_cast<char *>(&byte), 1)) {
      new_arc << byte;
    }
    std::filesystem::resize_file(opt.filename, new_arc.tellp());
    new_arc.close();
    arc.close();
  }
  return true;
}

//

bool ArchivatorManager(const Options &opt) {
  BreakArchive(opt.filename);
  if (opt.need_creation) {
    Create(opt);
  } else if (opt.need_list) {
    if (!List(opt)) {
      return false;
    }
  } else if (opt.need_extraction) {
    if (!Extract(opt)) {
      return false;
    }
  } else if (opt.need_addition) {
    if (!Insert(opt)) {
      return false;
    }
  } else if (opt.need_deletion) {
    if (!Delete(opt)) {
      return false;
    }
  } else if (opt.need_concatenation) {
    if (!Concatenate(opt)) {
      return false;
    }
  } else {
    return false;
  }
  return true;
}

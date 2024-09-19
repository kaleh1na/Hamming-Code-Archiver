#pragma once
#include <fstream>

#include "ArgParser.h"

using namespace ArgumentParser;

struct Options {
  std::string filename;
  std::vector<std::string> filenames;
  bool need_creation;
  bool need_list;
  bool need_extraction;
  bool need_addition;
  bool need_deletion;
  bool need_concatenation;
  int block_size;
};

bool CheckFileExistence(const std::string &filename);

bool Parse(int argc, char **argv, Options &opt);

//

size_t CountEncodedBlockSize(const size_t block_size);

size_t CountBlockSize(const size_t encoded_block_size);

size_t CountEncodedSize(const size_t file_size, const size_t block_size);

size_t GetFileSize(const std::string &filename);

//

struct Block {
  std::vector<uint8_t> bytes;
  std::vector<size_t> indexes;
  uint8_t byte = 0;
  size_t bit = 0;
};

class HamCode {
  std::vector<bool> data_;
  std::vector<bool> encoded_data_;
  int ind_fixed_bit_ = -1;

  size_t FindMistake() const;

 public:
  HamCode(uint8_t bytes[], const size_t block_size);

  HamCode(Block &block, const size_t block_size);

  int GetIndMistake() const;

  bool FixMistake();

  void Coding();

  void Decoding();

  void FillBytes(Block &block) const;

  void GetBytes(uint8_t bytes[]) const;
};

//

void FillBlock(std::ofstream &arc, uint8_t buffer[], size_t block_size,
               Block &block);

void SizeCoding(std::ofstream &arc, const Options &opt, const size_t i);

void FileCoding(std::ofstream &arc, const Options &opt, const size_t i);

void NameCoding(std::ofstream &arc, const Options &opt, const size_t i);

//

void BlockDecoding(Block &block, size_t block_size, uint8_t bytes[]);

void ReadBlock(std::ifstream &arc, Block &block, const size_t block_size);

void HeaderDecoding(std::ifstream &arc, Block &block, const size_t block_size,
                    std::vector<uint8_t> &answer);

void DataDecoding(std::ifstream &arc, Block &block, const size_t block_size,
                  std::ofstream &file);

void HeaderFullDecoding(std::ifstream &arc, const size_t block_size,
                        const size_t size, std::vector<uint8_t> &answer);

void DataFullDecoding(std::ifstream &arc, const size_t block_size,
                      const size_t size, std::ofstream &file);

//

size_t ReadNameSize(std::ifstream &arc);

void ReadName(std::ifstream &arc, const Options &opt,
              std::vector<uint8_t> &name_bytes);

void ReadSize(std::ifstream &arc, const Options &opt, size_t &size);

bool FindFile(const Options &opt, const size_t ind, size_t position[]);

void ExtractAllFiles(const Options &opt);

//

void RewriteFixedByte(const size_t ind, const size_t ind_fixed_bit, std::ofstream &arc,
                      Block &block);

bool FixMistake(std::ofstream &arc, Block &block, size_t block_size);

bool FixMistakeSize(std::ofstream &arc, Block &block, size_t block_size,
                    std::vector<uint8_t> &size_bytes);

bool FixBlockMistake(std::ifstream &arc_in, std::ofstream &arc_out,
                     Block &block, const size_t block_size);

bool FixMistakes(std::ifstream &arc_in, std::ofstream &arc_out,
                 const size_t block_size, const size_t size);

bool FixNameSize(std::ifstream &arc_in, std::ofstream &arc_out,
                 size_t &name_size);

bool FixSize(std::ifstream &arc_in, std::ofstream &arc_out,
             const size_t block_size, size_t &size);

bool FixArchive(const std::string &name, const Options &opt);

void BreakArchive(const std::string &name);

//

bool Insert(const Options &opt);

void Create(const Options &opt);

bool Concatenate(const Options &opt);

bool Extract(const Options &opt);

bool List(const Options &opt);

bool Delete(const Options &opt);

//

bool ArchivatorManager(const Options &opt);

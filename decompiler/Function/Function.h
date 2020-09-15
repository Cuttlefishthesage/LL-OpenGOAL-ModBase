#ifndef NEXT_FUNCTION_H
#define NEXT_FUNCTION_H

#include <string>
#include <vector>
#include "decompiler/Disasm/Instruction.h"
#include "BasicBlocks.h"
#include "CfgVtx.h"

struct FunctionName {
  enum class FunctionKind {
    UNIDENTIFIED,  // hasn't been identified yet.
    GLOBAL,        // global named function
    METHOD,
    TOP_LEVEL_INIT,
  } kind = FunctionKind::UNIDENTIFIED;

  std::string function_name;  // only applicable for GLOBAL
  std::string type_name;      // only applicable for METHOD
  int method_id = -1;         // only applicable for METHOD

  std::string to_string() const {
    switch (kind) {
      case FunctionKind::GLOBAL:
        return function_name;
      case FunctionKind::METHOD:
        return "(method " + std::to_string(method_id) + " " + type_name + ")";
      case FunctionKind::TOP_LEVEL_INIT:
        return "(top-level-login)";
      case FunctionKind::UNIDENTIFIED:
        return "(?)";
      default:
        throw std::runtime_error("Unsupported FunctionKind");
    }
  }

  bool empty() const { return kind == FunctionKind::UNIDENTIFIED; }

  void set_as_top_level() { kind = FunctionKind::TOP_LEVEL_INIT; }

  void set_as_global(std::string name) {
    kind = FunctionKind::GLOBAL;
    function_name = std::move(name);
  }

  void set_as_method(std::string tn, int id) {
    kind = FunctionKind::METHOD;
    type_name = std::move(tn);
    method_id = id;
  }

  bool expected_unique() const {
    return kind == FunctionKind::GLOBAL || kind == FunctionKind::METHOD;
  }
};

class Function {
 public:
  Function(int _start_word, int _end_word);
  void analyze_prologue(const LinkedObjectFile& file);
  void find_global_function_defs(LinkedObjectFile& file);
  void find_method_defs(LinkedObjectFile& file);

  int segment = -1;
  int start_word = -1;
  int end_word = -1;  // not inclusive, but does include padding.

  FunctionName guessed_name;

  bool suspected_asm = false;

  std::vector<Instruction> instructions;
  std::vector<BasicBlock> basic_blocks;
  std::shared_ptr<ControlFlowGraph> cfg = nullptr;

  int prologue_start = -1;
  int prologue_end = -1;

  int epilogue_start = -1;
  int epilogue_end = -1;

  std::string warnings;

  struct Prologue {
    bool decoded = false;  // have we removed the prologue from basic blocks?
    int total_stack_usage = -1;

    // ra/fp are treated differently from other register backups
    bool ra_backed_up = false;
    int ra_backup_offset = -1;

    bool fp_backed_up = false;
    int fp_backup_offset = -1;

    bool fp_set = false;

    int n_gpr_backup = 0;
    int gpr_backup_offset = -1;

    int n_fpr_backup = 0;
    int fpr_backup_offset = -1;

    int n_stack_var_bytes = 0;
    int stack_var_offset = -1;

    bool epilogue_ok = false;

    std::string to_string(int indent = 0) const;

  } prologue;

  bool uses_fp_register = false;

 private:
  void check_epilogue(const LinkedObjectFile& file);
};

#endif  // NEXT_FUNCTION_H

#ifndef _CULT_INSTBENCH_H
#define _CULT_INSTBENCH_H

#include <vector>

#include "basebench.h"

namespace cult {

// ============================================================================
// [cult::InstSpec]
// ============================================================================

struct InstSpec {
  // Instruction signature is 4 values (8-bit each) describing 4 operands:
  enum Op : uint32_t {
    kOpNone = 0,
    kOpRel,
    kOpGpb,
    kOpGpw,
    kOpGpd,
    kOpGpq,
    kOpAl,
    kOpCl,
    kOpDl,
    kOpBl,
    kOpAx,
    kOpCx,
    kOpDx,
    kOpBx,
    kOpEax,
    kOpEcx,
    kOpEdx,
    kOpEbx,
    kOpRax,
    kOpRcx,
    kOpRdx,
    kOpRbx,
    kOpMm,
    kOpXmm,
    kOpXmm0,
    kOpYmm,
    kOpZmm,
    kOpKReg,
    kOpImm8,
    kOpImm16,
    kOpImm32,
    kOpImm64,
    kOpMem8,
    kOpMem16,
    kOpMem32,
    kOpMem64,
    kOpMem128,
    kOpMem256,
    kOpMem512
  };

  static inline InstSpec none() {
    return InstSpec { 0 };
  }

  static inline InstSpec pack(uint32_t o0, uint32_t o1 = 0, uint32_t o2 = 0, uint32_t o3 = 0, uint32_t o4 = 0, uint32_t o5 = 0) {
    return InstSpec { uint64_t(o0) | uint64_t(o1 << 8) | uint64_t(o2 << 16) | uint64_t(o3 << 24) | (uint64_t(o4) << 32) | (uint64_t(o5) << 40) };
  }

  inline bool isValid() const { return value != 0; }

  inline uint32_t count() const {
    uint32_t i = 0;
    uint64_t v = value;
    while (v & 0xFF) {
      i++;
      v >>= 8;
    }
    return i;
  }

  inline uint32_t get(size_t index) const {
    assert(index < 6);
    return uint32_t((value >> (index * 8)) & 0xFF);
  }

  static inline bool isImplicitOp(uint32_t op) {
    return (op >= kOpAl && op <= kOpRdx) || op == kOpXmm0;
  }

  uint64_t value;
};

// ============================================================================
// [cult::InstBench]
// ============================================================================

class InstBench : public BaseBench {
public:
  typedef void (*Func)(uint32_t nIter, uint64_t* out);

  InstBench(App* app);
  virtual ~InstBench();

  void classify(std::vector<InstSpec>& dst, InstId instId);
  double testInstruction(InstId instId, InstSpec instSpec, uint32_t parallel, bool overheadOnly);

  inline bool is64Bit() const {
    return Environment::is64Bit(Arch::kHost);
  }

  bool isImplicit(InstId instId);

  uint32_t numIterByInstId(InstId instId);

  inline bool isMMX(InstId instId, InstSpec spec) {
    return spec.get(0) == InstSpec::kOpMm || spec.get(1) == InstSpec::kOpMm;
  }

  inline bool isVec(InstId instId, InstSpec spec) {
    const x86::InstDB::InstInfo& inst = x86::InstDB::infoById(instId);
    return inst.isVec() && !isMMX(instId, spec);
  }

  inline bool isSSE(InstId instId, InstSpec spec) {
    const x86::InstDB::InstInfo& inst = x86::InstDB::infoById(instId);
    return inst.isVec() && !isMMX(instId, spec) && !inst.isVex() && !inst.isEvex();
  }

  inline bool isAVX(InstId instId, InstSpec spec) {
    const x86::InstDB::InstInfo& inst = x86::InstDB::infoById(instId);
    return inst.isVec() && (inst.isVex() || inst.isEvex());
  }

  inline bool canRun(InstId instId) const {
    return _canRun(BaseInst(instId), nullptr, 0);
  }

  template<typename... ArgsT>
  inline bool canRun(InstId instId, ArgsT&&... args) const {
    Operand_ ops[] = { args... };
    return _canRun(BaseInst(instId), ops, sizeof...(args));
  }

  bool _canRun(const BaseInst& inst, const Operand_* operands, uint32_t count) const;

  void run() override;
  void beforeBody(x86::Assembler& a) override;
  void compileBody(x86::Assembler& a, x86::Gp rCnt) override;
  void afterBody(x86::Assembler& a) override;

  uint32_t _instId;
  InstSpec _instSpec;
  uint32_t _nUnroll;
  uint32_t _nParallel;
  bool _overheadOnly;
};

} // cult namespace

#endif // _CULT_INSTBENCH_H

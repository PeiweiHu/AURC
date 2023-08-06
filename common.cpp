#include "common.h"

void debugPrint() {}

void printBasicBlock(BasicBlock *B) {
  for (Instruction &Ins : *B) {
    errs() << Ins.getOpcodeName() << " " << Ins << "\n";
  }
}

void printOperand(Instruction *ins) {
  errs() << "Operands of " << *ins << "\n";
  for (size_t t = 0; t < ins->getNumOperands(); t++) {
    errs() << t << ": " << *(ins->getOperand(t)) << "\n";
  }
}

void printStrVector(vector<string> *Vec) {
  for (string S : *Vec) {
    errs() << S << " ";
  }
  errs() << "\n";
}

void splitStr(string _Str, string Deli, vector<string> *Res) {
  if (_Str == "") {
    return;
  }

  string Str = _Str + Deli;
  size_t Pos = Str.find(Deli);
  int Step = Deli.size();

  while (Pos != Str.npos) {
    string Temp = Str.substr(0, Pos);
    Res->push_back(Temp);
    Str = Str.substr(Pos + Step, Str.size());
    Pos = Str.find(Deli);
  }
}

void printPath(vector<vector<BasicBlock *> *> *Paths) {
  for (vector<BasicBlock *> *Path : *Paths) { // single path
    errs() << "--- Path ---\n";
    for (BasicBlock *Blk : *Path) {
      for (Instruction &Ins : *Blk) {
        errs() << Ins << "\n";
      }
    }
  }
}

void printSinglePath(vector<BasicBlock *> *Path) {
  errs() << "--- Path ---\n";
  for (BasicBlock *Blk : *Path) {
    for (Instruction &Ins : *Blk) {
      errs() << Ins << "\n";
    }
  }
}

set<Function *> *collectCallee(Function *F) {
  set<Function *> *callee_set = new set<Function *>();
  if (callee_set == NULL) {
    errs() << "Fail to allocate the memory\n";
    return NULL;
  }

  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction &Ins = *I;
    if (auto CallIns = dyn_cast<CallInst>(&Ins)) {
      Function *callee = CallIns->getCalledFunction();
      if (callee != NULL) {
        callee_set->insert(callee);
      } else {
        errs() << "Fail to getCalledFunction in collectCallee at " << Ins
               << "\n";
      }
    }
  }
  return callee_set;
}

void clearPath(vector<vector<BasicBlock *> *> *path) {
  // <BasicBlock *> will be duplicate
  vector<uint64_t> freed_addr;
  for (size_t i = 0; i != path->size(); i++) {
    vector<BasicBlock *> *ele = (*path)[i];
    // first, we clean BasicBlock *
    for (auto &e : *ele) {
      bool free_flag = true;
      for (auto addr : freed_addr) {
        if (addr == (uint64_t)e) {
          free_flag = false;
          // errs() << "do not free " << (uint64_t)e << " again\n";
        }
      }
      if (free_flag) {
        // delete e; // COMMENT THIS SINCE (delete LLVMCtx;) free this!!
        // errs() << "freed " << (uint64_t)e << "\n";
        freed_addr.push_back((uint64_t)e);
      }
    }
    delete ele;
  }
  delete path;
}

// Do not forget to use clearPath on the return value
vector<vector<BasicBlock *> *> *collectPath(Function *F) {
  unrollLoops(F);

  vector<vector<BasicBlock *> *> *path = new vector<vector<BasicBlock *> *>();
  assert(path);
  // construct startup, add single entry block
  BasicBlock *entry = &F->getEntryBlock();
  if (entry == NULL) {
    // errs() << "[-] Fail to find entry block\n";
    delete path;
    return NULL;
  }
  vector<BasicBlock *> *p = new vector<BasicBlock *>();
  assert(p);
  p->push_back(entry);
  path->push_back(p);
  // start iteration until no path has successors
  bool flag = true;
  int loop = 0, maxloop = 300000;
  while (flag) {
    flag = false;
    for (size_t idx = 0; idx != path->size(); idx++) {
      loop++;
      if (loop > maxloop) {
        // errs() << "[-] Over max loop constraints\n";
        goto ERR;
      }
      vector<BasicBlock *> *cur_path = (*path)[idx];
      // access the last instruction of the last block
      BasicBlock *last = cur_path->back();
      Instruction *last_ins = last->getTerminator();
      if (last_ins == NULL) {
        // errs() << "[-] Fail to getTerminator\n";
        goto ERR;
      }
      if (last_ins->getNumSuccessors()) {
        // the first successor is added to cur_path directly
        if (find(cur_path->begin(), cur_path->end(),
                 last_ins->getSuccessor(0)) == cur_path->end()) {
          // not an existing block
          cur_path->push_back(last_ins->getSuccessor(0));
          flag = true;
        }
        for (size_t succ_id = 1; succ_id < last_ins->getNumSuccessors();
             succ_id++) {
          // whether an existing block in current path
          if (find(cur_path->begin(), cur_path->end(),
                   last_ins->getSuccessor(succ_id)) != cur_path->end()) {
            continue;
          }
          vector<BasicBlock *> *expanded_path = new vector<BasicBlock *>();
          assert(expanded_path != NULL);
          expanded_path->insert(expanded_path->end(), cur_path->begin(),
                                cur_path->end() - 1);
          expanded_path->push_back(last_ins->getSuccessor(succ_id));
          path->push_back(expanded_path);
          flag = true;
        }
      }
    }
  }
  // when the above iteration ends, all paths in <path> have no successors
  // except back edges
  return path;
ERR:
  for (size_t st = 0; st < path->size(); st++) {
    vector<BasicBlock *> *Vec = path->at(st);
    delete Vec;
  }
  delete path;
  return NULL;
}

vector<BasicBlock *> *getLoopHeaders(Function *F) {

  // if (F->isDeclaration())
  //	return;

  vector<BasicBlock *> *Headers = new vector<BasicBlock *>();
  assert(Headers);

  DominatorTree DT = DominatorTree();
  DT.recalculate(*F);
  LoopInfo *LI = new LoopInfo();
  LI->releaseMemory();
  LI->analyze(DT);

  // Collect all loops in the function
  set<Loop *> LPSet;
  for (LoopInfo::iterator i = LI->begin(), e = LI->end(); i != e; ++i) {

    Loop *LP = *i;
    LPSet.insert(LP);

    list<Loop *> LPL;

    LPL.push_back(LP);
    while (!LPL.empty()) {
      LP = LPL.front();
      LPL.pop_front();
      vector<Loop *> SubLPs = LP->getSubLoops();
      for (auto SubLP : SubLPs) {
        LPSet.insert(SubLP);
        LPL.push_back(SubLP);
      }
    }
  }

  for (Loop *LP : LPSet) {

    // Get the header,latch block, exiting block of every loop
    BasicBlock *HeaderB = LP->getHeader();
    Headers->push_back(HeaderB);
  }

  delete LI;
  return Headers;
}

// cut from Crix
void unrollLoops(Function *F) {

  if (F->isDeclaration())
    return;

  DominatorTree DT = DominatorTree();
  DT.recalculate(*F);
  LoopInfo *LI = new LoopInfo();
  LI->releaseMemory();
  LI->analyze(DT);

  // Collect all loops in the function
  set<Loop *> LPSet;
  for (LoopInfo::iterator i = LI->begin(), e = LI->end(); i != e; ++i) {

    Loop *LP = *i;
    LPSet.insert(LP);

    list<Loop *> LPL;

    LPL.push_back(LP);
    while (!LPL.empty()) {
      LP = LPL.front();
      LPL.pop_front();
      vector<Loop *> SubLPs = LP->getSubLoops();
      for (auto SubLP : SubLPs) {
        LPSet.insert(SubLP);
        LPL.push_back(SubLP);
      }
    }
  }

  for (Loop *LP : LPSet) {

    // Get the header,latch block, exiting block of every loop
    BasicBlock *HeaderB = LP->getHeader();

    unsigned NumBE = LP->getNumBackEdges();
    SmallVector<BasicBlock *, 4> LatchBS;

    LP->getLoopLatches(LatchBS);

    for (BasicBlock *LatchB : LatchBS) {
      if (!HeaderB || !LatchB) {
        errs() << "[-] ERROR: Cannot find Header Block or Latch Block\n";
        continue;
      }
      // Two cases:
      // 1. Latch Block has only one successor:
      // 	for loop or while loop;
      // 	In this case: set the Successor of Latch Block to the
      //	successor block (out of loop one) of Header block
      // 2. Latch Block has two successor:
      // do-while loop:
      // In this case: set the Successor of Latch Block to the
      //  another successor block of Latch block

      // get the last instruction in the Latch block
      Instruction *TI = LatchB->getTerminator();
      // Case 1:
      if (LatchB->getSingleSuccessor() != NULL) {
        for (succ_iterator sit = succ_begin(HeaderB); sit != succ_end(HeaderB);
             ++sit) {

          BasicBlock *SuccB = *sit;
          BasicBlockEdge BBE = BasicBlockEdge(HeaderB, SuccB);
          // Header block has two successor,
          // one edge dominate Latch block;
          // another does not.
          if (DT.dominates(BBE, LatchB))
            continue;
          else {
            TI->setSuccessor(0, SuccB);
          }
        }
      }
      // Case 2:
      else {
        for (succ_iterator sit = succ_begin(LatchB); sit != succ_end(LatchB);
             ++sit) {

          BasicBlock *SuccB = *sit;
          // There will be two successor blocks, one is header
          // we need successor to be another
          if (SuccB == HeaderB)
            continue;
          else {
            TI->setSuccessor(0, SuccB);
          }
        }
      }
    }
  }

  delete LI;
}

Function *loadFuncFromFile(LLVMContext *Context, string Path, string Name) {
  SMDiagnostic Err;
  unique_ptr<Module> M = parseIRFile(Path, Err, *Context);
  if (M == NULL) {
    errs() << "Error loading file: " << Path << "\n";
    return NULL;
  }
  Module *Module = M.release();
  Module::FunctionListType &FuncList = Module->getFunctionList();
  for (Function &F : FuncList) {
    if (F.isIntrinsic() || F.isDeclaration()) {
      continue;
    }
    if (F.getName() == Name) {
      return &F;
    }
  }
  return NULL;
}

Function *loadFuncFromFileList(LLVMContext *Context, string Name,
                               vector<string> *Bitcodes) {
  assert(Context && Bitcodes);

  for (string B : *Bitcodes) {
    Function *F = loadFuncFromFile(Context, B, Name);
    if (F) {
      return F;
    }
  }
  return NULL;
}

vector<Function *> *loadAllFuncFromFile(LLVMContext *Context, string Path) {
  SMDiagnostic Err;
  unique_ptr<Module> M;
  try {
    M = parseIRFile(Path, Err, *Context);
  } catch (exception E) {
    M = NULL;
  }
  if (M == NULL) {
    errs() << "Error loading file: " << Path << "\n";
    return NULL;
  }
  Module *Module = M.release();
  Module::FunctionListType &FuncList = Module->getFunctionList();
  vector<Function *> *AllFunc = new vector<Function *>();
  assert(AllFunc);
  for (Function &F : FuncList) {
    if (F.isIntrinsic() || F.isDeclaration()) {
      continue;
    }
    if (F.getName() == "main") {
      continue;
    }
    AllFunc->push_back(&F);
  }
  return AllFunc;
}

/* collect and return the references of the instruction */
vector<Value *> *collectRefs(Value *Ins) {
  if (isa<ConstantInt>(Ins)) {
    return new vector<Value *>();
  }
  vector<Value *> *Refs = new vector<Value *>();
  assert(Refs);
  Refs->push_back(Ins);

  if (LoadInst *LoadIns = dyn_cast<LoadInst>(Ins)) {
    Refs->push_back(LoadIns->getOperand(0));
  }

  bool Flag = true;
  int PosStart = 0, PosEnd = Refs->size();
  while (Flag) {
    Flag = false;
    for (int Idx = PosStart; Idx < PosEnd; Idx++) {
      // here we set a max loop constraint
      if (Idx > 5000) {
        errs() << COLOR_RED CONS_TAB CONS_TAB CONS_TAB
            "|- ERROR: over the max loop constraint while collecting "
            "refs\n" COLOR_CLOSE;
        break;
      }
      Value *V = Refs->at(Idx);
      for (User *U : V->users()) {
        if (LoadInst *LoadIns = dyn_cast<LoadInst>(U)) {
          // if the loaded element is within Refs
          Value *Loaded = LoadIns->getOperand(0);
          if (find(Refs->begin() + PosStart, Refs->begin() + PosEnd, Loaded) !=
              Refs->begin() + PosEnd) {
            Refs->push_back(LoadIns);
            Flag = true;
          }
        }

        if (StoreInst *StoreIns = dyn_cast<StoreInst>(U)) {
          Value *From = StoreIns->getOperand(0);
          if (find(Refs->begin() + PosStart, Refs->begin() + PosEnd, From) !=
              Refs->begin() + PosEnd) {
            Refs->push_back(StoreIns->getOperand(1));
            Flag = true;
          }
        }
      }
    }

    PosStart = PosEnd;
    PosEnd = Refs->size();
  }

  // remove possible duplicated values
  set<Value *> S(Refs->begin(), Refs->end());
  Refs->erase(Refs->begin(), Refs->end());
  Refs->assign(S.begin(), S.end());

  return Refs;
}

string getCmpSym(CmpInst::Predicate P) {
  switch (P) {
  case CmpInst::ICMP_NE:
    return "NE";
  case CmpInst::ICMP_EQ:
    return "EQ";
  case CmpInst::ICMP_SGT:
    return "SGT";
  case CmpInst::ICMP_SLT:
    return "SLT";
  case CmpInst::ICMP_SGE:
    return "SGE";
  case CmpInst::ICMP_SLE:
    return "SLE";
  default:
    assert(0);
  }
}

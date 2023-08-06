#include "caller_analysis.h"
#include "llvm/IR/Constants.h"

/**
 * The high-level interface for collecting return checks.
 *
 * Read a list of bitcode files and collect return checks.
 *
 * @param BcFiles the bitcode files of the target project
 */
void CollectCheck(vector<string> *BcFiles) {
  // LLVM Context
  LLVMContext *CoreContext = new LLVMContext();
  assert(CoreContext);
  // CallerContext
  CallerContext *CContext = new CallerContext();
  assert(CContext);
  CContext->unsetDebug();

  // analyse each file
  for (auto BF : *BcFiles) {
    errs() << COLOR_GREEN "\n[Caller] analyze file - " << BF
           << "\n" COLOR_CLOSE;
    vector<Function *> *FileFunc =
        loadAllFuncFromFile(CoreContext, BF); // assert(FileFunc);
    if (!FileFunc) {
      continue;
    }
    // analyse each function
    for (auto F : *FileFunc) {
      SingleFunction(F, CContext);
    }

    // cleanup
    delete FileFunc;
  }

  errs() << COLOR_BLUE "[*] writing checks to file......\n";
  CContext->writeCheckTo("cops.txt");

  delete CContext;
  delete CoreContext;
}

void SingleFunction(Function *F, CallerContext *CContext) {
  assert(F);
  assert(CContext);

  string FuncName = F->getName().str();
  if (CContext->isAnalyzed(FuncName)) {
    return;
  }

  errs() << CONS_TAB "|-analyze func - " << FuncName << "\n";

  for (auto Iter = inst_begin(F); Iter != inst_end(F); Iter++) {
    // get the use of call instruction
    Instruction *Ins = &*Iter;
    if (CallInst *CallIns = dyn_cast<CallInst>(Ins)) {
      SingleCallInst(CallIns, CContext);
    }
  }
}

void SingleCallInst(CallInst *CallIns, CallerContext *CContext) {
  assert(CallIns && CContext);

  // get the called function
  Function *Callee = CallIns->getCalledFunction();
  if (!Callee) {
    return;
  }
  if (Callee->isIntrinsic()) {
    return;
  }
  // if (Callee->isDeclaration()) {
  //     return;
  // }
  if (!Callee->getReturnType()->isIntegerTy()) {
    return;
  }

  string CalleeName = Callee->getName().str();
  errs() << CONS_TAB CONS_TAB "|- analysing the callee " << CalleeName << "\n";

  // collect all references to the invocation
  vector<Value *> *Refs = collectRefs(CallIns);
  assert(Refs);

  // store loop info, we wanna if check instead of loop check
  vector<BasicBlock *> *LoopHeaderVec =
      getLoopHeaders(CallIns->getParent()->getParent());
  assert(LoopHeaderVec);

  // catch compare instructions
  vector<ICmpInst *> *CmpInsVec = new vector<ICmpInst *>();
  assert(CmpInsVec);
  // V refers to the invocation
  for (Value *V : *Refs) {
    /*
     *  Here may be inaccurate since the pointer variable (assigned
     *  by AllocaInst) may store values beyond CallInst. However, it's
     *  a conservative approximation and will not affect the FPs.
     *
     *  trade-off with running speed
     * */
    for (User *U : V->users()) {
      if (ICmpInst *ICmpIns = dyn_cast<ICmpInst>(U)) {
        if (find(LoopHeaderVec->begin(), LoopHeaderVec->end(),
                 ICmpIns->getParent()) != LoopHeaderVec->end()) {
          continue;
        }
        // currently focus on comparison with zero
        if (V != ICmpIns->getOperand(0)) {
          continue;
        }
        Value *N = ICmpIns->getOperand(1);
        if (!isa<ConstantInt>(N)) {
          continue;
        }
        // ConstantInt *CI = dyn_cast<ConstantInt>(N);
        // if (!CI->isZero()) {
        //     continue;
        // }
        CmpInsVec->push_back(ICmpIns);
      }
    }
  }
  delete LoopHeaderVec;

  // now we get all comparison instructions, deduce ranges
  string CallerName = CallIns->getParent()->getParent()->getName().str();
  string ModuleName = CallIns->getModule()->getName().str();

  // print the comparison instructions in debug mode
  if (CContext->isDebug()) {
    errs() << "\n" CONS_TAB CONS_TAB CONS_TAB "comparison instructions:\n";
    for (auto Ins : *CmpInsVec) {
      errs() << CONS_TAB CONS_TAB CONS_TAB << *Ins << "\n";
    }
    errs() << "\n";
  }

  ReturnCheck *RC = new ReturnCheck(CalleeName, CallerName, ModuleName);
  assert(RC);
  for (ICmpInst *IC : *CmpInsVec) {
    string Sym = getCmpSym(IC->getSignedPredicate());
    ConstantInt *CI = dyn_cast<ConstantInt>(IC->getOperand(1));
    assert(CI);
    string Val = to_string(CI->getValue().getSExtValue());
    RC->addCheck(Sym, Val);
  }
  if (RC->Checks->size()) {
    CContext->addCheck(RC);
  } else {
    delete RC;
  }

  // convert the comparison instructions to symbols
  // set<string> Syms;
  // for (auto Ins : *CmpInsVec) {
  //     Syms.insert(getCmpSym(Ins->getSignedPredicate()));
  // }

  // if (Syms.size() == 1) {
  //     string CheckSym = *(Syms.begin());
  //     vector<string> *Sym = new vector<string>(); assert(Sym);
  //     Sym->push_back(CheckSym);
  //     CContext->addCheck(CalleeName, CallerName, ModuleName, Sym);
  //     errs() << COLOR_BLUE CONS_TAB CONS_TAB CONS_TAB "|- " << CalleeName <<
  //     " " << CheckSym << "\n" << COLOR_CLOSE;
  // } else if (Syms.size() == 2) {
  //     string SA = *(Syms.begin());
  //     string SB = *(++Syms.begin());
  //     bool ZeroCheck = false, LessCheck = false, GreatCheck = false;
  //     if (SA == "NE" || SA == "EQ" || SB == "NE" || SB == "EQ") {
  //         ZeroCheck = true;
  //     }
  //     if (SA == "SLT" || SA == "SLE" || SB == "SLT" || SB == "SLE") {
  //         LessCheck = true;
  //     }
  //     if (SA == "SGT" || SA == "SGE" || SB == "SGT" || SB == "SGE") {
  //         GreatCheck = true;
  //     }
  //     if ((ZeroCheck && LessCheck) || (ZeroCheck && GreatCheck) || (LessCheck
  //     && GreatCheck)) {
  //         vector<string> *Sym = new vector<string>(); assert(Sym);
  //         Sym->push_back("EQ");
  //         Sym->push_back("SLT");
  //         Sym->push_back("SGT");
  //         CContext->addCheck(CalleeName, CallerName, ModuleName, Sym);
  //         errs() << COLOR_BLUE CONS_TAB CONS_TAB CONS_TAB "|- " << CalleeName
  //         << " EQ SLT SGT\n"  << COLOR_CLOSE;
  //     } else {
  //         errs() << COLOR_RED CONS_TAB CONS_TAB CONS_TAB "|- EXCEPTION:
  //         exceptions during multiple checks of " << CalleeName << "\n"
  //         COLOR_CLOSE;
  //     }
  // } else {
  //     errs() << COLOR_RED CONS_TAB CONS_TAB CONS_TAB "|- EXCEPTION: " <<
  //     CmpInsVec->size() << " comparison instructions of " << CalleeName <<
  //     "\n" COLOR_CLOSE;
  // }

  // cleanup
  delete CmpInsVec;
  delete Refs;
}

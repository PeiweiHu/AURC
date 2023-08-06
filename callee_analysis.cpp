#include "callee_analysis.h"
#include "common.h"

/*
    Context-sensitive Backtrace Prediction - CBP
*/

/**
 * The high-level CBP interface.
 *
 * Read a list of bitcode files and perform return
 * value prediction.
 *
 * @param BcFiles the bitcode files of the target project
 * @param CntPath true while counting the CBP's effectiveness in solving path
 * explosion
 */
void CBP(vector<string> *BcFiles, bool CntPath) {
  // LLVM Context
  LLVMContext *CoreContext = new LLVMContext();
  assert(CoreContext);
  // CBP Context
  CBPContext *CContext = new CBPContext();
  assert(CContext);
  CContext->unsetDebug();

  // analyse each file
  for (auto BF : *BcFiles) {
    errs() << COLOR_GREEN "\n[Callee] analyze file - " << BF
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

  // CContext->printReturnVal();

  // remove duplicated return values before replacement
  vector<string> *PF = CContext->getPredictedFunction();
  assert(PF);
  for (string Name : *PF) {
    CContext->removeDupReturnVal(Name);
  }
  delete PF;

  // for experiments
  if (CntPath) {
    unsigned long long AllNestPath = 0, AllRepPath = 0;
    vector<string> *PredictedFunc = CContext->getPredictedFunction();
    for (string Name : *PredictedFunc) {
      int FirstLevel = CContext->getCnt()->getFuncPathNum(Name);
      if (FirstLevel == -1) {
        continue;
      }
      // for nested invocation replacement, the number of the first
      // level is the number of analyzed paths
      AllRepPath += FirstLevel;
      // the second level
      vector<CBPRet> *DependOn = CContext->RetDependOn(Name);
      if (!DependOn) {
        AllNestPath += FirstLevel;
        continue;
      }
      for (CBPRet R : *DependOn) {
        assert(R.isFuncType());
        int Num = CContext->getCnt()->getFuncPathNum(R.getVal());
        if (Num == -1) {
          continue;
        }
        FirstLevel *= Num;
        // the third level
        vector<CBPRet> *InnerDependOn = CContext->RetDependOn(R.getVal());
        if (!InnerDependOn) {
          continue;
        }
        for (CBPRet InnerR : *InnerDependOn) {
          assert(InnerR.isFuncType());
          int InnerNum = CContext->getCnt()->getFuncPathNum(InnerR.getVal());
          if (InnerNum == -1) {
            continue;
          }
          FirstLevel *= InnerNum;
        }
        delete InnerDependOn;
      }
      delete DependOn;
      AllNestPath += FirstLevel;
    }
    // errs() << "replaced path, nest path, replaced path / nest path = " +
    // to_string(AllRepPath);
    errs() << " " + to_string(AllNestPath) + " " +
                  to_string((double)AllRepPath / (double)AllNestPath) + "\n";

    delete PredictedFunc;
    delete CContext;
    delete CoreContext;
    return;
  }

  // count the function that invokes itself as the return value
  int CCnt = CContext->cntSelfInvocation();
  // errs() << " Self Invocation: " << to_string(CCnt) << "\n";

  // now AURC starts nested invocation replacement
  errs() << COLOR_GREEN
      "[*] Conducting nested invocation replacement......\n" COLOR_CLOSE;
  bool flag = true;
  while (flag) {
    flag = false;

    // iterate every predicted function
    vector<string> *PredictedFunc = CContext->getPredictedFunction();
    for (string Name : *PredictedFunc) {
      vector<CBPRet> *DependOn = CContext->RetDependOn(Name);
      if (DependOn && DependOn->size()) {
        // D is the name of functions that <Name> depends on
        for (CBPRet D : *DependOn) {
          if (!CContext->isPredicted(D.getVal())) {
            continue;
          }

          /*
              replace strategy:

                  If A depends on B, check whether there is a value
                  that is in B but not A. If so, add it to A and set
                  the flag.
          */

          // first we save the range constraints of the return value
          string Range = D.getRange();
          // remove D from the return values of Name - MUST be the range
          // constaints
          CContext->removeReturnVal(Name, D);
          // add the return values of D to Name
          vector<CBPRet> *DRet = CContext->getReturnVal(D.getVal());
          assert(DRet);
          // append the old range constraints
          for (int Idx = 0; Idx < DRet->size(); Idx++) {
            CBPRet *R = &(DRet->at(Idx));
            string RRange = R->getRange();
            if (Range.size()) {
              if (RRange.size()) {
                R->setRange(RRange + "|" + Range);
              } else {
                R->setRange(Range);
              }
            }
          }
          // add new value
          for (CBPRet R : *DRet) {
            if (!CContext->hasReturnVal(Name, R)) {
              CContext->addReturnVal(Name, R);
              flag = true;
            }
          }
          delete DRet;
        }
      }
      delete DependOn;
    }
    delete PredictedFunc;
  }

  // remove the return values that contradicts with the range constraints
  errs() << COLOR_GREEN "[*] Removing the return values that contradicts with "
                        "the range constraints......\n" COLOR_CLOSE;
  vector<string> *PredictedFunc = CContext->getPredictedFunction();
  for (string FuncName : *PredictedFunc) {
    CContext->removeContradictedValue(FuncName);
  }

  delete PredictedFunc;

  // CContext->printReturnVal();
  CContext->writeRetTo("return_values.txt");
  CContext->getCnt()->printStatus();
  errs() << "\nbackward size / forward size = " +
                to_string(CContext->getCnt()->getCBPInstRate()) + "\n";

  delete CoreContext;
  delete CContext;
}

/**
 * The CBP interface for single function.
 *
 * Predict the return values of single function.
 *
 * @param F function under test
 */
void SingleFunction(Function *F, CBPContext *CContext) {
  assert(F);
  assert(CContext);

  string FuncName = F->getName().str();

  errs() << CONS_TAB "|-analyze func - " << FuncName << "\n";

  // 1. whether skip this function
  // 1.1 return type
  if (!F->getReturnType()->isIntegerTy()) {
    errs() << CONS_TAB CONS_TAB
        "|-SKIP: return type is not satisfied (isIntegerTy)\n";
    return;
  }
  // if (F->getReturnType()->getIntegerBitWidth() != 32) { // distinguish int
  // and long
  //     errs() << CONS_TAB CONS_TAB "|-SKIP: return type is not satisfied (32
  //     bit)\n"; return;
  // }
  // 1.2 inner function
  if (F->isIntrinsic()) {
    errs() << CONS_TAB CONS_TAB "|-SKIP: inner function\n";
    return;
  }
  // 1.3 extern function
  if (F->isDeclaration()) {
    errs() << CONS_TAB CONS_TAB "|-SKIP: external function\n";
    return;
  }
  // 1.4 already predicted
  if (CContext->isPredicted(FuncName)) {
    errs() << CONS_TAB CONS_TAB "|-SKIP: already predicted\n";
    return;
  }

  // set function under test
  CContext->setFUT(FuncName);

  // collect the execution path
  vector<vector<BasicBlock *> *> *Paths = collectPath(F);
  if (!Paths) {
    errs() << COLOR_RED CONS_TAB CONS_TAB
        "|-ERROR: Over max loop constraint while collecting path\n" COLOR_CLOSE;
    return;
  }

  // this is for experiments
  CContext->getCnt()->logFuncPathNum(CContext->getFUT(), Paths->size());

  // analyse each path
  for (auto Path : *Paths) {
    SinglePath(Path, CContext);
  }

  // cleanup
  clearPath(Paths);
}

/**
 * Parse the returned value iteratively.
 *
 * For example,
 *
 *        %2 = load i32, i32* %retval, align 4
 *        ret i32 %2
 *
 * ret i32 %2 -> %2 -> %retval
 *
 * Path is used for phi node.
 *
 * @param Ret the returned value
 * @param Path the path under test
 * @return parsed return value
 */
Value *getRet(Value *Ret, vector<Instruction *> *Insts) {
  Value *OldRet;
  int LoopCnt = 0;
  do {
    LoopCnt += 1;
    OldRet = Ret;
    if (ReturnInst *RI = dyn_cast<ReturnInst>(Ret)) {
      Ret = RI->getOperand(0);
    }
    if (LoadInst *LI = dyn_cast<LoadInst>(Ret)) {
      Ret = LI->getOperand(0);
    }
    /* PHINode can lead to loop, e.g., Curl_if2ip in curl */
    if (PHINode *PN = dyn_cast<PHINode>(Ret)) {
      // get the last instruction before the phi node
      int PhiId;
      for (PhiId = 0; PhiId < Insts->size(); PhiId++) {
        if (Insts->at(PhiId) == PN) {
          break;
        }
      }
      // Debug Later
      if (PhiId >= Insts->size()) {
        return NULL;
      }

      BasicBlock *PredBlock = Insts->at(PhiId - 1)->getParent();
      // decide the value
      int N = PN->getNumIncomingValues();
      for (int Idx = 0; Idx < N; Idx++) {
        Ret = PN->getIncomingValue(Idx);
        BasicBlock *B = PN->getIncomingBlock(Idx);
        if (B == PredBlock) {
          break;
        }
      }
    }
    if (ZExtInst *ZI = dyn_cast<ZExtInst>(Ret)) {
      Ret = ZI->getOperand(0);
    }
  } while (OldRet != Ret && LoopCnt < 10000);

  if (LoopCnt >= 10000) {
    Ret = NULL;
  }
  return Ret;
}

/**
 * The CBP interface for single path.
 *
 * Predict the return values of single path.
 *
 * @param Path path under test
 */
void SinglePath(vector<BasicBlock *> *Path, CBPContext *CContext) {
  assert(Path);
  assert(CContext);

  if (CContext->isDebug()) {
    printSinglePath(Path);
  }

  // AURC skips the INSTANT execution path like simple parameter null checks
  // and keeps the paths containing real errors
  // typical parameter null checks --- entry block -> set return values ->
  // return block (a || b) --- entry block and first check -> second check ->
  // set return values -> return block
  bool NullCheck = false;

  BasicBlock *EntryBlock = NULL;
  int SetRetIdx;
  if (Path->size() == 3) {
    EntryBlock = Path->at(0);
    SetRetIdx = 1;
  }
  if (Path->size() == 4) {
    EntryBlock = Path->at(1);
    SetRetIdx = 2;
  }

  if (EntryBlock) {
    if (BranchInst *BrIns = dyn_cast<BranchInst>(EntryBlock->getTerminator())) {
      bool SeqOfBranch = BrIns->getNumOperands() == 3 &&
                         (BrIns->getOperand(2) == Path->at(SetRetIdx));
      ICmpInst *CmpIns = dyn_cast<ICmpInst>(BrIns->getOperand(0));

      if (SeqOfBranch && CmpIns &&
          getCmpSym(CmpIns->getSignedPredicate()) == "EQ") {
        // is this a null check for parameters ?
        Value *Op0 = CmpIns->getOperand(0);
        Value *Op1 = CmpIns->getOperand(1);

        if (ConstantPointerNull *CI = dyn_cast<ConstantPointerNull>(Op1)) {
          // whether Op0 is the reference of some parameter
          Function *F = EntryBlock->getParent();
          assert(F);
          vector<Value *> *ParaRef = new vector<Value *>();
          assert(ParaRef);
          for (Argument &Para : F->args()) {
            if (Para.getType()->isPointerTy()) {
              for (auto V : Para.users()) {
                if (StoreInst *SI = dyn_cast<StoreInst>(V)) {
                  if (SI->getOperand(0) == &Para) {
                    ParaRef->push_back(SI->getOperand(1));
                  }
                }
              }
            }
          }
          vector<Value *> *Ref = collectRefs(Op0);
          for (Value *R : *Ref) {
            if (find(ParaRef->begin(), ParaRef->end(), R) != ParaRef->end()) {
              NullCheck = true;
            }
          }
          delete ParaRef;
          delete Ref;
        }
      }
    }
  }

  bool InstantPrefix;
  if (NullCheck) {
    errs() << COLOR_BLUE CONS_TAB CONS_TAB CONS_TAB
        "|- Find a null check path\n" COLOR_CLOSE;
    InstantPrefix = true;
  } else {
    InstantPrefix = false;
  }

  // currently Path is basic block-based, change it to single instructions
  vector<Instruction *> *Insts = new vector<Instruction *>();
  assert(Insts);
  for (BasicBlock *B : *Path) {
    for (Instruction &I : *B) {
      Insts->push_back(&I);
    }
  }

  // 1. some checks
  // 1.1 the last instruction should be return statement
  Instruction *LastIns = Insts->at(Insts->size() - 1);
  if (!isa<ReturnInst>(LastIns)) {
    // cleanup
    delete Insts;
    return;
  }
  // 1.2 if return the constant int
  if (ConstantInt *CI = dyn_cast<ConstantInt>(LastIns)) {
    CContext->addReturnVal(CContext->getFUT(),
                           to_string(CI->getValue().getSExtValue()), false,
                           InstantPrefix);
    CContext->getCnt()->addSucc(CContext->getFUT());
    CContext->getCnt()->addCBPInstNum(Insts->size(), 1);
    errs() << COLOR_BLUE CONS_TAB CONS_TAB CONS_TAB
        "|- Return constant integer\n" COLOR_CLOSE;
    return;
  }

  // path constraints
  // errs() << COLOR_BLUE CONS_TAB CONS_TAB CONS_TAB "|- Start collecting path
  // constraints\n" COLOR_CLOSE;
  vector<string> Constraints;
  Value *Ret = LastIns->getOperand(0);
  vector<Value *> *Refs = collectRefs(Ret);
  assert(Refs);
  // any ref is used in comparison instructions?
  for (Value *V : *Refs) {
    for (User *U : V->users()) {
      if (ICmpInst *ICmpIns = dyn_cast<ICmpInst>(U)) {
        if (ICmpIns->getOperand(0) != V) {
          continue;
        }
        if (!isa<ConstantInt>(ICmpIns->getOperand(1))) {
          continue;
        }
        ConstantInt *CI = dyn_cast<ConstantInt>(ICmpIns->getOperand(1));
        if (!CI->isZero()) {
          continue;
        }
        string Sym = getCmpSym(ICmpIns->getSignedPredicate());

        // is current path the sequence or alternative path ?
        // generate the subsequent instructions in sequence path

        // first get br instruction
        BasicBlock *B = ICmpIns->getParent();
        if (!isa<BranchInst>(B->getTerminator())) {
          continue;
        }
        BranchInst *Br = dyn_cast<BranchInst>(B->getTerminator());
        if (!Br || !Br->isConditional()) {
          continue;
        }
        // operand 0 -> condition,  operand 1 -> false branch, operand 2 -> true
        // branch now we generate the instruction sequence of the false branch

        BasicBlock *FBr = dyn_cast<BasicBlock>(Br->getOperand(1));
        if (!FBr) {
          continue;
        }
        vector<Instruction *> FBrIns;
        for (Instruction &I : *FBr) {
          FBrIns.push_back(&I);
        }
        // check whether the current path contain the above instructions
        int Idx;
        for (Idx = 0; Idx < Insts->size(); Idx++) {
          if (Insts->at(Idx) == Br) {
            break;
          }
        }
        if (Idx == Insts->size()) {
          continue;
        }
        // let's loop
        bool IsFalsePath = true;
        for (int Idx1 = 0, Idx2 = ++Idx;
             Idx1 < FBrIns.size() && Idx2 < Insts->size(); Idx1++, Idx2++) {
          if (FBrIns.at(Idx1) != Insts->at(Idx2)) {
            IsFalsePath = false;
            break;
          }
        }
        if (IsFalsePath) {
          if (Sym == "SLT") {
            Constraints.push_back("SGE");
          } else if (Sym == "SLE") {
            Constraints.push_back("SGT");
          }
        }
      }
    }
  }
  delete Refs;

  // construct the range constraints
  string Range = "";
  for (string S : Constraints) {
    Range += (S + "|");
  }
  // remove the last "|"
  if (Constraints.size()) {
    Range = Range.substr(0, Range.size() - 1);
  }

  // now we start predicting return values by backtracking

  // used for CBP experiment, prove the effectiveness of backward analysis
  int ForwardSize = Insts->size();
  int BackwardSize;
  Ret = getRet(LastIns->getOperand(0), Insts);
  if (!Ret) {
    CContext->addReturnVal(CContext->getFUT(), "F", false, InstantPrefix,
                           Range);
    CContext->getCnt()->addFailOthers(CContext->getFUT());
    delete Insts;
    return;
  }
  int Idx;
  for (Idx = Insts->size() - 1; Idx >= 0; Idx--) {
    BackwardSize = Insts->size() - Idx;
    assert(BackwardSize >= 0);

    // 2. check whether the current Ret is predictable.
    //    PLEASE ENSURE add one entry per path for the ease of later statistic
    // 2.1 integer and boolean
    if (ConstantInt *CI = dyn_cast<ConstantInt>(Ret)) {
      if (CI->isOne()) {
        CContext->addReturnVal(CContext->getFUT(), "1", false, InstantPrefix,
                               Range);
        CContext->getCnt()->addSucc(CContext->getFUT());
        CContext->getCnt()->addCBPInstNum(ForwardSize, BackwardSize);
        break;
      }
      if (CI->isZero()) {
        CContext->addReturnVal(CContext->getFUT(), "0", false, InstantPrefix,
                               Range);
        CContext->getCnt()->addSucc(CContext->getFUT());
        CContext->getCnt()->addCBPInstNum(ForwardSize, BackwardSize);
        break;
      }
      // https://stackoverflow.com/questions/5315176/llvm-get-constant-integer-back-from-value
      CContext->addReturnVal(CContext->getFUT(), to_string(CI->getSExtValue()),
                             false, InstantPrefix, Range);
      CContext->getCnt()->addSucc(CContext->getFUT());
      CContext->getCnt()->addCBPInstNum(ForwardSize, BackwardSize);
      break;
    }
    // 2.2 invocation
    if (CallInst *CI = dyn_cast<CallInst>(Ret)) {
      Function *Callee = CI->getCalledFunction();
      if (Callee) {
        CContext->addReturnVal(CContext->getFUT(), Callee->getName().str(),
                               true, false, Range);
        CContext->getCnt()->addSucc(CContext->getFUT());
        CContext->getCnt()->addCBPInstNum(ForwardSize, BackwardSize);
      } else {
        CContext->addReturnVal(CContext->getFUT(), "F", true, false,
                               Range); // indirect call
        CContext->getCnt()->addFailIndirectCall(CContext->getFUT());
        errs() << CONS_TAB CONS_TAB CONS_TAB "indirect call\n";
      }
      break;
    }
    // 2.3 logic calculation
    if (ICmpInst *II = dyn_cast<ICmpInst>(Ret)) {
      CContext->addReturnVal(CContext->getFUT(), "0,1", false, false, Range);
      CContext->getCnt()->addSucc(CContext->getFUT());
      CContext->getCnt()->addCBPInstNum(ForwardSize, BackwardSize);
      break;
    }

    // whether the current Inst helps
    Instruction *CurIns = Insts->at(Idx);
    if (StoreInst *SI = dyn_cast<StoreInst>(CurIns)) {
      Value *Dst = SI->getOperand(1);
      if (Dst == Ret) {
        Ret = getRet(SI->getOperand(0), Insts);
      }
    }
  }

  if (Idx == -1) {
    CContext->addReturnVal(CContext->getFUT(), "F", false, false, Range);

    // log the reason for failure
    if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(Ret)) {
      // return value coming from the property, like a[1], a.b
      CContext->getCnt()->addFailGetEle(CContext->getFUT());
      errs() << CONS_TAB CONS_TAB CONS_TAB "can not solve GetElementPtrInst\n";
    } else if (BinaryOperator *BO = dyn_cast<BinaryOperator>(Ret)) {
      // operation like add and sub
      CContext->getCnt()->addFailArithOp(CContext->getFUT());
      errs() << CONS_TAB CONS_TAB CONS_TAB
          "can not solve arithmatic operation\n";
    } else {
      CContext->getCnt()->addFailReachDef(CContext->getFUT());
      errs() << CONS_TAB CONS_TAB CONS_TAB "no reach def\n";
    }
  }

  // cleanup
  delete Insts;
}

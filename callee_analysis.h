#ifndef CALLEE_H
#define CALLEE_H

#include "common.h"
#include "llvm/IR/DataLayout.h"

/* class Ret is used to store the return values of the function */
class CBPRet {

  string Val;
  bool FuncType;
  bool Instant; // is this return value a instant check
  string Range;

public:
  CBPRet(string _Val, bool _FuncType, bool _Instant, string _Range) {
    Val = _Val;
    FuncType = _FuncType;
    Range = trimRange(_Range);
    Instant = _Instant;
  }

  CBPRet(const CBPRet &R) {
    Val = R.Val;
    FuncType = R.FuncType;
    Range = R.Range;
    Instant = R.Instant;
  }

  string str() {
    // return "$" + Val + "$" + to_string(FuncType) + "$" + Range + "$";
    // the format of the reutrn value: value | property1 | property1
    // the entries are divided by '|', the first entry is the value, while
    // the others are properties like func, instant
    string R = Val;
    if (Instant) {
      R += "|INSTANT";
    }
    if (FuncType) {
      R += "|Func";
    }
    if (Range.size()) {
      R += "|Range:" + Range;
    }
    return R;
  }

  string debugStr() {
    return "$" + Val + "$" + to_string(FuncType) + "$" + Range + "$";
    // return Val;
  }

  bool isFuncType() { return FuncType; }

  // is the current CBPRet an invocation with Name
  bool isFuncVal(string Name) { return FuncType && Val == Name; }

  string getVal() { return Val; }

  string getRange() { return Range; }

  void setRange(string _R) { Range = trimRange(_R); }

  vector<string> *parseRange(string _R) const {
    // remove duplicated range
    vector<string> *Vec = new vector<string>();
    assert(Vec);
    splitStr(_R, "|", Vec);
    set<string> S(Vec->begin(), Vec->end());
    Vec->erase(Vec->begin(), Vec->end());
    Vec->assign(S.begin(), S.end());
    return Vec;
  }

  string trimRange(string R) {
    // remove duplicate and sort the range
    vector<string> *Vec = parseRange(R);
    assert(Vec);
    std::sort(Vec->begin(), Vec->end(),
              [](string A, string B) { return A > B; });
    // merge
    string Res = "";
    for (string S : *Vec) {
      if (S != Vec->at(0)) {
        Res += "|";
      }
      Res += S;
    }
    delete Vec;
    return Res;
  }

  // bool rangeEqual(string R1, string R2) const {
  //     vector<string> *V1 = parseRange(R1);
  //     vector<string> *V2 = parseRange(R2);

  //    if (V1->size() != V2->size()) {
  //        return false;
  //    }

  //    for (string S : *V1) {
  //        if (find(V2->begin(), V2->end(), S) == V2->end()) {
  //            delete V1;
  //            delete V2;
  //            return false;
  //        }
  //    }

  //    delete V1;
  //    delete V2;
  //    return true;
  //}

  /* for method like find and remove */
  bool operator==(const CBPRet &Ret) {
    return Val == Ret.Val && FuncType == Ret.FuncType && Range == Ret.Range &&
           Instant == Ret.Instant;
  }

  /* set uses < for equation check, if both A < B and B < A are false, A = B */
  bool operator<(const CBPRet &Ret) const {
    string S1 = "$" + Val + "$" + to_string(FuncType) + "$" +
                to_string(Instant) + "$" + Range + "$";
    string S2 = "$" + Ret.Val + "$" + to_string(Ret.FuncType) + "$" +
                to_string(Ret.Instant) + "$" + Ret.Range + "$";
    return S1 < S2;
  }
};

class CBPCnt {
  /* this class is used to evaluate the effectiveness of CBP */
private:
  /*
   * Data stores in the format key = <function name>, value = <count>.
   *
   * In particular, within value, the first is successed predicted value,
   * the others are failing for <indirect call>, <no reaching def>, <others>
   *
   * */
  map<string, vector<int> *> *Data;

  // store the analyzed instruction numbers of forward and backward
  // analysis to prove the effectiveness of CBP
  vector<tuple<int, int>> *InstNum;

  // store the path number of the function
  map<string, int> *FuncPath;

  enum TYPE_INDEX { // make sure SUCC is the first and OTHER_FAIL is the last
    SUCC,
    GET_ELE,
    ARITH_OP,
    INDIRECT_CALL,
    NO_REACH_DEF,
    OTHER_FAIL,
  };

  string Tips[TYPE_INDEX::OTHER_FAIL + 1] = {
      "Success",       "GetELEmentPtrInst", "BinaryOperator",
      "Indirect Call", "Reaching Def",      "Other",
  };

public:
  CBPCnt() {
    Data = new map<string, vector<int> *>();
    assert(Data);
    InstNum = new vector<tuple<int, int>>();
    assert(InstNum);
    FuncPath = new map<string, int>();
    assert(FuncPath);
  }

  ~CBPCnt() {
    for (auto Entry : *Data) {
      delete Entry.second;
    }
    delete Data;
    delete InstNum;
    delete FuncPath;
  }

  void logFuncPathNum(string FuncName, int PathNum) {
    FuncPath->insert(make_pair<>(FuncName, PathNum));
  }

  int getFuncPathNum(string FuncName) {
    auto Iter = FuncPath->find(FuncName);
    if (Iter == FuncPath->end()) {
      return -1;
    }
    return Iter->second;
  }

  void addCBPInstNum(int ForwardNum, int BackwardNum) {
    assert(BackwardNum <= ForwardNum);
    InstNum->push_back(make_tuple(ForwardNum, BackwardNum));
  }

  float getCBPInstRate() {
    float TempRate, Cnt = 0, Rate = 0;
    for (tuple<int, int> T : *InstNum) {
      TempRate = (float)get<1>(T) / (float)get<0>(T);
      assert(TempRate <= 1);
      Rate += TempRate;
      Cnt++;
    }
    return Rate / Cnt;
  }

  vector<int> *checkKey(string FuncName) {
    if (Data->find(FuncName) == Data->end()) {
      // create the vector and init it
      enum TYPE_INDEX TI;
      vector<int> *Vec = new vector<int>();
      assert(Vec);
      for (TI = SUCC; TI <= OTHER_FAIL; TI = (TYPE_INDEX)(TI + 1)) {
        Vec->push_back(0);
      }
      Data->insert(make_pair<>(FuncName, Vec));
    }
    return Data->find(FuncName)->second;
  }

  void addSucc(string FuncName) {
    vector<int> *Val = checkKey(FuncName);
    assert(Val);
    Val->at(TYPE_INDEX::SUCC) = Val->at(TYPE_INDEX::SUCC) + 1;
  }

  void addFailGetEle(string FuncName) {
    vector<int> *Val = checkKey(FuncName);
    assert(Val);
    Val->at(TYPE_INDEX::GET_ELE) = Val->at(TYPE_INDEX::GET_ELE) + 1;
  }

  void addFailArithOp(string FuncName) {
    vector<int> *Val = checkKey(FuncName);
    assert(Val);
    Val->at(TYPE_INDEX::ARITH_OP) = Val->at(TYPE_INDEX::ARITH_OP) + 1;
  }

  void addFailIndirectCall(string FuncName) {
    vector<int> *Val = checkKey(FuncName);
    assert(Val);
    Val->at(TYPE_INDEX::INDIRECT_CALL) = Val->at(TYPE_INDEX::INDIRECT_CALL) + 1;
  }

  void addFailReachDef(string FuncName) {
    vector<int> *Val = checkKey(FuncName);
    assert(Val);
    Val->at(TYPE_INDEX::NO_REACH_DEF) = Val->at(TYPE_INDEX::NO_REACH_DEF) + 1;
  }

  void addFailOthers(string FuncName) {
    vector<int> *Val = checkKey(FuncName);
    assert(Val);
    Val->at(TYPE_INDEX::OTHER_FAIL) = Val->at(TYPE_INDEX::OTHER_FAIL) + 1;
  }

  void printStatus() {
    vector<int> Cnt;
    enum TYPE_INDEX TI;
    for (TI = SUCC; TI <= OTHER_FAIL; TI = (TYPE_INDEX)(TI + 1)) {
      Cnt.push_back(0);
    }

    for (auto Entry : *Data) {
      vector<int> *Vec = Entry.second;
      for (TI = SUCC; TI <= OTHER_FAIL; TI = (TYPE_INDEX)(TI + 1)) {
        Cnt.at(TI) = Cnt.at(TI) + Vec->at(TI);
      }
    }

    errs() << "\n\n---------------------------\n";
    for (int I = 0; I < Cnt.size(); I++) {
      errs() << Tips[I] + ": " + to_string(Cnt.at(I)) + "  ";
    }
    errs() << "\n---------------------------\n";
  }
};

class CBPContext {
private:
  /* record the predicted return values */
  map<string, vector<CBPRet> *> *ReturnVal;
  string FunctionUnderTest;
  bool debug;

  /* cleanup function */
  void cleanVecDic(map<string, vector<CBPRet> *> *Dic) {
    for (auto Entry : *Dic) {
      delete Entry.second;
    }
    delete Dic;
  }

  /* count for evaluation */
  CBPCnt *CCnt;

public:
  CBPContext() {
    FunctionUnderTest = "";
    debug = false;
    ReturnVal = new map<string, vector<CBPRet> *>();
    assert(ReturnVal);
    CCnt = new CBPCnt();
    assert(CCnt);
  }

  ~CBPContext() {
    delete CCnt;
    cleanVecDic(ReturnVal);
  }

  CBPCnt *getCnt() { return CCnt; }

  void setFUT(string FuncName) {
    if (debug) {
      errs() << "-------- set FUT " << FuncName << " -------\n";
    }
    FunctionUnderTest = FuncName;
  }

  string getFUT() { return FunctionUnderTest; }

  void setDebug() { debug = true; }

  void unsetDebug() { debug = false; }

  bool isDebug() { return debug; }

  vector<string> *getPredictedFunction() {
    vector<string> *Func = new vector<string>();
    assert(Func);
    for (auto Entry : *ReturnVal) {
      Func->push_back(Entry.first);
    }
    return Func;
  }

  // check whether the function <Name> depends on other functions
  // to predict return values
  vector<CBPRet> *RetDependOn(string Name) {
    if (!isPredicted(Name)) {
      return NULL;
    }
    vector<CBPRet> *DependFunc = NULL;
    vector<CBPRet> *RV = ReturnVal->find(Name)->second;
    assert(RV);
    for (CBPRet R : *RV) {
      if (R.isFuncType()) {
        if (DependFunc == NULL) {
          DependFunc = new vector<CBPRet>();
        }
        if (find(DependFunc->begin(), DependFunc->end(), R) ==
            DependFunc->end()) {
          DependFunc->push_back(R);
        }
      }
    }
    return DependFunc;
  }

  void printReturnVal() {
    for (auto Entry : *ReturnVal) {
      set<string> S;
      errs() << Entry.first << " -> ";
      for (CBPRet Val : *(Entry.second)) {
        if (S.find(Val.str()) == S.end()) {
          errs() << " " << Val.str();
        }
        S.insert(Val.str());
      }
      errs() << "\n";
    }
  }

  // whether some function exist in RetVal
  bool isPredicted(string Val) {
    return ReturnVal->find(Val) != ReturnVal->end();
  }

  void removeDupReturnVal(string FuncName) {
    vector<CBPRet> *Ret = ReturnVal->find(FuncName)->second;
    if (!Ret) {
      return;
    }
    set<CBPRet> S(Ret->begin(), Ret->end());
    Ret->erase(Ret->begin(), Ret->end());
    Ret->assign(S.begin(), S.end());
  }

  void addReturnVal(string FuncName, string Val, bool isFuncType = false,
                    bool isInstant = false, string Range = "") {
    assert(Val.length());

    if (ReturnVal->find(FuncName) == ReturnVal->end()) {
      ReturnVal->insert(make_pair<>(FuncName, new vector<CBPRet>()));
    }
    // Don't forget to remove duplicated values later
    ReturnVal->find(FuncName)->second->push_back(
        CBPRet(Val, isFuncType, isInstant, Range));
  }

  void addReturnVal(string FuncName, CBPRet R) {
    if (ReturnVal->find(FuncName) == ReturnVal->end()) {
      ReturnVal->insert(make_pair<>(FuncName, new vector<CBPRet>()));
    }

    ReturnVal->find(FuncName)->second->push_back(CBPRet(R));
  }

  vector<CBPRet> *getReturnVal(string FuncName) {
    // create a new one, which should be released by the user
    if (!isPredicted(FuncName)) {
      return NULL;
    }
    vector<CBPRet> *Ret = new vector<CBPRet>();
    assert(Ret);
    vector<CBPRet> *OriginalRet = ReturnVal->find(FuncName)->second;
    assert(OriginalRet);
    for (CBPRet R : *OriginalRet) {
      Ret->push_back(CBPRet(R));
    }
    return Ret; // the USER SHOULD releases this
  }

  bool hasReturnVal(string FuncName, CBPRet Val) {
    vector<CBPRet> *Ret = getReturnVal(FuncName);
    if (!Ret) {
      return false;
    }

    bool R = find(Ret->begin(), Ret->end(), Val) != Ret->end();
    delete Ret;
    return R;
  }

  /* remove the completely same value */
  void removeReturnVal(string FuncName, CBPRet R) {
    if (!isPredicted(FuncName)) {
      return;
    }

    vector<CBPRet> *Rets = ReturnVal->find(FuncName)->second;
    assert(Rets);
    // remove directly
    auto Iter = remove(Rets->begin(), Rets->end(), R);
    Rets->erase(Iter, Rets->end());
  }

  /* remove the value with the same Val and FuncType (ignore range) */
  void removeReturnVal(string FuncName, string Val, bool FuncType) {
    if (!isPredicted(FuncName)) {
      return;
    }

    vector<CBPRet> *Rets = ReturnVal->find(FuncName)->second;
    assert(Rets);
    vector<CBPRet> Vec;
    for (CBPRet CR : *Rets) {
      if (CR.getVal() == Val && CR.isFuncType() == FuncType) {
        continue;
      }
      Vec.push_back(CBPRet(CR));
    }
    Rets->erase(Rets->begin(), Rets->end());
    for (CBPRet CR : Vec) {
      Rets->push_back(CBPRet(CR));
    }
  }

  void removeContradictedValue(string FuncName) {
    if (!isPredicted(FuncName)) {
      return;
    }

    vector<CBPRet> *Rets = ReturnVal->find(FuncName)->second;
    assert(Rets);
    vector<CBPRet> Vec;
    for (CBPRet CR : *Rets) {
      if (CR.getRange().find("SGE") != string::npos) {
        if (CR.getVal()[0] == '-') {
          continue;
        }
      }

      if (CR.getRange().find("SGT") != string::npos) {
        if (CR.getVal()[0] == '-') {
          continue;
        }
        if (CR.getVal() == "0") {
          continue;
        }
      }

      Vec.push_back(CBPRet(CR));
    }
    Rets->erase(Rets->begin(), Rets->end());
    for (CBPRet CR : Vec) {
      Rets->push_back(CBPRet(CR));
    }
  }

  // whether the Callee returns an invocation of FuncName
  bool hasFuncRet(string Callee, string FuncName) {
    if (!isPredicted(Callee)) {
      return false;
    }

    vector<CBPRet> *Rets = ReturnVal->find(Callee)->second;
    assert(Rets);
    for (CBPRet R : *Rets) {
      if (R.isFuncVal(FuncName)) {
        return true;
      }
    }
    return false;
  }

  int cntSelfInvocation() {
    int Cnt = 0;
    vector<string> *PF = getPredictedFunction();
    assert(PF);
    for (string Func : *PF) {
      if (hasFuncRet(Func, Func)) {
        Cnt += 1;
      }
    }
    delete PF;
    return Cnt;
  }

  void writeRetTo(string Path) {
    fstream FS;
    FS.open(Path, ios::trunc | ios::in | ios::out);
    if (!FS.is_open()) {
      errs() << "Fail to open " << Path << "!\n";
      return;
    }
    for (auto Entry : *ReturnVal) {
      string FuncName = Entry.first;
      vector<CBPRet> *Ret = Entry.second;
      FS << FuncName;
      set<CBPRet> S;
      for (CBPRet R : *Ret) {
        if (S.find(R) == S.end()) {
          FS << " " << R.str();
          S.insert(R);
        }
      }
      FS << "\n";
    }
    FS.close();
  }
};

void CBP(vector<string> *BcFiles, bool CntPath = false);
void SingleFunction(Function *F, CBPContext *CContext);
void SinglePath(vector<BasicBlock *> *Path, CBPContext *CContext);

#endif

#ifndef CALLER_H
#define CALLER_H

#include "common.h"

class SingleCheck {

  string Sym;
  string Val;

public:
  SingleCheck(string _Sym, string _Val) {
    Sym = _Sym;
    Val = _Val;
  }

  string outStr() { return Sym + "|" + Val; }

  bool operator<(const SingleCheck &SC) const {
    string S1 = Sym + "|" + Val;
    string S2 = SC.Sym + "|" + SC.Val;
    return S1 < S2;
  }
};

class ReturnCheck {

public:
  string Callee;
  string Caller;
  string ModuleName;
  set<SingleCheck> *Checks;

  ReturnCheck(string _Callee, string _Caller, string _ModuleName) {
    Callee = _Callee;
    Caller = _Caller;
    ModuleName = _ModuleName;
    Checks = new set<SingleCheck>();
    assert(Checks);
  }

  ~ReturnCheck() { delete Checks; }

  string outStr() {
    string Str = Caller + " " + Callee;
    for (SingleCheck SC : *Checks) {
      Str += " " + SC.outStr();
    }
    return Str;
  }

  void addCheck(string Sym, string Val) {
    Checks->insert(SingleCheck(Sym, Val));
  }
};

class CallerContext {

private:
  // store the check
  vector<ReturnCheck *> *Checks;
  // in case duplicated analysis
  set<string> *AnalyzedCaller;
  bool debug;

public:
  CallerContext() {
    debug = false;
    Checks = new vector<ReturnCheck *>();
    assert(Checks);
    AnalyzedCaller = new set<string>();
    assert(AnalyzedCaller);
  }

  ~CallerContext() {
    for (ReturnCheck *R : *Checks) {
      delete R;
    }
    delete Checks;
    delete AnalyzedCaller;
  }

  void setDebug() { debug = true; }

  void unsetDebug() { debug = false; }

  bool isDebug() { return debug; }

  void startAnalysisOf(string Caller) { AnalyzedCaller->insert(Caller); }

  bool isAnalyzed(string Caller) {
    return AnalyzedCaller->find(Caller) != AnalyzedCaller->end();
  }

  void addCheck(ReturnCheck *RC) {
    string Caller = RC->Caller;
    AnalyzedCaller->insert(Caller);
    Checks->push_back(RC);
  }

  void writeCheckTo(string Path) {
    fstream FS;
    FS.open(Path, ios::trunc | ios::in | ios::out);
    if (!FS.is_open()) {
      errs() << "Fail to open " << Path << "!\n";
      return;
    }
    for (ReturnCheck *C : *Checks) {
      FS << C->outStr() + "\n";
    }
    FS.close();
  }

  void printCheck() {
    for (ReturnCheck *C : *Checks) {
      errs() << C->outStr() + "\n";
    }
  }
};

void CollectCheck(vector<string> *BcFiles);
void SingleFunction(Function *F, CallerContext *CContext);
void SingleCallInst(CallInst *CallIns, CallerContext *CContext);
string getCmpSym(CmpInst::Predicate P);
#endif

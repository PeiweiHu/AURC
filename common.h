#ifndef COMMON_H
#define COMMON_H

#include "llvm/ADT/StringSet.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include <fstream>
#include <iostream>
#include <llvm/Support/CommandLine.h>
#include <map>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

using namespace std;
using namespace llvm;

// console color
#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"
#define COLOR_CLOSE "\033[0m"
#define COLOR_BLUE "\033[34m"

#define CONS_TAB "    "

// print the instructions within BasicBlock B
void printBasicBlock(BasicBlock *B);

//
void printOperand(Instruction *ins);

//
void splitStr(string Str, string Deli, vector<string> *Res);

//
void printStrVector(vector<string> *Vec);

//
void printPath(vector<vector<BasicBlock *> *> *Paths);

//
void printSinglePath(vector<BasicBlock *> *Path);

// return the names of all invoked functions in Function F
set<Function *> *collectCallee(Function *F);

// collect the execution path of Function F
vector<vector<BasicBlock *> *> *collectPath(Function *F);

// clean the path returned by collectPath
void clearPath(vector<vector<BasicBlock *> *> *path);

//
vector<BasicBlock *> *getLoopHeaders(Function *F);

//
void unrollLoops(Function *F);

// search a function from bitcode file
Function *loadFuncFromFile(LLVMContext *Context, string Path, string Name);

//
Function *loadFuncFromFileList(LLVMContext *Context, string Name,
                               vector<string> *Bitcodes);

// collects all functions from bitcode file
vector<Function *> *loadAllFuncFromFile(LLVMContext *Context, string Path);

//
vector<Value *> *collectRefs(Value *Ins);

//
string getCmpSym(CmpInst::Predicate P);

#endif

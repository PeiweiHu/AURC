#include "main.h"

cl::list<string> InputFilenames(cl::Positional, cl::OneOrMore,
                                cl::desc("<input bitcode files>"));

/* ./main --print-func=<function name> <bitcode files> */
cl::opt<std::string> Print("print-func",
                           cl::desc("Print the LLVM IR of the function"),
                           cl::Optional);

/* ./main --find-func=<function name> <bitcode files> */
cl::opt<std::string>
    Find("find-func", cl::desc("Find the LLVM bitcode file of the function"),
         cl::Optional);

/* ./main --cops <bitcode files> */
cl::opt<bool> ProjCops(
    "cops",
    cl::desc("Generate the cops of the whole project and write to cops.txt"),
    cl::init(false));

/* ./main --cbp <bitcode files> */
cl::opt<bool> ProjCbp(
    "cbp",
    cl::desc(
        "Generate the cbp of the whole project and write to return_values.txt"),
    cl::init(false));

cl::opt<bool> ProjPathExp("path-exp", cl::desc("Path explosion experiment"),
                          cl::init(false));

int main(int argc, char **argv) {
  cl::ParseCommandLineOptions(
      argc, argv,
      "AURs consistency Checker - Peiwei Hu <jlu.hpw@foxmail.com>\n");

  vector<string> *BcFiles = new vector<string>();
  assert(BcFiles);
  for (unsigned i = 0; i < InputFilenames.size(); ++i) {
    BcFiles->push_back(InputFilenames[i]);
  }

  if (Find.size()) {
    LLVMContext *Context = new LLVMContext();
    assert(Context);
    Function *F = NULL;
    for (string B : *BcFiles) {
      F = loadFuncFromFile(Context, B, Find);
      if (F) {
        errs() << COLOR_BLUE "Find in " << B << "\n" COLOR_CLOSE;
        break;
      }
    }

    if (!F) {
      errs() << COLOR_RED "Fail to find this function\n" COLOR_CLOSE;
    }

    delete Context;
  }

  if (Print.size()) {
    LLVMContext *Context = new LLVMContext();
    assert(Context);
    Function *F = loadFuncFromFileList(Context, Print, BcFiles);
    if (F) {
      errs() << COLOR_BLUE "------ LLVM IR of " << Print
             << " ------\n" COLOR_CLOSE;
      for (auto Iter = inst_begin(F); Iter != inst_end(F); Iter++) {
        errs() << CONS_TAB << *Iter << "\n";
      }
    } else {
      errs() << COLOR_RED
          "Fail to find the function definition......\n" COLOR_CLOSE;
    }
    delete Context;
  }

  if (ProjPathExp) {
    CBP(BcFiles, true);
  }

  if (ProjCbp) {
    CBP(BcFiles);
  }

  if (ProjCops) {
    CollectCheck(BcFiles);
  }

  delete BcFiles;

  return 0;
}

LLVM_CONFIG?=llvm-config


SRC_DIR?=$(PWD)
LDFLAGS+=$(shell $(LLVM_CONFIG) --ldflags)
#Debug flags
COMMON_FLAGS=-Wall -Wextra -g
CXXFLAGS+=$(COMMON_FLAGS) $(shell $(LLVM_CONFIG) --cxxflags) -fno-rtti -fexceptions -fsanitize=address
CPPFLAGS+=$(shell $(LLVM_CONFIG) --cppflags) -I$(SRC_DIR)
#Add the static libs that may be linked
LLVMLIBS=$(shell $(LLVM_CONFIG) --libs irreader bitwriter passes core support)
SYSTEMLIBS=$(shell $(LLVM_CONFIG) --system-libs)


default: main

common.o : $(SRC_DIR)/common.cpp $(SRC_DIR)/common.h
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

callee_analysis.o : $(SRC_DIR)/callee_analysis.cpp $(SRC_DIR)/callee_analysis.h
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

caller_analysis.o : $(SRC_DIR)/caller_analysis.cpp $(SRC_DIR)/caller_analysis.h
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

main : common.o callee_analysis.o caller_analysis.o $(SRC_DIR)/main.cpp $(SRC_DIR)/main.h
	@echo Linking $@
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $^ $(LLVMLIBS) $(SYSTEMLIBS)

.PHONY: clean
clean::
	rm -f main *.o

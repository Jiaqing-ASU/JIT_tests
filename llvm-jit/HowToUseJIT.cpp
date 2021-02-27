//int add1(int x) {
//   return x+1;
// }
//
// int foo() {
//   return add1(10);
// }

#include "llvm/ADT/STLExtras.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

/**
#include "llvm/ExecutionEngine/MCJIT.h"
This will call some static initializer function which is required for JITting to work in LLVM.

LLVMInitializeNativeAsmPrinter();
This function is required to produce assembly for the target machine.

clang++ -g HowToUseJIT.cpp $(/usr/local/opt/llvm@8/bin/llvm-config --cxxflags --ldflags --system-libs --libs core mcjit native orcjit) -rdynamic -O3 -o HowToUseJIT
 */

using namespace llvm;
using namespace std;

// Use LLVM IR command to generate the add1 and foo functions
int initModule(LLVMContext &Context, Module *M, Function *FooF) {

    // Create the add1 function entry and insert this entry into module M.  The
    // function will have a return type of "int" and take an argument of "int".
    Function *Add1F =
            cast<Function>(M->getOrInsertFunction("add1", Type::getInt32Ty(Context),
                                                  Type::getInt32Ty(Context)));

    // Add a basic block to the function. As before, it automatically inserts
    // because of the last argument.
    BasicBlock *BB = BasicBlock::Create(Context, "EntryBlock", Add1F);

    // Create a basic block builder with default parameters.  The builder will
    // automatically append instructions to the basic block `BB'.
    IRBuilder<> builder(BB);

    // Get pointers to the constant `1'.
    Value *One = builder.getInt32(1);

    // Get pointers to the integer argument of the add1 function.
    assert(Add1F->arg_begin() != Add1F->arg_end()); // Make sure there's an arg
    Argument *ArgX = &*Add1F->arg_begin();          // Get the arg
    ArgX->setName("AnArg");            // Give it a name.

    // Create the add instruction, inserting it into the end of BB.
    Value *Add = builder.CreateAdd(One, ArgX);

    // Create the return instruction and add it to the basic block
    builder.CreateRet(Add);

    // Now, function add1 is ready.

    // Create another function `foo', which returns an int and takes no arguments.
    
    // Add a basic block to the FooF function.
    BB = BasicBlock::Create(Context, "EntryBlock", FooF);

    // Tell the basic block builder to attach itself to the new basic block
    builder.SetInsertPoint(BB);

    // Get pointer to the constant `10'.
    Value *Ten = builder.getInt32(10);
    
    // Pass Ten to the call to Add1F
    CallInst *Add1CallRes = builder.CreateCall(Add1F, Ten);
    Add1CallRes->setTailCall(true);

    // Create the return instruction and add it to the basic block.
    // The builder writes the IR into the function.
    builder.CreateRet(Add1CallRes);
    
    return 0;
}

// Use LLVM JIT to run the above 2 functions
int main() {
    
    // Initialization
    InitializeNativeTarget(); //No this, cannot get TargetMachine
    InitializeNativeTargetAsmPrinter();// No this, cannot generate machine code
    
    LLVMContext Context;
    
    // Create some module to put our function into it.
    std::unique_ptr<Module> Owner = make_unique<Module>("test", Context);
    Module *M = Owner.get();

    Function *FooF =
            cast<Function>(M->getOrInsertFunction("foo", Type::getInt32Ty(Context)));

    initModule(Context, M, FooF);

    // Now we create the JIT.
    ExecutionEngine *EE = EngineBuilder(std::move(Owner)).create();
    
    //output the IR code in M
    outs() << "We just constructed this LLVM module:\n\n" << *M;
    outs() << "\n\nRunning foo: ";
    outs().flush();
    
    // Call the `foo' function with no arguments:
    std::vector<GenericValue> noargs;
    GenericValue gv = EE->runFunction(FooF, noargs);
    
    // Import result of execution:
    outs() << "Result: " << gv.IntVal << "\n";
    delete EE;
    llvm_shutdown();
    return 0;
}
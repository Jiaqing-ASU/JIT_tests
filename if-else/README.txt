The environment of this test code is:

clang version 4.0.1
LLVM version 8.0.1

The command to run this code is:

$clang++ -g if-else.cpp $(/usr/local/opt/llvm@8/bin/llvm-config --cxxflags --ldflags --system-libs --libs core mcjit native orcjit) -rdynamic -O3 -o if-else

And then:

$./if-else

Remember to replace the "/usr/local/opt/llvm@8/bin/" with the path your llvm@8 path.

Remember to use llvm@8 or higher. This code run under llvm 8.0.1 and llvm@4, llvm@5, llvm@7 all do not work. But this code has not tested under llvm@9 or llvm@10 or llvm@11.
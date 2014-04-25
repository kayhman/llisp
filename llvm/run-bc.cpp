#include <string>
#include <memory>
#include <iostream>

#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
//#include <llvm/ModuleProvider.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/ExecutionEngine/JIT.h>
#include "llvm/ADT/OwningPtr.h"

using namespace std;
using namespace llvm;

int main()
{
    InitializeNativeTarget();
    llvm_start_multithreaded();
    LLVMContext context;
    string error;
    OwningPtr<llvm::MemoryBuffer> fileBuf;
    MemoryBuffer::getFile("hw.bc", fileBuf);
    ErrorOr<Module*> m = parseBitcodeFile(fileBuf.get(), context);
    ExecutionEngine *ee = ExecutionEngine::create(m.get());

    Function* func = ee->FindFunctionNamed("main");
    std::cout << "hop " << m.get()  << " ee " << ee << " f " << func << std::endl;

    typedef void (*PFN)();
    PFN pfn = reinterpret_cast<PFN>(ee->getPointerToFunction(func));
    pfn();
    delete ee;
}

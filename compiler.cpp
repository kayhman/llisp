#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include <vector>
#include <string>
#include <iostream>
#include "cell.h"

using namespace llvm;

static std::map<std::string, Value*> NamedValues;

static Function *CreateFibFunction(Module *M, LLVMContext &Context) {
  // Create the fib function and insert it into module M. This function is said
  // to return an int and take an int parameter.
  Function *FibF =
    cast<Function>(M->getOrInsertFunction("fib", Type::getInt32Ty(Context),
                                          Type::getInt32Ty(Context),
                                          (Type *)0));

  // Add a basic block to the function.
  BasicBlock *BB = BasicBlock::Create(Context, "EntryBlock", FibF);

  // Get pointers to the constants.
  Value *One = ConstantInt::get(Type::getInt32Ty(Context), 1);
  Value *Two = ConstantInt::get(Type::getInt32Ty(Context), 2);

  // Get pointer to the integer argument of the add1 function...
  Argument *ArgX = FibF->arg_begin();   // Get the arg.
  ArgX->setName("AnArg");            // Give it a nice symbolic name for fun.

  // Create the true_block.
  BasicBlock *RetBB = BasicBlock::Create(Context, "return", FibF);
  // Create an exit block.
  BasicBlock* RecurseBB = BasicBlock::Create(Context, "recurse", FibF);

  // Create the "if (arg <= 2) goto exitbb"
  Value *CondInst = new ICmpInst(*BB, ICmpInst::ICMP_SLE, ArgX, Two, "cond");
  BranchInst::Create(RetBB, RecurseBB, CondInst, BB);

  // Create: ret int 1
  ReturnInst::Create(Context, One, RetBB);

  // create fib(x-1)
  Value *Sub = BinaryOperator::CreateSub(ArgX, One, "arg", RecurseBB);
  CallInst *CallFibX1 = CallInst::Create(FibF, Sub, "fibx1", RecurseBB);
  CallFibX1->setTailCall();

  // create fib(x-2)
  Sub = BinaryOperator::CreateSub(ArgX, Two, "arg", RecurseBB);
  CallInst *CallFibX2 = CallInst::Create(FibF, Sub, "fibx2", RecurseBB);
  CallFibX2->setTailCall();


  // fib(x-1)+fib(x-2)
  Value *Sum = BinaryOperator::CreateAdd(CallFibX1, CallFibX2,
                                         "addresult", RecurseBB);

  // Create the return instruction and add it to the basic block
  ReturnInst::Create(Context, Sum, RecurseBB);

  return FibF;
}


int compile()
{
  InitializeNativeTarget();
  
  llvm::LLVMContext & context = llvm::getGlobalContext();
  llvm::Module *module = new llvm::Module("elisp", context);
  llvm::IRBuilder<> builder(context);

  llvm::FunctionType *funcType = llvm::FunctionType::get(builder.getVoidTy(), false);
  llvm::Function *FibF = CreateFibFunction(module, context);


  llvm::Function *mainFunc = 
    llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module);
  llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entrypoint", mainFunc);
  builder.SetInsertPoint(entry);
  
  llvm::Value *helloWorld = builder.CreateGlobalStringPtr("hello world!\n");

  std::vector<llvm::Type *> putsArgs;
  putsArgs.push_back(builder.getInt8Ty()->getPointerTo());
  llvm::ArrayRef<llvm::Type*>  argsRef(putsArgs);

  llvm::FunctionType *putsType = 
    llvm::FunctionType::get(builder.getInt32Ty(), argsRef, false);
  llvm::Constant *putsFunc = module->getOrInsertFunction("puts", putsType);
  
  llvm::Constant *FibConst = module->getOrInsertFunction("fib", Type::getInt32Ty(context),
                                          Type::getInt32Ty(context),
                                          (Type *)0);
  
  Value *fArg = ConstantInt::get(Type::getInt32Ty(context), 1);
  
  builder.CreateCall(putsFunc, helloWorld);
  builder.CreateCall(FibConst, fArg);
  builder.CreateRetVoid();
  //module->dump();



 // Now we going to create JIT
  std::string errStr;
  ExecutionEngine *EE = EngineBuilder(module).setErrorStr(&errStr).setEngineKind(EngineKind::JIT).create();

  if (!EE) {
    std::cout << ": Failed to construct ExecutionEngine: " << errStr
           << "\n";
    return 1;
  }

  std::cout << "verifying... ";
  if (verifyModule(*module)) {
    std::cout << ": Error constructing function!\n";
    return 1;
  }


  Function* f = EE->FindFunctionNamed("fib");
  std::cout << "big " << f << std::endl;
  
  //    typedef std::function<int(int)> fibType;
  typedef int (*fibType)(int);
  fibType ffib = reinterpret_cast<fibType>(EE->getPointerToFunction(f));
    std::cout << "fib " << ffib(7) << std::endl;
}

llvm::Value* codegen(const Cell& cell, llvm::LLVMContext& context, 
		     llvm::IRBuilder<>& builder);

llvm::Value* codegen(const RealAtom& atom, llvm::LLVMContext& context,
		     llvm::IRBuilder<>& builder)
{
  return ConstantFP::get(context, APFloat(atom.real));
}

llvm::Value* codegen(const StringAtom& atom, llvm::LLVMContext& context,
		     llvm::IRBuilder<>& builder)
{
  return ConstantDataArray::getString(context, atom.val);
}

llvm::Value* codegen(const SymbolAtom& atom, llvm::LLVMContext& context,
		     llvm::IRBuilder<>& builder)
{
  return ConstantDataArray::getString(context, atom.val);
}

llvm::Value* codegen(const Sexp& sexp, llvm::LLVMContext& context, 
		     llvm::IRBuilder<>& builder)
{
  std::shared_ptr<Cell> fun = sexp.cells[0];
  std::cout << "codegen : " << fun->val << std::endl;
  if(fun->val.compare("+") == 0)
    {
      llvm::Value* sum = ConstantFP::get(context, APFloat(0.));
      for(int i = 1 ; i < sexp.cells.size() ; i++)
	sum = builder.CreateFAdd(sum, codegen(*sexp.cells[i], context, builder), "addtmp");
      std::cout << "---  sum " << std::endl;
      sum->dump();
      return sum;
    }
  return 0;
}

llvm::Value* codegen(const Cell& cell, llvm::LLVMContext& context, 
		     llvm::IRBuilder<>& builder)
{
  const SymbolAtom* symb = dynamic_cast<const SymbolAtom*>(&cell);
  const RealAtom* real = dynamic_cast<const RealAtom*>(&cell); 
  const StringAtom* string = dynamic_cast<const StringAtom*>(&cell);
  const Sexp* sexp = dynamic_cast<const Sexp*>(&cell);
 
  std::cout << "s  " << symb << " r " << real << " sx " << sexp << std::endl;

  if(symb)
    return codegen(*symb, context, builder);
  if(real)
    return codegen(*real, context, builder);
  if(string)
    return codegen(*string, context, builder);
  if(sexp)
    return codegen(*sexp, context, builder);

  return NULL;
}

llvm::Function* compileBody(const std::string& name, const Sexp& sexp, const std::vector<const Cell*> args, llvm::Module *module)
{
  llvm::LLVMContext& context = llvm::getGlobalContext();
  llvm::IRBuilder<> builder(context);

  std::vector<Type*> argsType;
  
  for(int i = 0 ; i < 2/* args.size()*/ ; i++)
    argsType.push_back(Type::getDoubleTy(context));
  
  FunctionType *FT = FunctionType::get(Type::getDoubleTy(context), argsType, false);

  Function* compiledF = cast<Function>(module->getOrInsertFunction(name,  FT));
  BasicBlock *RetBB = BasicBlock::Create(context, "return", compiledF);
  builder.SetInsertPoint(RetBB);
  
  Value* code = codegen(sexp, context, builder);
  builder.CreateRet(code);

  return compiledF;
}


llvm::Function* createCaller(Function* compiledF, const std::vector<const Cell*> args, llvm::Module *module)
{
  llvm::LLVMContext& context = llvm::getGlobalContext();
  llvm::IRBuilder<> builder(context);

  Function* execF = cast<Function>(module->getOrInsertFunction("execF", Type::getDoubleTy(context), (Type *)0));
  BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", execF);
  builder.SetInsertPoint(BB);
  
  std::vector<Value*> ArgsV;
  for (unsigned i = 0; i < args.size(); ++i) {
    ArgsV.push_back(codegen(*args[i], context, builder));
  }

  builder.CreateRet(builder.CreateCall(compiledF, ArgsV, "calltmp"));
    
  return execF;
}

extern "C" void registerCompilerHandlers(Cell::CellEnv& env)
{
  InitializeNativeTarget();
  
  llvm::LLVMContext& context = llvm::getGlobalContext();
  llvm::Module *module = new llvm::Module("elisp", context);
  llvm::IRBuilder<> builder(context);

  std::cout << "module : " << module << std::endl;
  module->dump();

  std::shared_ptr<Atom> compile = SymbolAtom::New(env, "compile");
  compile->closure = [module](Sexp* sexp, Cell::CellEnv& env) {
    const std::string& fname = sexp->cells[1]->val;
    auto clIt = env.func.find(fname);
    if(clIt != env.func.end())
      if(auto fun = static_cast<SymbolAtom*>(clIt->second.get()))
	{
	  std::cout << "compile fonction " << sexp->cells.size()-1 << " in module " << module << std::endl;
	  std::vector<const Cell*> args;
	  for(int i = 1 ; i < sexp->cells.size() ; i++)
	    args.push_back(sexp->cells[i].get());
	  Function* bodyF = compileBody(fname, dynamic_cast<Sexp&>(*fun->code.get()), args, module);
	  Function* callerF = createCaller(bodyF, args, module);
	   // Now we going to create JIT
	  std::string errStr;
	  ExecutionEngine *EE = EngineBuilder(module).setErrorStr(&errStr).setEngineKind(EngineKind::JIT).create();

	  if (!EE) {
	    std::cout << ": Failed to construct ExecutionEngine: " << errStr
		      << "\n";
	  }
	  std::cout << "verifying... ";
	  if (verifyModule(*module)) {
	    std::cout << ": Error constructing function!\n";

	  }
	  
	  Function* f = EE->FindFunctionNamed(fname.c_str());
	  std::cout << "func " << f << std::endl;
	  Function* ff = EE->FindFunctionNamed("execF");
	  std::cout << "execF " << ff << std::endl;



	  typedef int (*fibType)(int, int);
	  fibType func = reinterpret_cast<fibType>(EE->getPointerToFunction(f));
	  std::cout << "func " << func(5, 3) << std::endl;
	  
	  typedef double (*ExecF)();
	  ExecF execF = reinterpret_cast<ExecF>(EE->getPointerToFunction(ff));
	  std::cout << "execF " << execF() << std::endl;
	  

	  module->dump();
	  return fun->code;
	}
    return sexp->cells[0];
  };
}

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
  Value* V = NamedValues[atom.val];
  return V;// ? V : ErrorV("Unknown variable name");
  //return ConstantDataArray::getString(context, atom.val);
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

llvm::Function* compileBody(const std::string& name, const Sexp& body, const std::vector<const Cell*> args, llvm::Module *module)
{
  NamedValues.clear();
  llvm::LLVMContext& context = llvm::getGlobalContext();
  llvm::IRBuilder<> builder(context);

  std::vector<Type*> argsType;
  
  for(int i = 0 ; i < 2/* args.size()*/ ; i++)
    argsType.push_back(Type::getDoubleTy(context));
  
  FunctionType *FT = FunctionType::get(Type::getDoubleTy(context), argsType, false);

  Function* compiledF = cast<Function>(module->getOrInsertFunction(name,  FT));

  unsigned Idx = 0;
  for (Function::arg_iterator AI = compiledF->arg_begin(); Idx != args.size(); ++AI, ++Idx) {
    AI->setName(args[Idx]->val);
    
    // Add arguments to variable symbol table.
    std::cout << "***** > add args " << args[Idx]->val << std::endl;
    NamedValues[args[Idx]->val] = AI;
  }


  BasicBlock *RetBB = BasicBlock::Create(context, "return", compiledF);
  builder.SetInsertPoint(RetBB);
  
  Value* code = codegen(body, context, builder);
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

  std::cout << "  #args " << args.size() << std::endl;
  std::vector<Value*> ArgsV;
  for (unsigned i = 0; i < args.size(); ++i) {
    std::cout << " ---- > add " << i << std::endl;
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
	  const Sexp* sArgs = dynamic_cast<const Sexp*>(fun->args.get());
	  for(int i = 0 ; i <  sArgs->cells.size(); i++)
	    args.push_back(sArgs->cells[i].get());
	  Function* bodyF = compileBody(fname, dynamic_cast<Sexp&>(*fun->code.get()), args, module);
	  
	  
	  std::cout << "--------------------" << std::endl;
	  module->dump();
	  std::cout << "--------------------" << std::endl;



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

	  typedef int (*fibType)(int, int);
	  fibType func = reinterpret_cast<fibType>(EE->getPointerToFunction(f));
	  std::cout << "func " << func(5, 3) << std::endl;
	  

	  //replace evaluated closure by compiled code
	  fun->closure = [env, bodyF, module](Sexp* self, Cell::CellEnv& dummy) mutable {
	    std::vector<const Cell*> args;
	    for(int i = 1 ; i < self->cells.size() ; i++)
	      args.push_back(self->cells[i].get());

	    Function* callerF = createCaller(bodyF, args, module);

	    module->dump();

	    std::string errStr;
	    ExecutionEngine *EE = EngineBuilder(module).setErrorStr(&errStr).setEngineKind(EngineKind::JIT).create();
	    Function* ff = EE->FindFunctionNamed("execF");
	    std::cout << "execF " << ff << std::endl;
	    
	    typedef double (*ExecF)();
	    ExecF execF = reinterpret_cast<ExecF>(EE->getPointerToFunction(ff));

	    double out = execF();
	    std::cout << "execF " << out << std::endl;
	    
	    std::shared_ptr<Cell> res(RealAtom::New());
	    res->real = out;

	    return res;
	  };

	  return fun->code;
	}
    return sexp->cells[0];
  };
}

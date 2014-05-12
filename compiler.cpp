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
#include <sstream>
#include "cell.h"

using namespace llvm;

static std::map<std::string, Value*> NamedValues;

llvm::Value* codegen(const Cell& cell, llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module *module);

llvm::Value* codegen(const RealAtom& atom, llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module *module)
{
  return ConstantFP::get(context, APFloat(atom.real));
}

llvm::Value* codegen(const StringAtom& atom, llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module *module)
{
  return ConstantDataArray::getString(context, atom.val);
}

llvm::Value* codegen(const SymbolAtom& atom, llvm::LLVMContext& context, llvm::IRBuilder<>& builder,llvm::Module *module)
{
  Value* V = NamedValues[atom.val];
  return V;// ? V : ErrorV("Unknown variable name");
  //return ConstantDataArray::getString(context, atom.val);
}

llvm::Value* codegen(const Sexp& sexp, llvm::LLVMContext& context, 
		     llvm::IRBuilder<>& builder, llvm::Module *module)
{
  std::shared_ptr<Cell> fun = sexp.cells[0];
  if(fun->val.compare("+") == 0)
    {
      llvm::Value* sum = codegen(*sexp.cells[1], context, builder, module);
      for(int i = 2 ; i < sexp.cells.size() ; i++)
	sum = builder.CreateFAdd(sum, codegen(*sexp.cells[i], context, builder, module), "addtmp");
      return sum;
    }

  if(fun->val.compare("-") == 0)
    {
      llvm::Value* diff = codegen(*sexp.cells[1], context, builder, module);
      for(int i = 2 ; i < sexp.cells.size() ; i++)
	diff = builder.CreateFSub(diff, codegen(*sexp.cells[i], context, builder, module), "difftmp");
      return diff;
    }

  if(fun->val.compare("*") == 0)
    {
      llvm::Value* prod = codegen(*sexp.cells[1], context, builder, module);
      for(int i = 2 ; i < sexp.cells.size() ; i++)
	prod = builder.CreateFMul(prod, codegen(*sexp.cells[i], context, builder, module), "addprod");
      return prod;
    }

  if(fun->val.compare("/") == 0)
    {
      llvm::Value* div = codegen(*sexp.cells[1], context, builder, module);
      for(int i = 2 ; i < sexp.cells.size() ; i++)
	div = builder.CreateFDiv(div, codegen(*sexp.cells[i], context, builder, module), "divquot");
      return div;
    }


  if(fun->val.compare("<") == 0)
    {
      Value* V0 = codegen(*sexp.cells[1], context, builder, module);
      Value* V1 = codegen(*sexp.cells[2], context, builder, module);
      V0 = builder.CreateFCmpULT(V0, V1, "cmptmp");
      // Convert bool 0/1 to double 0.0 or 1.0
      return builder.CreateUIToFP(V0, Type::getDoubleTy(context), "booltmp");
    }

  if(fun->val.compare("if") == 0)
    {
      Value* cond = codegen(*sexp.cells[1], context, builder, module);

      if (cond == 0) return 0;
      
      // Convert condition to a bool by comparing equal to 0.0.
      cond = builder.CreateFCmpONE(cond, ConstantFP::get(context, APFloat(0.0)), "ifcond");
      
      Function *TheFunction = builder.GetInsertBlock()->getParent();
      
      // Create blocks for the then and else cases.  Insert the 'then' block at the
      // end of the function.
      BasicBlock *thenBB = BasicBlock::Create(context, "then", TheFunction);
      BasicBlock *elseBB = BasicBlock::Create(context, "else");
      BasicBlock *mergeBB = BasicBlock::Create(context, "ifcont");
      
      builder.CreateCondBr(cond, thenBB, elseBB);
      
      // Emit then value.
      builder.SetInsertPoint(thenBB);
      
      Value *thenV = codegen(*sexp.cells[2], context, builder, module);
      if (thenV == 0) return 0;
      
      builder.CreateBr(mergeBB);
      // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
      thenBB = builder.GetInsertBlock();
      
      // Emit else block.
      TheFunction->getBasicBlockList().push_back(elseBB);
      builder.SetInsertPoint(elseBB);
      
      Value *elseV = codegen(*sexp.cells[3], context, builder, module);
      if (elseV == 0) return 0;
      
      builder.CreateBr(mergeBB);
      // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
      elseBB = builder.GetInsertBlock();
      
      // Emit merge block.
      TheFunction->getBasicBlockList().push_back(mergeBB);
      builder.SetInsertPoint(mergeBB);
      PHINode *PN = builder.CreatePHI(Type::getDoubleTy(context), 2, "iftmp");
      
      PN->addIncoming(thenV, thenBB);
      PN->addIncoming(elseV, elseBB);
      return PN;
    }

  if(SymbolAtom* symbol = dynamic_cast<SymbolAtom*>(fun.get()))
    {
      Function* F = module->getFunction(symbol->val);

      std::vector<Value*> ArgsV;
      for (unsigned i = 1; i < sexp.cells.size(); ++i) {
	ArgsV.push_back(codegen(*sexp.cells[i], context, builder, module));
      }
      
      CallInst *call = builder.CreateCall(F, ArgsV, "calltmp");
      call->setTailCall();
      return call;
      //return builder.CreateRet(call);
      }

  return 0;
}

llvm::Value* codegen(const Cell& cell, llvm::LLVMContext& context, 
		     llvm::IRBuilder<>& builder, llvm::Module *module)
{
  const SymbolAtom* symb = dynamic_cast<const SymbolAtom*>(&cell);
  const RealAtom* real = dynamic_cast<const RealAtom*>(&cell); 
  const StringAtom* string = dynamic_cast<const StringAtom*>(&cell);
  const Sexp* sexp = dynamic_cast<const Sexp*>(&cell);
 
  if(symb)
    return codegen(*symb, context, builder, module);
  if(real)
    return codegen(*real, context, builder, module);
  if(string)
    return codegen(*string, context, builder, module);
  if(sexp)
    return codegen(*sexp, context, builder, module);

  return NULL;
}

llvm::Function* compileBody(const std::string& name, const Sexp& body, const std::vector<const Cell*> args, llvm::Module *module)
{
  NamedValues.clear();
  llvm::LLVMContext& context = llvm::getGlobalContext();
  llvm::IRBuilder<> builder(context);

  std::vector<Type*> argsType;
  
  for(int i = 0 ; i < args.size() ; i++)
    {
	argsType.push_back(Type::getDoubleTy(context));
    }
  
  FunctionType *FT = FunctionType::get(Type::getDoubleTy(context), argsType, false);

  Function* compiledF = cast<Function>(module->getOrInsertFunction(name,  FT));

  unsigned Idx = 0;
  for (Function::arg_iterator AI = compiledF->arg_begin(); Idx != args.size(); ++AI, ++Idx) {
    AI->setName(args[Idx]->val);
    
    // Add arguments to variable symbol table.
    NamedValues[args[Idx]->val] = AI;
  }


  BasicBlock *RetBB = BasicBlock::Create(context, "return", compiledF);
  builder.SetInsertPoint(RetBB);
  
  Value* code = codegen(body, context, builder, module);
  builder.CreateRet(code);

  return compiledF;
}


llvm::Function* createCaller(const std::string& name, Function* compiledF, const std::vector<std::shared_ptr<Cell> > args, llvm::Module *module)
{
  llvm::LLVMContext& context = llvm::getGlobalContext();
  llvm::IRBuilder<> builder(context);

  Function* execF = cast<Function>(module->getOrInsertFunction(name.c_str(), Type::getDoubleTy(context), (Type *)0));
  if(execF)
    execF->removeFromParent();

  execF = cast<Function>(module->getOrInsertFunction(name.c_str(), Type::getDoubleTy(context), (Type *)0));  

  BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", execF);
  builder.SetInsertPoint(BB);

  std::vector<Value*> ArgsV;
  for (unsigned i = 0; i < args.size(); ++i) {
    ArgsV.push_back(codegen(*args[i], context, builder, module));
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

  //module->dump();

  std::shared_ptr<Atom> compile = SymbolAtom::New(env, "compile");
  compile->closure = [module](Sexp* sexp, Cell::CellEnv& env) {
    const std::string& fname = sexp->cells[1]->val;
    auto clIt = env.func.find(fname);
    if(clIt != env.func.end())
      if(auto fun = static_cast<SymbolAtom*>(clIt->second.get()))
	{
	  std::vector<const Cell*> args;
	  const Sexp* sArgs = dynamic_cast<const Sexp*>(fun->args.get());
	  for(int i = 0 ; i <  sArgs->cells.size(); i++)
	    args.push_back(sArgs->cells[i].get());
	  Function* bodyF = compileBody(fname, dynamic_cast<Sexp&>(*fun->code.get()), args, module);
	  
	  
	  //std::cout << "--------------------" << std::endl;
	  //module->dump();
	  //std::cout << "--------------------" << std::endl;

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

	  typedef int (*fibType)(int, int);
	  //	  fibType func = reinterpret_cast<fibType>(EE->getPointerToFunction(f));

	  //replace evaluated closure by compiled code
	  fun->closure = [env, fname, bodyF, module](Sexp* self, Cell::CellEnv& dummy) mutable {
	    std::vector<std::shared_ptr<Cell> > args(self->cells.begin()+1, self->cells.end());  
	    std::stringstream ss;
	    ss << fname << "_call";
	    Function* callerF = createCaller(ss.str(), bodyF, args, module);

	    //	    module->dump();

	    std::string errStr;
	    ExecutionEngine *EE = EngineBuilder(module).setErrorStr(&errStr).setEngineKind(EngineKind::JIT).create();
	    	    
	    typedef double (*ExecF)();
	    ExecF execF = reinterpret_cast<ExecF>(EE->getPointerToFunction(callerF));

	    std::shared_ptr<Cell> res(RealAtom::New());
	    res->real = execF();

	    return res;
	  };

	  return fun->code;
	}
    return sexp->cells[0];
  };
}

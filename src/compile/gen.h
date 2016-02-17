#pragma once


#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ConstantFolder.h"

#include "../global.h"
#include "../core/type.h"


namespace def {
    namespace core {
        class AST;
    }
}


namespace def {
namespace compile {

using namespace std;
using namespace llvm;
using namespace def::core;


class Gen{

public:

    LLVMContext & context;
    IRBuilder<> builder;
    Module module;
    Function * main;
    BasicBlock * entry;

    Gen(LLVMContext & con, const string & m = "main")
        : context(con)
        , builder(con)
        , module(m, con)
    {
        FunctionType *fty = FunctionType::get(builder.getVoidTy(), false);
        main = Function::Create(fty, Function::ExternalLinkage, "main", &module);
        entry = BasicBlock::Create(context, "entry", main);
        builder.SetInsertPoint(entry);
    }

    // codegen 完成后的收尾工作
    void finish() {
        // builder.SetInsertPoint(entry);
        builder.CreateRetVoid();
    }

    ~Gen(){
        // delete main;
        // delete entry;
    }

    // Type 类型映射
    llvm::Type* fixType(
        def::core::Type*, 
        vector<def::core::Type*>* append=nullptr // 追加的类型参数
       );
    llvm::Type* fixBuiltinFunctionType(def::core::TypeFunction*);

    // 通用处理
    Value* createLoad(AST*);
    Value* createLoad(Value*);
    Value* varyPointer(AST*); // 将 AST 处理成可以作为函数参数使用
    Value* varyPointer(Value*);

    Function* createFunction(AST*);    

    // 变量操作
    Value* getValue(const string &);
    Value* putValue(const string &, Value*); // 返回旧变量


public:
    
    // 当前可见作用域创建的变量
    map<string, Value*> values;

};




} // --end-- namespace compile
} // --end-- namespace def

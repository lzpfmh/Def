/**
 * Def 虚拟机（解释器）
 */

#include <iostream>

#include "vm.h"

using namespace std;

using namespace def::token;
using namespace def::node;
using namespace def::object;
using namespace def::stack;
using namespace def::vm;



Vm::Vm(){
    vm_stack = new Stack(); //新建执行栈
}

/**
 * 运行 Def 语言脚本
 * @return 表示运行成功或失败
 */
bool Vm::Eval(string txt, bool ispath=false)
{
    // 词法分析
    vector<Word> words; // 词法分析结果
    Tokenizer T(ispath, txt, words); // 初始化词法分析器
    T.Scan(); // 执行词法分析

    // 语法分析
    Nodezer N(words); // 初始化语法分析器
    Node *node = N.BuildAST(); // 解析得到语法树（表达式）

    // 解释执行分析树
    bool done = Execute(node);

    /*
    cout <<
    node->Child(2)->Right()->GetName()
    << endl;
    */
    
    

    delete node; // 析构语法树
    
	return true;
}


/**
 * 解释执行 Def 语法树
 * @return 表示解释成功或失败
 */
bool Vm::Execute(Node *p)
{
    // 组合表达式 TypeNode::Group
    size_t i=0;
    while(1){
        Node *e = p->Child(i);
        if(!e) break;
        //cout<<i<<endl;
        GetValue( e );
        i++;
    }

    return true;
}


/**
 * 对语法节点进行求值操作
 */
DefObject* Vm::GetValue(Node* n)
{

#define T TypeNode

    T t = n->type; //当前节点类型

    if(t==T::Assign){ // 赋值

        DefObject *rv = GetValue(n->Right()); //等号右值
        vm_stack->Put( // 变量入栈
            n->Left()->GetName() ,
            rv
        );
        return rv;

    }else if(t==T::Variable){ // 通过名字取得变量值

        return vm_stack->Get(n->GetName());

    }else if(t==T::Print){ // print 打印

        //cout<<"print!!!"<<endl;
        return OperatePrint(
            GetValue(n->Right())
        ); // 打印

    }else if(t==T::None){
    

    }else if(t==T::Add){ // + 加法操作

        return OperateAdd(
            GetValue(n->Left()), 
            GetValue(n->Right())
        );

    }else if(t==T::Int){ // 整数求值

        return new ObjectInt(n->GetInt());

    }else{

    }

#undef T   // TypeNode

}


#define T ObjType


/**
 * 加法操作
 */
DefObject* Vm::OperateAdd(DefObject *l, DefObject *r)
{
    T lt = l->type;
    T rt = r->type;

    // 整数相加
    if( lt==T::Int && lt==T::Int ){
        return new ObjectInt(
            ((ObjectInt*)l)->value + 
            ((ObjectInt*)r)->value
        );

    }else{

    }

}

/**
 * 加法操作
 */
DefObject* Vm::OperatePrint(DefObject *obj)
{

    T t = obj->type; // 获取类型

    if( t==T::Int ){ // 整数

        cout << ((ObjectInt*)obj)->value << endl;

    }else{

    }

    return obj;
}


#undef T   // ObjType


/****** 脚本解释器测试 ******/


int main()
{
    //cout << "\n";

    Vm v = Vm(); // 初始化引擎
    v.Eval("./test.d", true);

    //cout << "\n\n";
}


/****** 测试结束 ******/
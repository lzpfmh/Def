/**
 * 抽象语法树解析抽象语法树解析
 */
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <vector>

#include "nodezer.h"

#define S Token::State
#define T NodeType

using namespace std;

using namespace def::token;
using namespace def::node;

// Log::log
#define ERR(str) cerr<<str<<endl;exit(1);

/**
 * 构造
 */
Nodezer::Nodezer(vector<Word>*w, string file=""):
    words(w),
    filepath(file)
{
    Clear();
}



/**
 * 判断第一个参数（类型）是否为后面的类型任意之一
 */
bool Nodezer::IsType(T c,
    T t0=T::End, T t1=T::End, T t2=T::End, T t3=T::End, T t4=T::End,
    T t5=T::End, T t6=T::End, T t7=T::End, T t8=T::End, T t9=T::End)
{
    if(c==T::End){ // 不可对比End
        return false;
    }
    if(c==t0||c==t1||c==t2||c==t3||c==t4||
       c==t5||c==t6||c==t7||c==t8||c==t9){
        return true;
    }else{
        return false;
    }
}



/**
 * 判断当前的节点类型
 */
NodeType Nodezer::CurNodeType()
{
    cnt = GetNodeType(cur);
    return cnt;
}


/**
 * 从当前单词获得当前的节点类型
 */
NodeType Nodezer::GetNodeType(Word &cwd)
{
    S s = cwd.type;
    string v = cwd.value;
    //S nt = next.type;
    //string nv = next.value;

    // cout<<"GetNodeType: "<<(int)s<<"->"<<v<<endl;

    if(s==S::Variable){
        return T::Variable; // 变量名

#define ST_ELSE_IF(name) }else if(s==S::name){ return T::name;
     ST_ELSE_IF(Int)  
     ST_ELSE_IF(Float)
     ST_ELSE_IF(String)
     ST_ELSE_IF(FuncCall) // 函数调用
     ST_ELSE_IF(ProcCall) // 处理器调用
     ST_ELSE_IF(ContainerAccess) // 容器访问
#undef ST_ELSE_IF

    }else if(s==S::Keyword){ // 关键字

        if(v=="none"){
            return T::None;
        }else if(v=="true"||v=="false"){
            return T::Bool;

        }else if(v=="def"){
            return T::ProcDefine;
        }else if(v=="defun"){
            return T::FuncDefine;

        }else if(v=="print"){
            return T::Print;
        }else if(v=="if"){
            return T::If;
        }else if(v=="while"){
            return T::While;
        }

    }else if(s==S::Sign){ // 符号

        if(v==":"){
            return T::Assign; //赋值 :
        }else if(v=="("){
            return T::List; //列表or优先级 (
        }else if(v=="["){
            return T::Dict; //字典 [
        }else if(v=="{"){
            return T::Block; //块 {

        }else if(v=="+"){
            return T::Add; // 加 +
        }else if(v=="-"){
            return T::Sub; // 减 -
        }else if(v=="*"){
            return T::Mul; // 乘 *
        }else if(v=="/"){
            return T::Div; // 除 /

        }else if(v=="."){
            return T::MemberAccess; // 成员访问
        }
    }else if(s==S::End){

        return T::End; // 终止符
    }


    // 无匹配
    return T::Normal;

}



/**
 * 取得节点的优先级
 */
int Nodezer::GetPriority(Node*p)
{
    if(p==NULL){
        return 0;
    }

    switch(p->type){

#define N(name, priori) 
#define D(name, priori) case T::name: return priori;
    NODELIST(N, D)
#undef N
#undef D

    }

    return 0;
}



// 节点类型定义
#define TN_VALUE T::Variable,T::None,T::Bool,T::Int,T::Float,T::String
#define TN_AS T::Add,T::Sub
#define TN_MD T::Mul,T::Div
#define TN_ASMD TN_AS,TN_MD
#define IS_SIGN(str) cur.type==S::Sign&&cur.value==str
#define IS_NO_SIGN(str) cur.type!=S::Sign||cur.value!=str
#define IS_KEYWORD(str) cur.type==S::Keyword&&cur.value==str
#define IS_NO_KEYWORD(str) cur.type!=S::Keyword||cur.value!=str




/**
 * 从当前单词新建节点
 */
Node* Nodezer::CreatNode()
{
    //Read();
    T cnt = CurNodeType();
    //Move(1);

    // cout<<"CreatNode: "<<(int)cnt<<"->"<<cur.value<<endl;

    switch(cnt){

#define N(name, priori) 
#define D(name, priori) case T::name: return new Node##name(cur);
    NODELIST(N, D)
#undef N
#undef D


    }

    return NULL;
}


/**
 * 解析延展部分节点
 */
Node* Nodezer::ParseNode(Node*p1=NULL, Node*p=NULL)
{
    if(p==NULL){ 
        return NULL; //表达式完成
    }

    T t = p->type;

    // 列表 or 优先级 or 函数调用
    if( t==T::List ){
        // cout << "-List or Priority or FuncCall-" << endl;
        if(p1 && p1->type==T::FuncCall){
            delete p;
            Move(1); //jump (
            Node* g = Group(); 
            Move(1); //jump )
            return g; // 返回函数调用右节点
        }
        // 列表or优先级
        NodePriority* pp = new NodePriority(cur);
        Move(1); //jump (
        Node *e = Express();
        if(e && IS_SIGN(")")){ //优先级
            delete p;
            if(!GetPriority(e)){
                // 不影响优先级计算
                delete pp;
                Move(1); // jump )
                return e;
            }
            pp->AddChild(e);
            Move(1); // jump )
            return pp;
        }
        if(e) p->AddChild(e);
        while(1){
            if(IS_SIGN(",")){
                Move(1); continue;
            }
            if(IS_SIGN(")")){
                Move(1); // jump )
                break; //列表结束
            }
            Node *e = Express();
            if(!e){
                ERR("Err: list didn't end with ) sign !");
            }
            p->AddChild(e);
        }
        delete pp;
        return p; // 列表结束

    // 字典 or 容器访问
    }else if( t==T::Dict ){

        // cout << "-Dict or ContainerAccess-" << endl;
        Move(1); // jump [
        if(p1 && p1->type==T::ContainerAccess){
            delete p;
            Node* g = Group(); 
            Move(1); //jump ]
            return g; // 返回容器调用又节点
        }
        // 字典
        while(1){
            if(IS_SIGN("]")){
                break; //字典结束
            }
            Node *e = Express();
            if(!e){
                ERR("Err: dict parse , IS_SIGN no }");
            }
            p->AddChild(e);
        }
        Move(1); //jump }
        return p;

    // 块 or 处理器调用
    }else if( t==T::Block ){
        //cout << "-Block or ProcCall-" << endl;
        if(p1 && p1->type==T::ProcCall){
            delete p;
            Move(1); //jump (
            Node* g = Group(); 
            Move(1); //jump )
            return g; // 返回处理器调用右节点
        }
        // 块
        Move(1); //jump {
        while(1){
            if(IS_SIGN("}")){
                break; // 块结束
            }
            Node *e = Express();
            if(!e){
                ERR("Err: block parse , IS_SIGN no }");
            }
            p->AddChild(e);
        }
        Move(1); //jump }
        return p;

    // 处理器定义
    }else if( t==T::ProcDefine ){
        // cout << "-ProcDefine-" << endl;
        Move(1); // jump def
        if(cur.type==S::Variable){
            string name = cur.value;
            Move(1); // jump name
            if(cur.type==S::ProcCall){
                p->SetName( name ); //处理器名
                Move(1); // jump ProcCall
            }else{
                Move(-1); //复位
            }
        }
        if(IS_SIGN("{")){ // 处理器参数
            Move(1); // jump {
            p->SetArgv( Group() );
            Move(1); // jump }
        }
        p->SetBody( Group() );
        Move(1); // jump ;
        return p;

    // 函数定义
    }else if( t==T::FuncDefine ){
        // cout << "-FuncDefine-" << endl;
        Move(1); // jump def
        if(cur.type==S::FuncCall){
            p->SetName( cur.value ); //处理器名
            Move(1); // jump ProcCall
        }
        if(IS_SIGN("(")){ // 处理器参数
            Move(1); // jump {
            p->SetArgv( Group() );
            Move(1); // jump }
        }
        p->SetBody( Group() );
        Move(1); // jump ;
        return p;

    // 条件分支
    }else if( t==T::If ){

        //cout << "-If-" << endl;
        Move(1); // jump if
        // 开始构建 If 块结构
        Node *gr = new NodeGroup(cur);
        while(1){

            if(IS_KEYWORD("elif")){
                p->AddChild(gr);
                gr = new NodeGroup(cur);
                Move(1); // jump elif
                continue;
            }else if(IS_KEYWORD("else")){
                p->AddChild(gr);
                gr = new NodeGroup(cur);
                gr->AddChild(NULL); //第一个子节点为空节点，表示为 else 节点
                Move(1); // jump else
                continue;
            }else if(IS_SIGN(";")){
                p->AddChild(gr);
                gr = new NodeGroup(cur);
                Move(1); // jump ;
                break; //If结构结束
            }
            //添加表达式
            gr->AddChild( Express() );

        }//end while

        return p;

    }else if( t==T::While ){

        //cout << "-While-" << endl;
        //Node *wh = new NodeWhile(cur);
        Move(1); // jump while
        while(1){
            if(IS_SIGN(";")){
                break; //If结构结束
            }
            //添加表达式
            p->AddChild( Express() );
        }//end while
        Move(1); // jump ;
        return p;


    // 打印
    }else if( t==T::Print ){

        //cout << "-Print-" << endl;
        Move(1); // jump print
        p->AddChild( Express() );
        return p;


    /*
    }else if(t==T::Sub ){ //是否为负数

        if( !p1 || !p->Left() ){ //负数加前缀0
            Node* n = new NodeInt(cur);
            n->SetInt(0);
            p->Left(n);
        }
    */

    }

    // cout << "-ParseNode  p->type  nothing-" << endl;
    // 当前节点不需要扩展 跳到下一个
    Move(1); //下一个
    return p;
}




/**
 * 按优先级组合语法节点
 * down 是否为上级运算组合下降
 */
Node* Nodezer::AssembleNode(Node*p1, Node*p2, bool down=false)
{

    int s1 = GetPriority(p1);
    int s2 = GetPriority(p2);

    // cout<<"s1="<<s1<<", s2="<<s2<<endl;
    
    if(down && s1 && s2 && s2<=s1){ //上级下降
        //cout << " down && s1 && s2 && s2<s1 " << endl;
        return NULL; //下降完成返回

    }else if(s1 && s2 && s2<=s1){ // 左结合
        //cout << " s1 && s2 && s2<=s1 " << endl;
        p2->Left( p1 );
        return p2;

    }else if( s1 && s2>s1 ){ // 优先级调整
        //cout << " s1 && s2>s1 " << endl;
        Node *p1r = p1->Right();
        if(!p1r&&p2->type!=T::Sub){
            ERR("ERR: !p1r ");
        }
        p2->Left( p1r );
        Move(1); // 下一个参与组合
        p1->Right( Express( p2, true ) ); //递归下降
        Move(-1); // 下降返回后需要回退，以便于上级节点重新组合 
        return p1;

    }else if( !s1 && s2 ){ // 左叶
        //cout << " !s1 && s2 " << endl;
        p2->Left( p1 );
        return p2;

    }else if( s1 && !s2 ){ // 右叶
        //cout << " s1 && !s2 " << endl;
        Node *p1r = p1->Right();
        if(p1r){
            return NULL; // 表达式完成
        }
        p1->Right( p2 );
        return p1;
    }

    // 无法组装节点  表达式完成
    return NULL;

}





/**
 * 建立语法表达式
 */
Node* Nodezer::Express(Node *p1, bool down)
{
    while(1){

        Node *p2;
        if(nodebuf){ // 获取上一步缓存节点
            p2 = nodebuf; nodebuf = NULL;
        }else{
            p2 = CreatNode(); //新建
        }

        if(cur.type==S::End){ // 全部结束
            return p2 ? p2 : p1;
        }

        if(!p2){ //表达式完成
            return p1;
        }

        if(!p1){
            p1 = ParseNode(p1, p2); //延展完善
            continue; // 获取下一个
        }

        // cout<<"p1="<<(int)p1->type<<", p2="<<(int)p2->type<<endl;

        // 按优先级组合节点
        Node* pn = AssembleNode(p1, p2, down);
        //cout << "  Node* pn =  " <<pn<< endl;
        if(pn){ // 可以组合
            p2 = ParseNode(p1, p2); // 延展完善节点
            p1 = pn;
            continue; //优先级组合成功，下一步组合
        }

        // 节点无法组合
        //cout <<"DeleteNode:"<<endl;
        nodebuf = p2; //缓存新建的节点，下一步重新开始
        return p1;

    }



}



/**
 * 扫描单词 构建单条表达式
 * @param pp 上级递归父节点
 * @param tt 上级递归父节点类型
 *
Node* Nodezer::Express(Node *pp=NULL, T tt=T::Normal)
{

    //T t = T::Normal; // 记录状态
    //string cv = cur.value; // 当前值
    Node *p = pp;
    T t = tt;
    T c = cnt;
    size_t old_i = 0;


    while(t!=T::End){

        if(i==0||old_i!=i){ // 已经move至下一个，重新读取词
            Read();
            c = CurNodeType(); // 当前类型
            //cout<<(int)c<<"->"<<cur.value<<endl;
            old_i = i;
        }else{ // 未 move，无需重新读取
            t = c;
        }

        // cout<<(int)c<<"->"<<cur.value<<endl;

        // 正式开始循环处理

        //// Normal
        if(t==T::Normal){ //默认状态

            //cout << "-Normal-" << endl;
            if( IsType(c,T::Print,T::If,T::While) ){
                //cout << "Normal continue !" << endl;
                t = c;
                continue;

            }else if( IsType(c,TN_VALUE) ){
                p = CreatNode(1);
                t = c;

            // [] 列表数据结构
            }else if(IS_SIGN("[")){
                //cout << "[] continue !" << endl;
                c = T::List;
                continue;

            }else{
                return p;
            }

        //// Variable None Bool Int Float String
        }else if( IsType(t,TN_VALUE) ){

            // cout << "-TN_VALUE-" << endl;

            if( t==T::Variable && c==T::ContainerAccess ){ // 容器访问 []
                p = CreatNode(1, p);
                Node *r = Express(); // 一条表达式 作为访问索引
                if(IS_SIGN("]")) ERR("err: ]"); //错误处理
                Move(1); //跳过右方括号 ]
                p->Right(r);
            }else if( t==T::Variable && c==T::FuncCall ){ // 函数调用 ()

                string name = p->GetName();
                p = CreatNode(1); // 右节点参数列表
                p->SetName(name); // 设置函数名称
                Node* paralist = Group();
                p->Right(paralist); // 设置参数列表
            
            }else if( t==T::Variable && c==T::Assign ){ // 变量赋值
                continue;

            // 运算
            }else if( IsType(c,TN_ASMD) ){
                p = CreatNode(1, p);
                t = c;
            }else{

                // cout << "}else{" << endl;
                // cout << cur.value << endl;
                return p; // 表达式结束
            }


        //// Add Sub
        }else if( IsType(t,TN_AS) ){ // 加法 减法 + -

            //cout << "-Add,Sub-" << endl;
            if( IsType(c,TN_VALUE) ){
                if(p->Right()){
                    //已经存在右节点
                    return p;
                }
                p->Right(CreatNode(1));
            // 同级左结合算符 + -
            }else if( IsType(c,TN_AS) ){
                p = CreatNode(1, p);
                t = c;
            // 优先算符 * /
            }else if( IsType(c,TN_MD) ){
                Node *pn = CreatNode(1);
                Node *r = p->Right();
                pn->Left(r);
                pn = Express(pn, c);
                p->Right(pn);
            }else{ 
                return p;
            }

        //// Mul Div
        }else if( IsType(t,TN_MD) ){ // 乘法 除法 * /

            //cout << "-Mul,Div-" << endl;
            if( IsType(c,TN_VALUE) ){
                if(p->Right()){
                    //已经存在右节点
                    return p;
                }
                p->Right(CreatNode(1));
            // 同级左结合算符
            }else if( IsType(c,TN_MD) || 
                //优先级低的算符且不是上一层递归传入
                !pp && IsType(c,TN_AS)
            ){
                p = CreatNode(1, p);
                t = c;
            }else{
                return p;
            }


        // FuncCall
        }else if( t==T::FuncCall ){

            //cout << "-FuncCall-" << endl;
            NodeFuncCall* fp = CreatNode(1);
            Move(1); // 跳过左括号 (
            fp->Right( Group() ); // 设置参数列表
            if(IS_SIGN(")")) ERR("err: )"); //错误处理
            if(p!=NULL){ // 表达式
                p->Right(fp);
                t ＝ p->type; // 恢复状态
            }else{
                return fp;
            }

        // Assign
        }else if( t==T::Assign ){

            //cout << "-Assign-" << endl;
            p = CreatNode(1, p);
            // 赋值算法后面的所有内容都将被赋值给左边的变量
            p->Right( Express() );
            return p;

        // While
        }else if( t==T::While ){

            //cout << "-While-" << endl;
            p = CreatNode(1);
            NodeGroup* g = new NodeGroup(cur);
            while(1){
                if(IS_SIGN(";")){
                    size_t s = g->ChildSize();
                    Node *nl = NULL
                        ,*nr = NULL;
                    if(s==0){
                    }else if(s==1){
                        nl = g->Child(0); // 第一个表达式作为 while 条件
                    }else if(s>=2){
                        nl = g->ChildPop(0);
                        nr = g;
                    }
                    p->Left(nl);
                    p->Right(nr);
                    Move(1);
                    break;
                }
                g->AddChild( Express() );
            }
            return p;

        // If
        }else if( t==T::If ){

            //cout << "-If-" << endl;
            p = CreatNode(1);
            NodeGroup* g = new NodeGroup(cur);
            string evt = "if";
            // 开始构建 If 块结构
            while(1){
                bool end = IS_SIGN(";")
                   , elif = IS_KEYWORD("elif")
                   , el = IS_KEYWORD("else");
                if(end||elif||el){ // 写入上部流程块
                    size_t s = g->ChildSize();
                    if(evt=="if"){
                        if(s==0){
                            p->AddChild(NULL);
                            p->AddChild(NULL);
                        }else if(s==1){
                            p->AddChild(g->Child(0)); // 第一个表达式作为if条件
                            p->AddChild(NULL);
                        }else if(s>=2){
                            p->AddChild(g->ChildPop(0));
                            p->AddChild(g);
                        }
                    }else if(evt=="else"){
                        p->AddChild(g);
                    }
                    Move(1);
                    g = new NodeGroup(cur);
                }
                if(end){ // If 结束
                    break;
                }
                if(el){
                    evt = "else"; //块改变
                }
                g->AddChild( Express() );
            }
            return p;

        // List 列表结构
        }else if( t==T::List ){

            //cout << "-List-" << endl;
            Word start = cur;
            Move(1); //跳过 [
            NodeList *list = new NodeList(cur);
            while(1){ // 列表项
                Node *e = Express();
                if(e) list->AddChild( e );
                else break;
                //cout<<"list item"<<endl;
            }
            if(IS_SIGN("]")){
                //cout<<"IS_SIGN(])"<<endl;
                Move(1); //跳过 ]
                //cout<<"Move(1)"<<endl;
                return list; //成功返回列表结构
            }else{
                //cout<<"Error(301)"<<endl;
                Error(301, start); //缺少右括号匹配
            }

        // Print
        }else if( t==T::Print ){

            // cout << "-Print-" << endl;
            p = CreatNode(1);
            // print 关键字左边的第一个值将被打印
            p->Right( Express() );
            return p;

        //// End 终止
        }else if( t==T::End ){

            //cout << "-End-" << endl;
            return p;

        }else{
            return p;
        }

    }

    return p;

}
*/



/**
 * 构建语法树
 */
Node* Nodezer::BuildAST()
{
    Read();
    return Group();
}


/**
 * 表达式组合
 */
Node* Nodezer::Group()
{
    NodeGroup* gr = new NodeGroup(cur);

    while(1){
        if(IS_SIGN(",")){
            Move(1); //跳过
            continue;
        }
        if(IsGroupEnd()){ // 结束符号
            break;
        }
        // 循环建立表达式
        Node *e = Express();
        // cout << "-Nodezer::Group while-" << endl;
        if(!e){
            break;
        }
        //添加
        gr->AddChild( e );
    }
    return gr;
}


/**
 * 表达式组合完结
 * @param endl 要检测的结束符号 ) ] } ; 
 */
bool Nodezer::IsGroupEnd()
{
    if(cur.type==S::Sign){
        string v = cur.value;
        if( v==")" ||
            v=="]" ||
            v=="}" ||
            v==";"
        ){
            //cout<<"end by )]}; ＝ "<<v<<endl;
            return true; // 表达式组结束
        }
    }

    return false;

}



#undef TN_VALUE
#undef TN_ASMD
#undef TN_AS
#undef TN_MD
#undef IS_SIGN
#undef IS_NO_SIGN
#undef IS_KEYWORD
#undef IS_NO_KEYWORD





#undef S // Token::State
#undef T // NodeType



#undef NODELIST


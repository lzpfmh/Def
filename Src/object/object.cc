/**
 * 
 */

#include <string>
#include <map>


#include "object.h"
#include "operat.h"

namespace def {
namespace object {



#define T ObjectType

/**
 * 打印对象
 */
void DefObject::Print(DefObject *obj){

    T t = obj->type; // 获取类型

    if( t==T::Int ){
        //cout<<"-Print Int-"<<endl;
        cout << ((ObjectInt*)obj)->value;

    }else if( t==T::Float ){
        //cout<<"-Print Float-"<<endl;
        cout << ((ObjectFloat*)obj)->value;

    }else if(t==T::String){
        cout << "\"";
        cout << ((ObjectString*)obj)->value;
        cout << "\"";

    }else if(t==T::List){ // 列表
        //cout<<"-Print List-"<<endl;
        ObjectList* list = (ObjectList*)obj;
        cout << "(";
        //size_t sz = obj->Size();
        size_t sz = list->Size();
        for(size_t i=0; i<sz; i++){
            if(i) cout<<" ";
            Print( list->Visit(i) );
        }
        cout << ")";

    }else if(t==T::Dict){ // 字典
        //cout<<"-Print Dict-"<<endl;
        ObjectDict* dict = (ObjectDict*)obj;
        cout << "[";
        map<string, DefObject*>::iterator it = dict->value.begin();
        bool dv = false;
        for(;it!=dict->value.end();++it){
            if(dv) cout<<", "; else dv=true;
            cout<<"'"<<it->first<<"'";
            Print( it->second );   
        }
        cout << "]";

    }else if(t==T::None){
        cout << "none";
    }else if(t==T::Bool){
        if( Conversion::Bool(obj) ){
            cout << "true";
        }else{
            cout << "false";
        }
    }else{

    }
}


#undef T   // ObjectType




} // --end-- namespace object
} // --end-- namespace def




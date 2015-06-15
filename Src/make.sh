#!/bin/sh

echo -e "- - - - - - - - - - - - - - - - - - - - - - -"
echo -e "- Compile def vm c++ source code to def ..."
echo -e "- - - - - - - - - - - - - - - - - - - - - - -"

g++ ./def.cc ./vm/exec.cc ./vm/gc.cc ./vm/stack.cc ./vm/module.cc ./object/object.cc ./object/operat.cc ./parse/node.cc ./parse/nodezer.cc ./parse/token.cc ./parse/tokenizer.cc -std=c++11 -w -o def 

#echo -e "\nsuccessful ！"


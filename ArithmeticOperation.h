#pragma once
#include <string>
#include <stack>
#include <functional>

using std::string;
using std::stack;
using std::function;

class ArithmeticOperation 
{
public:
    string name;
    int priority;
    bool isBinary;
    function<void(stack<double>&)> run;

    ArithmeticOperation(string name_, int priority_, bool isBinary_, function<void(stack<double>&)> operation_) :
        name(name_),
        priority(priority_),
        isBinary(isBinary_), 
        run(operation_) {}
    ~ArithmeticOperation() = default; 

};
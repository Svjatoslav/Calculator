#include <iostream>
#include <windows.h>
#include <functional>
#include <exception>
#include <filesystem>
#include <memory>
#include <list>
#include "ArithmeticOperation.h"


class CalculatorException : public std::exception
{
public:
    CalculatorException(char *msg, std::string error_place_) : std::exception(), error_place(error_place_) {}
    std::string& GetErrorPlace() { return error_place; }
    std::string error_place;
};

class Calculator
{
private:
    std::vector <ArithmeticOperation*> operations;
    std::string expression;
    std::list<std::string> polish_list;
    std::list<HMODULE> descriptors;

    void SetExpression(std::string const& str) {
        expression = str;
    }

    void RemoveGaps() {
        for (int i = 0; i < expression.size(); ++i) {
            if (expression[i] == ' ') {
                expression.erase(i, 1);
                i--;
            }
        }
    }

    void MakePolishNot() {

        std::stack<ArithmeticOperation*> operations_stack;
        RemoveGaps();
        ImportDll();

        size_t i = 0;
        while (i <= expression.size() - 1) {
            if (expression[i] >= '0' && expression[i] <= '9' || expression[i] == '-' && (i == 0 || (expression.size() != 1 && expression[i - 1] == '('))) {
                std::string buffer;
                buffer += expression[i];
                int j = 1;
                short int one_dot = 0;
                while (i <= expression.size() - 1 && (expression[i + 1] >= '0' && expression[i + 1] <= '9' || expression[i + 1] == '.')) {
                    if (expression[i] == '.') {
                        one_dot++;
                        if (one_dot > 1)
                            throw CalculatorException("operation failed: ", ".");
                    }
                    buffer += expression[i + 1];
                    j++;
                    i++;
                }
                i++;
                polish_list.push_back(buffer);
            }
            else if (expression[i] == ',') {
                while (!operations_stack.empty() && operations_stack.top()->name != "(") {
                    polish_list.push_back(operations_stack.top()->name);
                    operations_stack.pop();
                }
                i++;
            }
            else {
                bool totalisFonud = false;
                for (auto const& item : operations) {
                    bool isFound = true;
                    for (int k = 0; k < item->name.size(); ++k) {
                        if (item->name[k] != expression[i + k]) {
                            isFound = false;
                            break;
                        }
                    }
                    if (isFound) {
                        totalisFonud = true;
                        i += item->name.size();
                        if (operations_stack.empty() || item->name == "(")
                            operations_stack.push(item);
                        else {
                            while (!operations_stack.empty() && operations_stack.top()->priority >= item->priority && operations_stack.top()->name != "(") {
                                polish_list.push_back(operations_stack.top()->name);
                                operations_stack.pop();
                            }
                            if (item->name != ")") {
                                operations_stack.push(item);
                            }
                            else {
                                operations_stack.pop();
                            }
                        }
                        break;
                    }
                }
                if (!totalisFonud) {
                    std::string cs(1, expression[i]);
                    throw CalculatorException("Operation or symbor is not correct:", cs);
                    return;
                }
            }
        }
        while (!operations_stack.empty()) {
            polish_list.push_back(operations_stack.top()->name);
            operations_stack.pop();
        }
    }

    void SolvePolishNot() {
        std::stack<double> solution_stack;
        while (!polish_list.empty()) {
            if (polish_list.front()[0] >= '0' && polish_list.front()[0] <= '9' || (polish_list.front()[0] == '-' && polish_list.front().size() > 1)) {
                solution_stack.push(std::stod(polish_list.front()));
                polish_list.pop_front();
            }
            else {
                for (auto const& item : operations) {
                    if (item->name == polish_list.front()) {
                        try {
                            item->run(solution_stack);  //  operation applying
                        }
                        catch (CalculatorException& e) {
                            std::cout << e.what() << " " << e.GetErrorPlace() << std::endl;

                            return;
                        }

                        break;
                    }
                }
                polish_list.pop_front();
            }
        }
        if (solution_stack.size() == 1) {
            std::cout << "answer -- : " << solution_stack.top() << std::endl;
            std::cout << "type \"stop\" to finish program\n";
        }
        else {
            throw CalculatorException("operation is missed", "");
        }
    }

    void ImportDll() {

        using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;

        std::filesystem::path PluginsPath = std::filesystem::current_path().parent_path();
        PluginsPath /= "plugins";

        for (const auto& File : recursive_directory_iterator(PluginsPath)) {

            std::string filename = File.path().string();
            HMODULE hLib;
            hLib = LoadLibraryA(filename.c_str());
            descriptors.push_back(hLib);

            if (hLib != nullptr) {

                char* func_name;
                (FARPROC&)func_name = GetProcAddress(hLib, "name");

                int* func_priority;
                (FARPROC&)func_priority = GetProcAddress(hLib, "priority");

                bool* func_bynary;
                (FARPROC&)func_bynary = GetProcAddress(hLib, "isBynary");

                operations.push_back(new ArithmeticOperation(func_name, *func_priority, *func_bynary, [hLib, func_bynary, func_name](stack<double>& st) -> double {
                    if (*func_bynary && st.size() >= 2) {
                        double (*pFunction)(double, double);
                        (FARPROC&)pFunction = GetProcAddress(hLib, "func");
                        double x = st.top();
                        st.pop();
                        double y = pFunction(st.top(), x);
                        st.top() = y;
                        return y;
                    }
                    else if (!*func_bynary && st.size() >= 1) {
                        double (*pFunction)(double);
                        (FARPROC&)pFunction = GetProcAddress(hLib, "func");
                        st.top() = pFunction(st.top());
                        return st.top();
                    }
                    else {
                        throw CalculatorException("operation failed: ", func_name);
                    }
                    }));
            }
        }
    }

public:
    Calculator() {
        operations.push_back(new ArithmeticOperation("+", 1, true, [](std::stack<double>& st) {
            if (st.size() < 2) {
                throw CalculatorException("operation failed: ", "addition");
            }
            double x = st.top();
            st.pop();
            double y = st.top() + x;
            st.top() = y;
            }));

        operations.push_back(new ArithmeticOperation("-", 1, true, [](std::stack<double>& st)  {
            if (st.size() < 2) {
                throw CalculatorException("operation failed: ", "subtraction");
            }
            double x = st.top();
            st.pop();
            double y = st.top() - x;
            st.top() = y;
            }));

        operations.push_back(new ArithmeticOperation("*", 2, true, [](std::stack<double>& st)  {
            if (st.size() < 2) {
                throw CalculatorException("operation failed: ", "multiply");
            }
            double x = st.top();
            st.pop();
            double y = st.top() * x;
            st.top() = y;
            }));

        operations.push_back(new ArithmeticOperation("/", 2, true, [](std::stack<double>& st)  {
            if (st.size() < 2 || st.top() == 0) {
                throw CalculatorException("operation failed: ", "division");
            }
            double x = st.top();
            st.pop();
            double y = st.top() / x;
            st.top() = y;
            }));

        operations.push_back(new ArithmeticOperation("(", 0, false, nullptr));
        operations.push_back(new ArithmeticOperation(")", 0, false, nullptr));
    }

    ~Calculator() {
        for (auto item : operations) {
            delete item;
        }

        for (auto item : descriptors) {
            FreeLibrary(item);
        }
    }

    void StartCalculator() {
        std::cout << "type the expression\n";
        std::string input;
        while (input != "stop") {
            input.clear();
            polish_list.clear();
            expression.clear();
            std::getline(std::cin, input);
            if (input == "stop")
                break;

            SetExpression(input);

            try {
                MakePolishNot();
                SolvePolishNot();
            }
            catch (CalculatorException& e) {
                std::cout << e.what() << " " << e.GetErrorPlace() << std::endl;
            }
        }
    }

};

int main()
{
    Calculator calculator;
    calculator.StartCalculator();

    return 0;
}


// src/tools/calculator_eval.cpp — YOU IMPLEMENT (spec §4.6).
//
// calc_evaluate: evaluate a well-formed token list with two stacks
// (one for numbers, one for operators and '('). See spec §4.6 and the
// worked trace in Appendix C. std::stack or std::vector are both fine.

#include "tools/calculator.h"

// returns which operators should be applied first


static int precedence(CalcToken::Kind op) {


if (op == CalcToken::Kind::Times ||
    op == CalcToken::Kind::Divide) {
    return 2;

}



if (op == CalcToken::Kind::Plus ||
    op == CalcToken::Kind::Minus) {
    return 1;
}


// parentheses and other tokens are not arithmetic operators
return 0;
}

static bool apply_operator(std::vector<double>& values,
                           CalcToken::Kind op,
                           std::string* error) {



    // checks that there are two numbers to work with



    if (values.size() < 2) {
        *error = "not enough values";
        return false;
    }





    // the top number is the right operand
    double right = values.back();
    values.pop_back();





    // the next number is the left operand
    double left = values.back();
    values.pop_back();




    double result = 0.0;


    // applies the operator in left to right order
    switch (op) {
        case CalcToken::Kind::Plus:
            result = left + right;
            break;



        case CalcToken::Kind::Minus:
            result = left - right;
            break;


        case CalcToken::Kind::Times:
            result = left * right;
            break;




        case CalcToken::Kind::Divide:
            // returns the exact error required by the spec
            if (right == 0.0) {
                *error = "division by zero";
                return false;
            }

            result = left / right;
            break;




        default:
            *error = "invalid operator";
            return false;
    }

    // puts the answer back on the value stack
    values.push_back(result);
    return true;
}



bool calc_evaluate(const std::vector<CalcToken>& tokens, double* out,
                   std::string* error) {






    // keeps numbers and operators on separate stacks

    std::vector<double> values;
    std::vector<CalcToken::Kind> ops;






    for (const auto& token : tokens) {
        if (token.kind == CalcToken::Kind::Number) {

            // puts the number on the value stack

            values.push_back(token.value);

        } else if (token.kind == CalcToken::Kind::LParen) {

            // marks the start of a group

            ops.push_back(token.kind);

        } else if (token.kind == CalcToken::Kind::RParen) {



            // finishes the calculations inside this group




            while (!ops.empty() &&
                   ops.back() != CalcToken::Kind::LParen) {
                CalcToken::Kind op = ops.back();
                ops.pop_back();




                if (!apply_operator(values, op, error)) {
                    return false;
                }
            }





            // there must be a matching opening parenthesis
            if (ops.empty()) {
                *error = "unmatched parenthesis";
                return false;
            }




            // removes the opening parenthesis

            ops.pop_back();





        } else if (precedence(token.kind) > 0) {

            // finishes operators with higher or equal precedence first


            while (!ops.empty() &&
                   ops.back() != CalcToken::Kind::LParen &&
                   precedence(ops.back()) >= precedence(token.kind)) {
                CalcToken::Kind op = ops.back();
                ops.pop_back();




                if (!apply_operator(values, op, error)) {
                    return false;
                }
            }




            ops.push_back(token.kind);



        } else {
            *error = "invalid token";
            return false;
        }
    }




    // finishes any operators still waiting


    while (!ops.empty()) {
        CalcToken::Kind op = ops.back();
        ops.pop_back();

        if (!apply_operator(values, op, error)) {
            return false;



        }

    }

    // a finished expression should leave exactly one answer



    if (values.size() != 1) {
        *error = "invalid expression";
        return false;
    }



    *out = values.back();
    return true;
}

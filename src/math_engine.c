#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "math_engine.h"

// Evaluate the expression (this is a simple version, extend as needed)
double evaluate_expression(const char* expression) {
    char operator;
    double operand1, operand2;
    
    // Log the expression being evaluated
    printf("Evaluating expression: %s\n", expression);

    // Parse the expression (currently supports basic expressions like "1 + 2")
    int parsed_items = sscanf(expression, "%lf %c %lf", &operand1, &operator, &operand2);

    // Log what has been parsed
    printf("Parsed items: %d. Operand1: %.2f, Operator: %c, Operand2: %.2f\n",
           parsed_items, operand1, operator, operand2);

    // If parsing is successful, perform the corresponding arithmetic operation
    if (parsed_items == 3) {
        switch (operator) {
            case '+':
                return add(operand1, operand2);
            case '-':
                return subtract(operand1, operand2);
            case '*':
                return multiply(operand1, operand2);
            case '/':
                return divide(operand1, operand2);
            default:
                printf("Unknown operator: %c\n", operator);
                return 0.0;
        }
    } else {
        printf("Failed to parse expression.\n");
        return 0.0;
    }
}

double add(double a, double b) {
    return a + b;
}

double subtract(double a, double b) {
    return a - b;
}

double multiply(double a, double b) {
    return a * b;
}

double divide(double a, double b) {
    if (b == 0) {
        printf("Error: Division by zero\n");
        return 0;
    }
    return a / b;
}

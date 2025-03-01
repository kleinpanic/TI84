#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "math_engine.h"

int use_degrees = 1;


// Helper function to determine operator precedence
int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    if (op == '^') return 3;
    return 0;
}

// Helper function to apply an operation
double apply_operation(double a, double b, char op) {
    switch (op) {
        case '+': return add(a, b);
        case '-': return subtract(a, b);
        case '*': return multiply(a, b);
        case '/': return divide(a, b);
        case '^': return pow(a, b);
        default: return 0.0;
    }
}

// Handle the negation case
double negate(double value) {
    return -value;
}

// Convert degrees to radians if necessary
double convert_to_radians(double value) {
    if (use_degrees) {
        return value * M_PI / 180.0;  // Convert degrees to radians
    }
    return value;  // If radians, return as is
}

// Helper function to handle math functions
double evaluate_function(const char* func, double value) {
    if (strcmp(func, "log") == 0) {
        return log10(value);  // Logarithm base 10
    } else if (strcmp(func, "ln") == 0) {
        return log(value);    // Natural logarithm
    } else if (strcmp(func, "sin") == 0) {
        value = convert_to_radians(value);  // Convert to radians if necessary
        return sin(value);    // Sine
    } else if (strcmp(func, "cos") == 0) {
        value = convert_to_radians(value);  // Convert to radians if necessary
        return cos(value);    // Cosine
    } else if (strcmp(func, "tan") == 0) {
        value = convert_to_radians(value);  // Convert to radians if necessary
        return tan(value);    // Tangent
    }
    return 0.0;
}

double evaluate_expression(const char* expression) {
    double values[100];  // Stack for numbers
    char ops[100];       // Stack for operators
    char func_stack[100][10]; // Stack for function names (like "sin", "log")
    int value_top = -1, op_top = -1, func_top = -1;
    int len = strlen(expression);
    int negation_flag = 0; // Flag to track negation

    printf("Evaluating expression: %s\n", expression);  // Log the full expression

    for (int i = 0; i < len; i++) {
        // Skip spaces
        if (expression[i] == ' ') continue;

        // Current character is a number, parse the full number
        if (isdigit(expression[i])) {
            double val = 0;
            while (i < len && (isdigit(expression[i]) || expression[i] == '.')) {
                if (expression[i] == '.') {
                    i++;
                    double decimal_place = 0.1;
                    while (i < len && isdigit(expression[i])) {
                        val += (expression[i] - '0') * decimal_place;
                        decimal_place *= 0.1;
                        i++;
                    }
                    break;
                } else {
                    val = (val * 10) + (expression[i] - '0');
                    i++;
                }
            }
            // Apply negation if the flag is set
            if (negation_flag) {
                val = -val;
                negation_flag = 0; // Reset flag
            }
            values[++value_top] = val;
            printf("Pushed number to stack: %.2f\n", val);  // Log the number
            i--;

            // Check for implicit multiplication if followed by a function (but not negation)
            if (i + 1 < len && isalpha(expression[i + 1])) {
                ops[++op_top] = '*';  // Insert implicit multiplication
                printf("Inserted implicit multiplication before function\n");
            }
        }
        // Handle negation "neg" represented as "~"
        else if (strncmp(&expression[i], "neg", 3) == 0 || expression[i] == '~') {
            // Handle negation (neg)
            i += (expression[i] == '~') ? 0 : 2;  // Skip past "neg" if found
            negation_flag = 1;  // Set negation flag
            printf("Negation flag set\n");
        }
        // Handle functions like "sin", "log", etc.
        else if (isalpha(expression[i])) {
            char func[10] = {0};
            int j = 0;
            while (i < len && isalpha(expression[i])) {
                func[j++] = expression[i++];
            }
            func[j] = '\0';
            strcpy(func_stack[++func_top], func);  // Push function to stack
            printf("Pushed function to stack: %s\n", func);  // Log the function
            i--;  // Reevaluate the parenthesis after the function
        }
        // Current character is an opening parenthesis
        else if (expression[i] == '(') {
            ops[++op_top] = expression[i];
            printf("Pushed open parenthesis to stack\n");  // Log the parenthesis
        }
        // Current character is a closing parenthesis, resolve parenthesis content
        else if (expression[i] == ')') {
            while (op_top >= 0 && ops[op_top] != '(') {
                double val2 = values[value_top--];
                double val1 = values[value_top--];
                char op = ops[op_top--];
                double result = apply_operation(val1, val2, op);
                values[++value_top] = result;
                printf("Applied operation %c: %.2f %c %.2f = %.2f\n", op, val1, op, val2, result);
            }
            op_top--;  // Pop the opening parenthesis from ops stack
            printf("Popped open parenthesis\n");

            // Apply the function now if there's a function waiting on the stack
            if (func_top >= 0) {
                double result = evaluate_function(func_stack[func_top--], values[value_top--]);
                values[++value_top] = result;
                printf("Applied function result: %.2f\n", result);  // Log the result
            }
        }
        // Current character is an operator
        else if (expression[i] == '+' || expression[i] == '-' || expression[i] == '*' || expression[i] == '/' || expression[i] == '^') {
            // Resolve previous operators with higher or equal precedence
            while (op_top >= 0 && precedence(ops[op_top]) >= precedence(expression[i])) {
                double val2 = values[value_top--];
                double val1 = values[value_top--];
                char op = ops[op_top--];
                double result = apply_operation(val1, val2, op);
                values[++value_top] = result;
                printf("Applied operation %c: %.2f %c %.2f = %.2f\n", op, val1, op, val2, result);
            }
            ops[++op_top] = expression[i];  // Push current operator
            printf("Pushed operator to stack: %c\n", expression[i]);  // Log the operator
        }
    }

    // Apply remaining operators in the stack
    while (op_top >= 0) {
        double val2 = values[value_top--];
        double val1 = values[value_top--];
        char op = ops[op_top--];
        double result = apply_operation(val1, val2, op);
        values[++value_top] = result;
        printf("Applied operation %c: %.2f %c %.2f = %.2f\n", op, val1, op, val2, result);
    }

    // Apply any remaining function on the stack
    while (func_top >= 0) {
        double result = evaluate_function(func_stack[func_top--], values[value_top--]);
        values[++value_top] = result;
        printf("Applied remaining function: %.2f\n", result);
    }

    // The result is the last value in the values stack
    printf("Final result: %.2f\n", values[value_top]);  // Log the final result
    return values[value_top];
}

// Basic arithmetic functions
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

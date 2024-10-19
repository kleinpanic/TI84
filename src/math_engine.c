#include <math.h>
#include "math_engine.h"

// Basic arithmetic operations
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
    if (b != 0) {
        return a / b;
    } else {
        return 0; // Handle division by zero
    }
}

// Advanced math functions
double square_root(double a) {
    if (a >= 0) {
        return sqrt(a);
    } else {
        return 0; // Handle square root of negative numbers
    }
}

double logarithm(double a) {
    if (a > 0) {
        return log(a);
    } else {
        return 0; // Handle log of negative or zero values
    }
}

double sine(double a) {
    return sin(a);
}

double cosine(double a) {
    return cos(a);
}

double tangent(double a) {
    return tan(a);
}

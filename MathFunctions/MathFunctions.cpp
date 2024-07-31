#include "MathFunctions.hpp"

using namespace MathFunctions;

MathFunction::~MathFunction()
{
}

std::optional<double> MathFunction::calDividedFunction(const double a, const double b) const
{
    double result;
    if (b == 0)
    {
        return std::nullopt;
    }
    else
    {
        return a / b;
    }
}

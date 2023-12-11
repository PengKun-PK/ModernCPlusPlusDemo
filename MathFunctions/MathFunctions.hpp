#pragma once

#include <optional>

namespace MathFunctions
{

class MathFunction
{
public:
    MathFunction() = default;
    virtual ~MathFunction();

    std::optional<double> calDividedFunction(const double a, const double b) const;
};

};
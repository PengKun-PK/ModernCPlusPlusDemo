#pragma once

#include <vector>
#include <string>
#include <optional>

namespace StateMachine
{

using strVector = std::vector<std::string>;

class IDataSource
{
public:
    virtual ~IDataSource() = default;

    virtual const strVector& GetStringVector() = 0;

    virtual std::optional<bool> InsertToStringVec(std::string infoString) = 0;

    virtual std::optional<bool> ClearStringVector() = 0;
};

}
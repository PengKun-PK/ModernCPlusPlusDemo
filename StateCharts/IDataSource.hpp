#pragma once

#include <optional>
#include <string>
#include <vector>

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

}  // namespace StateMachine

#pragma once

#include "IDataSource.hpp"

namespace StateMachine
{

class DataSource : public IDataSource
{
public:
    DataSource() = default;
    ~DataSource();

    const strVector& GetStringVector() override;

    std::optional<bool> InsertToStringVec(std::string infoString) override;

    std::optional<bool> ClearStringVector() override;

private:
    strVector m_strVector;
};

}
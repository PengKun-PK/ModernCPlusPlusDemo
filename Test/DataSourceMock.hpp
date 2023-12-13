#pragma once

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "../StateCharts/IDataSource.hpp"

namespace StateMachine
{

class DataSourceMock : public IDataSource
{
public:
    MOCK_METHOD0(GetStringVector,
                 const strVector&());

    MOCK_METHOD1(InsertToStringVec,
                 std::optional<bool>(std::string infoString));

    MOCK_METHOD0(ClearStringVector,
                 std::optional<bool>());
};

}
#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "DataSourceMock.hpp"

namespace StateMachine
{

class CameraTest : public ::testing::Test
{
public:
    CameraTest();
    virtual ~CameraTest();

protected:
    DataSourceMock m_dataSource;
};

}
#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "DataSourceMock.hpp"
#include "../StateCharts/Camera.hpp"

namespace StateMachine
{

class CameraTest : public ::testing::Test
{
public:
    CameraTest();
    virtual ~CameraTest();

protected:
    void SetUp() override;

    void TearDown() override;

    DataSourceMock m_dataSourceMock;

    std::unique_ptr<Camera> m_cameraHandler;
};

}
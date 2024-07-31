#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../StateCharts/Camera.hpp"
#include "DataSourceMock.hpp"

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

}  // namespace StateMachine

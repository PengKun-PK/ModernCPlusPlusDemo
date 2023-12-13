#include "TestCamera.hpp"

#include <string>

using namespace testing;

namespace StateMachine
{

CameraTest::CameraTest()
{

}

CameraTest::~CameraTest()
{

}

void CameraTest::SetUp()
{
    m_cameraHandler = std::make_unique<Camera>(m_dataSourceMock);
}

void CameraTest::TearDown()
{

}

TEST_F(CameraTest, testEnterShooting)
{
    m_cameraHandler->initiate();
    m_cameraHandler->process_event(EvShutterFull("enter shooting"));
}

}
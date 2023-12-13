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

    EXPECT_CALL(m_dataSourceMock, InsertToStringVec(_)).Times(AtLeast(1));
    m_cameraHandler->process_event(EvShutterFull("enter shooting"));
}

}
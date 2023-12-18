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

using event = boost::statechart::event_base;

enum class eventType
{
    ShutterHalf,
    ShutterFull,
    ShutterRelease,
    ShutterConfig
};

struct TestInput
{
    eventType type;
    std::string strData;
};

class CameraTestWithParams : public CameraTest,
                             public WithParamInterface<TestInput>
{
public:
    CameraTestWithParams() = default;
};

/**
 * @test
 * Preconditions:
 *     STM initiate.
 * Description:
 *     Camera stm process some events.
 *      => Data Source's function correctly called.
 */
TEST_P(CameraTestWithParams, TestDataSourceFunction)
{
    // Preconditions
    m_cameraHandler->initiate();

    // Test
    const auto data = GetParam();
    EXPECT_CALL(m_dataSourceMock, InsertToStringVec(_)).Times(1);
    switch (data.type)
    {
    case eventType::ShutterFull:
        m_cameraHandler->process_event(EvShutterFull(data.strData));
        break;
    case eventType::ShutterRelease:
        m_cameraHandler->process_event(EvShutterRelease(data.strData));
        break;
    case eventType::ShutterConfig:
        m_cameraHandler->process_event(EvConfig(data.strData));
        break;
    }
}

INSTANTIATE_TEST_SUITE_P(TestDataSourceFunction, CameraTestWithParams, Values(
    TestInput{ eventType::ShutterFull,          "enter shooting" },
    TestInput{ eventType::ShutterConfig,        "Enter Config"}
));

}
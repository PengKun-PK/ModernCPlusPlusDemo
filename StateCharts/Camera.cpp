#include "Camera.hpp"

namespace StateMachine
{

Camera::Camera(IDataSource &dataSource)
	: m_dataSource(dataSource)
{

}

sc::result NotShooting::react(const EvShutterHalf& ev)
{
	auto& stm = context<Camera>();
	if (stm.IsMemoryAvailable())
	{
		std::cout << ev.msg << std::endl;
		return transit<Shooting>();
	}
	else
	{
		return discard_event();
	}
}

sc::result NotShooting::react(const EvShutterFull& ev)
{
	auto& stm = context<Camera>();
	if (stm.IsBatteryLow())
	{
		return discard_event();
	}
	else
	{
		std::cout << ev.msg << std::endl;
		return transit<Shooting>();
	}
}

sc::result Shooting::react(const EvShutterRelease& ev)
{
	std::cout << ev.msg << std::endl;
	return transit<NotShooting>();
}

sc::result Idle::react(const EvConfig& ev)
{
	std::cout << ev.msg << std::endl;
	return transit<Configuring>();
}

sc::result Configuring::react(const EvConfig& ev)
{
	std::cout << ev.msg << std::endl;
	return transit<Idle>();
}

}
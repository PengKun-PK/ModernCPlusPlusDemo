#pragma once

#include <iostream>
#include <string>

#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state_machine.hpp>

#include "IDataSource.hpp"

namespace sc = boost::statechart;

namespace StateMachine
{

// event
struct EvShutterHalf : sc::event<EvShutterHalf>
{
    EvShutterHalf(std::string str)
    {
        msg = str;
    }

    std::string msg;
};

struct EvShutterFull : sc::event<EvShutterFull>
{
    EvShutterFull(std::string str)
    {
        msg = str;
    }

    std::string msg;
};

struct EvShutterRelease : sc::event<EvShutterRelease>
{
    EvShutterRelease(std::string str)
    {
        msg = str;
    }

    std::string msg;
};

struct EvConfig : sc::event<EvConfig>
{
    EvConfig(std::string str)
    {
        msg = str;
    }

    std::string msg;
};

// state machine
struct NotShooting;
struct Camera : sc::state_machine<Camera, NotShooting>
{
    Camera(IDataSource& dataSource);
    bool IsMemoryAvailable() const
    {
        return true;
    }
    bool IsBatteryLow() const
    {
        return false;
    }

    IDataSource& m_dataSource;
};

// state NotShooting
struct Idle;
struct NotShooting : sc::simple_state<NotShooting, Camera, Idle>
{
    using reactions = boost::mpl::list<sc::custom_reaction<EvShutterFull>, sc::custom_reaction<EvShutterHalf>>;

    // result
    sc::result react(const EvShutterFull&);
    sc::result react(const EvShutterHalf&);
};

struct Idle : sc::simple_state<Idle, NotShooting>
{
    using reactions = sc::custom_reaction<EvConfig>;
    sc::result react(const EvConfig&);
};

struct Configuring : sc::simple_state<Configuring, NotShooting>
{
    using reactions = sc::custom_reaction<EvConfig>;
    sc::result react(const EvConfig&);
};

// state Shooting
struct Shooting : sc::simple_state<Shooting, Camera>
{
    using reactions = sc::custom_reaction<EvShutterRelease>;
    sc::result react(const EvShutterRelease&);
};

}  // namespace StateMachine

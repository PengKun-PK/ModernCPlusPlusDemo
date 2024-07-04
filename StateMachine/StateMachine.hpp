#include <iostream>
#include <string>
#include <variant>
#include <functional>
#include <unordered_map>
#include <memory>
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>

namespace msm = boost::msm;
namespace mpl = boost::mpl;
using namespace boost::msm::front;

// 前向声明
template <class T>
class MyStateMachine;

// 状态机模板类定义
template <class T>
class StateMachine_ : public state_machine_def<StateMachine_<T>>
{
public:
    // 使用 std::variant 定义事件
    struct Event1 { int data; };
    struct Event2 { std::string message; };
    struct Event3 {};
    using Event = std::variant<Event1, Event2, Event3>;

    // 定义状态
    struct State1 : state<> {};
    struct State2 : state<> {};
    struct State3 : state<> {};

    // 定义初始状态
    using initial_state = State1;

    // 动作
    struct Action1 {
        template <class EVT, class FSM, class SourceState, class TargetState>
        void operator()(EVT const&, FSM&, SourceState&, TargetState&) const {
            std::cout << "Executing Action1\n";
        }
    };

    struct Action2 {
        template <class EVT, class FSM, class SourceState, class TargetState>
        void operator()(EVT const&, FSM&, SourceState&, TargetState&) const {
            std::cout << "Executing Action2\n";
        }
    };

    // 守卫
    struct Guard1 {
        template <class EVT, class FSM, class SourceState, class TargetState>
        bool operator()(EVT const&, FSM&, SourceState&, TargetState&) const {
            std::cout << "Checking Guard1\n";
            return true;
        }
    };

    // 转换表
    struct transition_table : mpl::vector<
        //    Start     Event      Next      Action      Guard
        Row < State1,   Event1,    State2,   Action1,    Guard1 >,
        Row < State2,   Event2,    State3,   Action2,    none   >,
        Row < State3,   Event3,    State1,   none,       none   >,
        Row < State1,   Event2,    State1,   Action2,    none   >  // 内部转换
    > {};

    // 允许的事件替代方案（如果没有匹配的转换）
    template <class FSM, class Event>
    void no_transition(Event const& e, FSM&, int state)
    {
        std::cout << "No transition from state " << state
                  << " on event " << typeid(e).name() << std::endl;
    }
};

// 状态机包装类
template <class T>
class MyStateMachine : public msm::back::state_machine<StateMachine_<T>>
{
public:
    // 便捷方法来处理 variant 事件
    void process(const typename StateMachine_<T>::Event& event)
    {
        this->process_event(event);
    }
};
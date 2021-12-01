// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef UXR_AGENT_AGENT_INSTANCE_HPP_
#define UXR_AGENT_AGENT_INSTANCE_HPP_

#include <uxr/agent/config.hpp>

#include <uxr/agent/utils/ArgumentParser.hpp>

#include <csignal>

namespace eprosima {
namespace uxr {

namespace middleware {
/**
 * @brief   Forward declaration.
 */
class CallbackFactory;
} // middleware

/**
 * @brief   Singleton class to manage the launch process of a MicroXRCE-DDS Agent.
 */
class AgentInstance
{
private:
    /**
     * @brief   Default constructor.
     */
    UXR_AGENT_EXPORT AgentInstance();
    /**
     * @brief   AgentInstance class shall not be copy constructible.
     */
    UXR_AGENT_EXPORT AgentInstance(
            const AgentInstance &) = delete;

    UXR_AGENT_EXPORT AgentInstance(
            AgentInstance &&) = delete;

    /**
     * @brief   AgentInstance class shall not be copy assignable.
     */
    UXR_AGENT_EXPORT AgentInstance& operator =(
            const AgentInstance &) = delete;

    UXR_AGENT_EXPORT AgentInstance& operator =(
            AgentInstance &&) = delete;

public:
    /**
     * @brief   Get instance associated to this class.
     * @return  Static reference to the singleton AgentInstance object.
     */
    UXR_AGENT_EXPORT static AgentInstance& getInstance();

    /**
     * @brief       Create an Agent instance, based on provided input parameters from user.
     * @param[in]   argc Number of available parameters introduced by the user.
     * @param[in]   argv List of parameters to be parsed to instantiate an Agent.
     * @return      Boolean value indicating if a Micro XRCE-DDS Agent was instantiated successfully.
     */
    UXR_AGENT_EXPORT bool create(
            int argc,
            char** argv);

    /**
     * @brief   Run the created agent until finished via user interrupt or process error.
     */
    UXR_AGENT_EXPORT void run();

#ifndef _WIN32
    UXR_AGENT_EXPORT void stop();
#endif

    /**
     * @brief Sets a callback function for a specific create/delete middleware entity operation.
     *        Note that not some middlewares might not implement every defined operation, or even
     *        no operation at all.
     * @param middleware_kind   Enumeration class defining all the supported pluggable middlewares for the agent.
     * @param callback_kind     Enumeration class defining all the different operations available to which
     *                          set a callback to.
     * @param callback_function std::function rvalue variable implementing the callback logic. Desirable
     *                          to be implemented using lambda expressions wrapped inside a std::function descriptor.
     */
    template <typename ... Args>
    UXR_AGENT_EXPORT void add_middleware_callback(
            const Middleware::Kind& middleware_kind,
            const middleware::CallbackKind& callback_kind,
            std::function<void (Args ...)>&& callback_function);

private:
    std::thread agent_thread_;
    std::condition_variable exit_signal;
    middleware::CallbackFactory& callback_factory_;
};
} // uxr
} // eprosima

#endif  // UXR_AGENT_AGENT_INSTANCE_HPP_
// Copyright 2020-present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef UXR__AGENT__MIDDLEWARE__UTILS__CALLBACKS_HPP_
#define UXR__AGENT__MIDDLEWARE__UTILS__CALLBACKS_HPP_

#include <uxr/agent/middleware/Middleware.hpp>

#ifdef UAGENT_FAST_PROFILE
#include <uxr/agent/middleware/fast/FastMiddleware.hpp>
#include <uxr/agent/middleware/fastdds/FastDDSMiddleware.hpp>

#endif // UAGENT_FAST_PROFILE
#ifdef UAGENT_CED_PROFILE
#include <uxr/agent/middleware/ced/CedMiddleware.hpp>
#endif // UAGENT_CED_PROFILE

#include <functional>

#include <stdarg.h>

namespace eprosima {
namespace uxr {
namespace middleware {

/**********************************************************************************************************************
 * CallbackKind
 **********************************************************************************************************************/
/**
 * @brief   Enumeration class defining the different types of callbacks available for a middleware.
 */
enum class CallbackKind : uint8_t
{
    CREATE_PARTICIPANT,
    CREATE_DATAWRITER,
    CREATE_DATAREADER,
    CREATE_REQUESTER,
    CREATE_REPLIER,
    DELETE_PARTICIPANT,
    DELETE_DATAWRITER,
    DELETE_DATAREADER,
    DELETE_REQUESTER,
    DELETE_REPLIER
};

/**********************************************************************************************************************
 * Callback
 **********************************************************************************************************************/
/**
 * @brief   Represents a callback function, which can be called anytime.
 */
template <typename ... Args>
class Callback
{
public:
    /**
     * @brief   Constructor.
     * @param callback_function Implementation of the callback function. Must be passed as rvalue.
     */
    Callback(
            std::function<void (Args ...)>&& callback_function)
        : callback_(std::move(callback_function))
    {
    }

    /**
     * @brief   Default destructor.
     */
    ~Callback() = default;

    /**
     * @brief   Const version of the call () operator.
     * @param args  Variable number of arguments to be passed to the callback function.
     */
    inline void operator ()(
            Args ... args) const
    {
        callback_(std::forward<Args>(args)...);
    }

    /**
     * @brief   Non-const version of the call () operator.
     * @param args  Variable number of arguments to be passed to the callback function.
     */
    inline void operator ()(
            Args ... args)
    {
        callback_(std::forward<Args>(args)...);
    }

private:

    std::function<void (Args ...)> callback_;
};

/**********************************************************************************************************************
 * CallbackFactory
 **********************************************************************************************************************/
/**
 * @brief   Represents a factory of all the defined callbacks for each active communication middleware.
 */
class CallbackFactory
{
public:
    /**
     * @brief   Default destructor.
     */
    ~CallbackFactory() = default;

    /**
     * @brief   Get instance associated to this singleton class.
     * @return  Reference to the CallbackFactory database static object.
     */
    static CallbackFactory& getInstance()
    {
        static CallbackFactory instance;
        return instance;
    }

    /**
     * @brief   Add callback function for a specific middleware kind and operation.
     * @param middleware_kind   Enumeration class defining all the supported middlewares.
     * @param callback_kind     Enumeration class defining all the different operations available to which
     *                          set a callback to.
     * @param callback_function std::function rvalue variable implementing the callback method logic to be
     *                          included in the callback database.
     */
    template <typename ... Args>
    void add_callback(
            const Middleware::Kind& middleware_kind,
            const CallbackKind& callback_kind,
            std::function<void (Args ...)>&& callback_function)
    {
    switch (middleware_kind)
        {
#ifdef UAGENT_FAST_PROFILE
            case Middleware::Kind::FASTRTPS:
            {
                fast_callback_factory_.add_callback(callback_kind, std::move(callback_function));
                break;
            }
            case Middleware::Kind::FASTDDS:
            {
                fast_dds_callback_factory_.add_callback(callback_kind, std::move(callback_function));
                break;
            }
#endif // UAGENT_FAST_PROFILE
#ifdef UAGENT_CED_PROFILE
            case Middleware::Kind::CED:
#endif // UAGENT_CED_PROFILE
            case Middleware::Kind::NONE:
            {
                // Callbacks not implemented
                break;
            }
        }
    }

    /**
     * @brief   Execute specific callback type functions registered for a certain middleware kind.
     * @param middleware_kind   Enumeration class defining all the supported middlewares.
     * @param callback_kind     Enumerated element from class CallbackKind indicating which callback
     *                          functions need to be executed.
     * @param args              Variadic number of arguments to be passed to the registered callbacks.
     *                          Note that this means that are registered callbacks for a certain Middleware::Kind
     *                          and CallbackKind must receive the same type of input parameters.
     */
    template <typename ... Args>
    void execute_callbacks(
            const Middleware::Kind& middleware_kind,
            const CallbackKind callback_kind,
            Args ... args)
    {
        switch (middleware_kind)
        {
#ifdef UAGENT_FAST_PROFILE
            case Middleware::Kind::FASTRTPS:
            {
                fast_callback_factory_.execute_callbacks(callback_kind, std::forward<Args>(args)...);
                break;
            }
            case Middleware::Kind::FASTDDS:
            {
                fast_dds_callback_factory_.execute_callbacks(callback_kind, std::forward<Args>(args)...);
                break;
            }
#endif // UAGENT_FAST_PROFILE
#ifdef UAGENT_CED_PROFILE
            case Middleware::Kind::CED:
#endif // UAGENT_CED_PROFILE
            case Middleware::Kind::NONE:
            {
                // Callbacks not implemented
                break;
            }
        }
    }

private:
    /**
     * @brief   Default constructor.
     */
    CallbackFactory() = default;

    /**
     * @brief   CallbackFactory shall not be copy constructible.
     */
    CallbackFactory(
            const CallbackFactory &) = delete;

    CallbackFactory(
            CallbackFactory &&) = delete;

    /**
     * @brief   CallbackFactory shall not be copy assignable.
     */
    CallbackFactory& operator = (
            const CallbackFactory &) = delete;

    CallbackFactory& operator = (
            CallbackFactory &&) = delete;

#ifdef UAGENT_FAST_PROFILE
    /**********************************************************************************************************************
     * FastCallbackFactory
     **********************************************************************************************************************/
    class FastCallbackFactory
    {
    public:
        /**
         * @brief   Default constructor.
         */
        FastCallbackFactory() = default;

        /**
         * @brief   Default destructor.
         */
        ~FastCallbackFactory() = default;

        /**
         * @brief   FastCallbackFactory shall not be copy constructible.
         */
        FastCallbackFactory(
                const FastCallbackFactory &) = delete;

        FastCallbackFactory(
                FastCallbackFactory &&) = delete;

        /**
         * @brief   FastCallbackFactory shall not be copy assignable.
         */
        FastCallbackFactory& operator = (
                const FastCallbackFactory &) = delete;

        FastCallbackFactory& operator = (
                FastCallbackFactory &&) = delete;

        /**
         * @brief   Adds a callback function to the database.
         * @param callback_kind     Enumerated element from class CallbackKind indicating which callback
         *                          functions need to be executed.
         * @param callback_function std::function rvalue variable implementing the callback method logic to be
         *                          included in the callback database.
         */
        template <typename ... Args>
        void add_callback(
                const CallbackKind& callback_kind,
                std::function<void (Args ...)>&& callback_function)
        {
            switch (callback_kind)
            {
                case CallbackKind::CREATE_PARTICIPANT:
                {
                    on_create_participant_callbacks_.emplace_back(Callback<Args ...>(std::move(callback_function)));
                    break;
                }
                case CallbackKind::CREATE_DATAWRITER:
                case CallbackKind::DELETE_DATAWRITER:
                case CallbackKind::CREATE_DATAREADER:
                case CallbackKind::DELETE_DATAREADER:
                case CallbackKind::CREATE_REQUESTER:
                case CallbackKind::DELETE_REQUESTER:
                case CallbackKind::CREATE_REPLIER:
                case CallbackKind::DELETE_REPLIER:
                {
                    // Only implemented in template specialization
                    break;
                }
                case CallbackKind::DELETE_PARTICIPANT:
                {
                    on_delete_participant_callbacks_.emplace_back(Callback<Args ...>(std::move(callback_function)));
                    break;
                }
            }
        }

        /**
         * @brief   Executes a certain kind of callback functions.
         * @param callback_kind Kind of callback functions to be executed.
         */
        void execute_callbacks(
                const CallbackKind callback_kind ...) const
        {
            va_list args;
            va_start(args, callback_kind);

            switch (callback_kind)
            {
                case CallbackKind::CREATE_PARTICIPANT:
                {
                    fastrtps::Participant* participant = va_arg(args, fastrtps::Participant*);
                    for (const auto& on_create_participant_callback : on_create_participant_callbacks_)
                    {
                        on_create_participant_callback(participant);
                    }
                    break;
                }
                case CallbackKind::CREATE_DATAWRITER:
                {
                    fastrtps::Participant* participant = va_arg(args, fastrtps::Participant*);
                    fastrtps::Publisher* datawriter = va_arg(args, fastrtps::Publisher*);
                    for (const auto& on_create_datawriter_callback : on_create_datawriter_callbacks_)
                    {
                        on_create_datawriter_callback(participant, datawriter);
                    }
                    break;
                }
                case CallbackKind::CREATE_DATAREADER:
                {
                    fastrtps::Participant* participant = va_arg(args, fastrtps::Participant*);
                    fastrtps::Subscriber* datareader = va_arg(args, fastrtps::Subscriber*);
                    for (const auto& on_create_datareader_callback : on_create_datareader_callbacks_)
                    {
                        on_create_datareader_callback(participant, datareader);
                    }
                    break;
                }
                case CallbackKind::CREATE_REQUESTER:
                {
                    fastrtps::Participant* participant = va_arg(args, fastrtps::Participant*);
                    fastrtps::Publisher* req_datawriter = va_arg(args, fastrtps::Publisher*);
                    fastrtps::Subscriber* repl_datareader = va_arg(args, fastrtps::Subscriber*);
                    for (const auto& on_create_requester_callback : on_create_requester_callbacks_)
                    {
                        on_create_requester_callback(participant, req_datawriter, repl_datareader);
                    }
                    break;
                }
                case CallbackKind::CREATE_REPLIER:
                {
                    fastrtps::Participant* participant = va_arg(args, fastrtps::Participant*);
                    fastrtps::Publisher* repl_datawriter = va_arg(args, fastrtps::Publisher*);
                    fastrtps::Subscriber* req_datareader = va_arg(args, fastrtps::Subscriber*);
                    for (const auto& on_create_replier_callback : on_create_replier_callbacks_)
                    {
                        on_create_replier_callback(participant, repl_datawriter, req_datareader);
                    }
                    break;
                }
                case CallbackKind::DELETE_PARTICIPANT:
                {
                    fastrtps::Participant* participant = va_arg(args, fastrtps::Participant*);
                    for (const auto& on_delete_participant_callback : on_delete_participant_callbacks_)
                    {
                        on_delete_participant_callback(participant);
                    }
                    break;
                }
                case CallbackKind::DELETE_DATAWRITER:
                {
                    fastrtps::Participant* participant = va_arg(args, fastrtps::Participant*);
                    fastrtps::Publisher* datawriter = va_arg(args, fastrtps::Publisher*);
                    for (const auto& on_delete_datawriter_callback : on_delete_datawriter_callbacks_)
                    {
                        on_delete_datawriter_callback(participant, datawriter);
                    }
                    break;
                }
                case CallbackKind::DELETE_DATAREADER:
                {
                    fastrtps::Participant* participant = va_arg(args, fastrtps::Participant*);
                    fastrtps::Subscriber* datareader = va_arg(args, fastrtps::Subscriber*);
                    for (const auto& on_delete_datareader_callback : on_delete_datareader_callbacks_)
                    {
                        on_delete_datareader_callback(participant, datareader);
                    }
                    break;
                }
                case CallbackKind::DELETE_REQUESTER:
                {
                    fastrtps::Participant* participant = va_arg(args, fastrtps::Participant*);
                    fastrtps::Publisher* req_datawriter = va_arg(args, fastrtps::Publisher*);
                    fastrtps::Subscriber* repl_datareader = va_arg(args, fastrtps::Subscriber*);
                    for (const auto& on_delete_requester_callback : on_delete_requester_callbacks_)
                    {
                        on_delete_requester_callback(participant, req_datawriter, repl_datareader);
                    }
                    break;
                }
                case CallbackKind::DELETE_REPLIER:
                {
                    fastrtps::Participant* participant = va_arg(args, fastrtps::Participant*);
                    fastrtps::Publisher* repl_datawriter = va_arg(args, fastrtps::Publisher*);
                    fastrtps::Subscriber* req_datareader = va_arg(args, fastrtps::Subscriber*);
                    for (const auto& on_delete_replier_callback : on_delete_replier_callbacks_)
                    {
                        on_delete_replier_callback(participant, repl_datawriter, req_datareader);
                    }
                    break;
                }
            }
            va_end(args);
        }

    private:

        using CreateParticipantCallback = Callback<
                        const fastrtps::Participant*>;
        using DeleteParticipantCallback = Callback<
                        const fastrtps::Participant*>;
        using CreateDataWriterCallback = Callback<
                        const fastrtps::Participant*,
                        const fastrtps::Publisher*>;

        using DeleteDataWriterCallback = Callback<
                        const fastrtps::Participant*,
                        const fastrtps::Publisher*>;

        using CreateDataReaderCallback = Callback<
                        const fastrtps::Participant*,
                        const fastrtps::Subscriber*>;

        using DeleteDataReaderCallback = Callback<
                        const fastrtps::Participant*,
                        const fastrtps::Subscriber*>;

        using CreateRequesterCallback = Callback<
                        const fastrtps::Participant*,
                        const fastrtps::Publisher*,
                        const fastrtps::Subscriber*>;

        using DeleteRequesterCallback = Callback<
                        const fastrtps::Participant*,
                        const fastrtps::Publisher*,
                        const fastrtps::Subscriber*>;

        using CreateReplierCallback = Callback<
                        const fastrtps::Participant*,
                        const fastrtps::Publisher*,
                        const fastrtps::Subscriber*>;

        using DeleteReplierCallback = Callback<
                        const fastrtps::Participant*,
                        const fastrtps::Publisher*,
                        const fastrtps::Subscriber*>;

        std::vector<CreateParticipantCallback> on_create_participant_callbacks_;
        std::vector<DeleteParticipantCallback> on_delete_participant_callbacks_;
        std::vector<CreateDataWriterCallback> on_create_datawriter_callbacks_;
        std::vector<DeleteDataWriterCallback> on_delete_datawriter_callbacks_;
        std::vector<CreateDataReaderCallback> on_create_datareader_callbacks_;
        std::vector<DeleteDataReaderCallback> on_delete_datareader_callbacks_;
        std::vector<CreateRequesterCallback> on_create_requester_callbacks_;
        std::vector<DeleteRequesterCallback> on_delete_requester_callbacks_;
        std::vector<CreateReplierCallback> on_create_replier_callbacks_;
        std::vector<DeleteReplierCallback> on_delete_replier_callbacks_;
    };
    FastCallbackFactory fast_callback_factory_;

    /**********************************************************************************************************************
     * FastDDSCallbackFactory
     **********************************************************************************************************************/
    class FastDDSCallbackFactory
    {
    public:
        /**
         * @brief   Default constructor.
         */
        FastDDSCallbackFactory() = default;

        /**
         * @brief   Default destructor.
         */
        ~FastDDSCallbackFactory() = default;

        /**
         * @brief   FastDDSCallbackFactory shall not be copy constructible.
         */
        FastDDSCallbackFactory(
                const FastDDSCallbackFactory &) = delete;

        FastDDSCallbackFactory(
                FastDDSCallbackFactory &&) = delete;

        /**
         * @brief   FastDDSCallbackFactory shall not be copy assignable.
         */
        FastDDSCallbackFactory& operator = (
                const FastDDSCallbackFactory &) = delete;

        FastDDSCallbackFactory& operator = (
                FastDDSCallbackFactory &&) = delete;

        /**
         * @brief   Adds a callback function to the database.
         * @param callback_kind     Enumerated element from class CallbackKind indicating which callback
         *                          functions need to be executed.
         * @param callback_function std::function rvalue variable implementing the callback method logic to be
         *                          included in the callback database.
         */
        template <typename ... Args>
        void add_callback(
                const CallbackKind& callback_kind,
                std::function<void (Args ...)>&& callback_function)
        {
            switch (callback_kind)
            {
                case CallbackKind::CREATE_PARTICIPANT:
                {
                    on_create_participant_callbacks_.emplace_back(Callback<Args ...>(std::move(callback_function)));
                    break;
                }
                case CallbackKind::CREATE_DATAWRITER:
                case CallbackKind::DELETE_DATAWRITER:
                case CallbackKind::CREATE_DATAREADER:
                case CallbackKind::DELETE_DATAREADER:
                case CallbackKind::CREATE_REQUESTER:
                case CallbackKind::DELETE_REQUESTER:
                case CallbackKind::CREATE_REPLIER:
                case CallbackKind::DELETE_REPLIER:
                {
                    // Only implemented in template specialization
                    break;
                }
                case CallbackKind::DELETE_PARTICIPANT:
                {
                    on_delete_participant_callbacks_.emplace_back(Callback<Args ...>(std::move(callback_function)));
                    break;
                }
            }
        }

        /**
         * @brief   Executes a certain kind of callback functions.
         * @param callback_kind Kind of callback functions to be executed.
         */
        void execute_callbacks(
                const CallbackKind callback_kind ...) const
        {
            va_list args;
            va_start(args, callback_kind);

            switch (callback_kind)
            {
                case CallbackKind::CREATE_PARTICIPANT:
                {
                    fastdds::dds::DomainParticipant* participant = va_arg(args, fastdds::dds::DomainParticipant*);
                    for (const auto& on_create_participant_callback : on_create_participant_callbacks_)
                    {
                        on_create_participant_callback(participant);
                    }
                    break;
                }
                case CallbackKind::CREATE_DATAWRITER:
                {
                    fastdds::dds::DomainParticipant* participant = va_arg(args, fastdds::dds::DomainParticipant*);
                    fastdds::dds::DataWriter* datawriter = va_arg(args, fastdds::dds::DataWriter*);
                    for (const auto& on_create_datawriter_callback : on_create_datawriter_callbacks_)
                    {
                        on_create_datawriter_callback(participant, datawriter);
                    }
                    break;
                }
                case CallbackKind::CREATE_DATAREADER:
                {
                    fastdds::dds::DomainParticipant* participant = va_arg(args, fastdds::dds::DomainParticipant*);
                    fastdds::dds::DataReader* datareader = va_arg(args, fastdds::dds::DataReader*);
                    for (const auto& on_create_datareader_callback : on_create_datareader_callbacks_)
                    {
                        on_create_datareader_callback(participant, datareader);
                    }
                    break;
                }
                case CallbackKind::CREATE_REQUESTER:
                {
                    fastdds::dds::DomainParticipant* participant = va_arg(args, fastdds::dds::DomainParticipant*);
                    fastdds::dds::DataWriter* req_datawriter = va_arg(args, fastdds::dds::DataWriter*);
                    fastdds::dds::DataReader* repl_datareader = va_arg(args, fastdds::dds::DataReader*);
                    for (const auto& on_create_requester_callback : on_create_requester_callbacks_)
                    {
                        on_create_requester_callback(participant, req_datawriter, repl_datareader);
                    }
                    break;
                }
                case CallbackKind::CREATE_REPLIER:
                {
                    fastdds::dds::DomainParticipant* participant = va_arg(args, fastdds::dds::DomainParticipant*);
                    fastdds::dds::DataWriter* repl_datawriter = va_arg(args, fastdds::dds::DataWriter*);
                    fastdds::dds::DataReader* req_datareader = va_arg(args, fastdds::dds::DataReader*);
                    for (const auto& on_create_replier_callback : on_create_replier_callbacks_)
                    {
                        on_create_replier_callback(participant, repl_datawriter, req_datareader);
                    }
                    break;
                }
                case CallbackKind::DELETE_PARTICIPANT:
                {
                    fastdds::dds::DomainParticipant* participant = va_arg(args, fastdds::dds::DomainParticipant*);
                    for (const auto& on_delete_participant_callback : on_delete_participant_callbacks_)
                    {
                        on_delete_participant_callback(participant);
                    }
                    break;
                }
                case CallbackKind::DELETE_DATAWRITER:
                {
                    fastdds::dds::DomainParticipant* participant = va_arg(args, fastdds::dds::DomainParticipant*);
                    fastdds::dds::DataWriter* datawriter = va_arg(args, fastdds::dds::DataWriter*);
                    for (const auto& on_delete_datawriter_callback : on_delete_datawriter_callbacks_)
                    {
                        on_delete_datawriter_callback(participant, datawriter);
                    }
                    break;
                }
                case CallbackKind::DELETE_DATAREADER:
                {
                    fastdds::dds::DomainParticipant* participant = va_arg(args, fastdds::dds::DomainParticipant*);
                    fastdds::dds::DataReader* datareader = va_arg(args, fastdds::dds::DataReader*);
                    for (const auto& on_delete_datareader_callback : on_delete_datareader_callbacks_)
                    {
                        on_delete_datareader_callback(participant, datareader);
                    }
                    break;
                }
                case CallbackKind::DELETE_REQUESTER:
                {
                    fastdds::dds::DomainParticipant* participant = va_arg(args, fastdds::dds::DomainParticipant*);
                    fastdds::dds::DataWriter* req_datawriter = va_arg(args, fastdds::dds::DataWriter*);
                    fastdds::dds::DataReader* repl_datareader = va_arg(args, fastdds::dds::DataReader*);
                    for (const auto& on_delete_requester_callback : on_delete_requester_callbacks_)
                    {
                        on_delete_requester_callback(participant, req_datawriter, repl_datareader);
                    }
                    break;
                }
                case CallbackKind::DELETE_REPLIER:
                {
                    fastdds::dds::DomainParticipant* participant = va_arg(args, fastdds::dds::DomainParticipant*);
                    fastdds::dds::DataWriter* repl_datawriter = va_arg(args, fastdds::dds::DataWriter*);
                    fastdds::dds::DataReader* req_datareader = va_arg(args, fastdds::dds::DataReader*);
                    for (const auto& on_delete_replier_callback : on_delete_replier_callbacks_)
                    {
                        on_delete_replier_callback(participant, repl_datawriter, req_datareader);
                    }
                    break;
                }
            }
            va_end(args);
        }

    private:

        using CreateParticipantCallback = Callback<
                        const fastdds::dds::DomainParticipant*>;
        using DeleteParticipantCallback = Callback<
                        const fastdds::dds::DomainParticipant*>;
        using CreateDataWriterCallback = Callback<
                        const fastdds::dds::DomainParticipant*,
                        const fastdds::dds::DataWriter*>;

        using DeleteDataWriterCallback = Callback<
                        const fastdds::dds::DomainParticipant*,
                        const fastdds::dds::DataWriter*>;

        using CreateDataReaderCallback = Callback<
                        const fastdds::dds::DomainParticipant*,
                        const fastdds::dds::DataReader*>;

        using DeleteDataReaderCallback = Callback<
                        const fastdds::dds::DomainParticipant*,
                        const fastdds::dds::DataReader*>;

        using CreateRequesterCallback = Callback<
                        const fastdds::dds::DomainParticipant*,
                        const fastdds::dds::DataWriter*,
                        const fastdds::dds::DataReader*>;

        using DeleteRequesterCallback = Callback<
                        const fastdds::dds::DomainParticipant*,
                        const fastdds::dds::DataWriter*,
                        const fastdds::dds::DataReader*>;

        using CreateReplierCallback = Callback<
                        const fastdds::dds::DomainParticipant*,
                        const fastdds::dds::DataWriter*,
                        const fastdds::dds::DataReader*>;

        using DeleteReplierCallback = Callback<
                        const fastdds::dds::DomainParticipant*,
                        const fastdds::dds::DataWriter*,
                        const fastdds::dds::DataReader*>;

        std::vector<CreateParticipantCallback> on_create_participant_callbacks_;
        std::vector<DeleteParticipantCallback> on_delete_participant_callbacks_;
        std::vector<CreateDataWriterCallback> on_create_datawriter_callbacks_;
        std::vector<DeleteDataWriterCallback> on_delete_datawriter_callbacks_;
        std::vector<CreateDataReaderCallback> on_create_datareader_callbacks_;
        std::vector<DeleteDataReaderCallback> on_delete_datareader_callbacks_;
        std::vector<CreateRequesterCallback> on_create_requester_callbacks_;
        std::vector<DeleteRequesterCallback> on_delete_requester_callbacks_;
        std::vector<CreateReplierCallback> on_create_replier_callbacks_;
        std::vector<DeleteReplierCallback> on_delete_replier_callbacks_;
    };

    FastDDSCallbackFactory fast_dds_callback_factory_;
#endif // UAGENT_FAST_PROFILE

#ifdef UAGENT_CED_PROFILE
    /**********************************************************************************************************************
     * CEDCallbackFactory (skeleton class, not implemented)
     **********************************************************************************************************************/
    class CEDCallbackFactory
    {
    public:
        /**
         * @brief   Default constructor.
         */
        CEDCallbackFactory() = default;

        /**
         * @brief   Default destructor.
         */
        ~CEDCallbackFactory() = default;

        /**
         * @brief   CEDCallbackFactory shall not be copy constructible.
         */
        CEDCallbackFactory(
                const CEDCallbackFactory &) = delete;

        CEDCallbackFactory(
                CEDCallbackFactory &&) = delete;

        /**
         * @brief   CEDCallbackFactory shall not be copy assignable.
         */
        CEDCallbackFactory& operator = (
                const CEDCallbackFactory &) = delete;

        CEDCallbackFactory& operator = (
                CEDCallbackFactory &&) = delete;
    };
#endif // UAGENT_CED_PROFILE
};

/**********************************************************************************************************************
 * Template specializations for callback methods
 **********************************************************************************************************************/
#ifdef UAGENT_FAST_PROFILE
template <>
inline void CallbackFactory::FastCallbackFactory::add_callback<
    const fastrtps::Participant*,
    const fastrtps::Publisher*>(
        const CallbackKind& callback_kind,
        std::function<void (const fastrtps::Participant*,
                            const fastrtps::Publisher*)>&& callback_function)
{
    switch (callback_kind)
    {
        case CallbackKind::CREATE_DATAWRITER:
        {
            on_create_datawriter_callbacks_.emplace_back(Callback<
                const fastrtps::Participant*,
                const fastrtps::Publisher*>(std::move(callback_function)));
            break;
        }
        case CallbackKind::DELETE_DATAWRITER:
        {
            on_delete_datawriter_callbacks_.emplace_back(Callback<
                const fastrtps::Participant*,
                const fastrtps::Publisher*>(std::move(callback_function)));
            break;
        }
        default:
        {
            break;
        }
    }
}

template <>
inline void CallbackFactory::FastCallbackFactory::add_callback<
    const fastrtps::Participant*,
    const fastrtps::Subscriber*>(
        const CallbackKind& callback_kind,
        std::function<void (const fastrtps::Participant*,
                            const fastrtps::Subscriber*)>&& callback_function)
{
    switch (callback_kind)
    {
        case CallbackKind::CREATE_DATAREADER:
        {
            on_create_datareader_callbacks_.emplace_back(Callback<
                const fastrtps::Participant*,
                const fastrtps::Subscriber*>(std::move(callback_function)));
            break;
        }
        case CallbackKind::DELETE_DATAREADER:
        {
            on_delete_datareader_callbacks_.emplace_back(Callback<
                const fastrtps::Participant*,
                const fastrtps::Subscriber*>(std::move(callback_function)));
            break;
        }
        default:
        {
            break;
        }
    }
}

template <>
inline void CallbackFactory::FastCallbackFactory::add_callback<
    const fastrtps::Participant*,
    const fastrtps::Publisher*,
    const fastrtps::Subscriber*>(
        const CallbackKind& callback_kind,
        std::function<void (const fastrtps::Participant*,
                            const fastrtps::Publisher*,
                            const fastrtps::Subscriber*)>&& callback_function)
{
    switch (callback_kind)
    {
        case CallbackKind::CREATE_REQUESTER:
        {
            on_create_requester_callbacks_.emplace_back(Callback<
                const fastrtps::Participant*,
                const fastrtps::Publisher*,
                const fastrtps::Subscriber*>(std::move(callback_function)));
            break;
        }
        case CallbackKind::CREATE_REPLIER:
        {
            on_create_replier_callbacks_.emplace_back(Callback<
                const fastrtps::Participant*,
                const fastrtps::Publisher*,
                const fastrtps::Subscriber*>(std::move(callback_function)));
            break;
        }
        case CallbackKind::DELETE_REQUESTER:
        {
            on_delete_requester_callbacks_.emplace_back(Callback<
                const fastrtps::Participant*,
                const fastrtps::Publisher*,
                const fastrtps::Subscriber*>(std::move(callback_function)));
            break;
        }
        case CallbackKind::DELETE_REPLIER:
        {
            on_delete_replier_callbacks_.emplace_back(Callback<
                const fastrtps::Participant*,
                const fastrtps::Publisher*,
                const fastrtps::Subscriber*>(std::move(callback_function)));
            break;
        }
        default:
        {
            break;
        }
    }
}

template <>
inline void CallbackFactory::FastDDSCallbackFactory::add_callback<
    const fastdds::dds::DomainParticipant*,
    const fastdds::dds::DataWriter*>(
        const CallbackKind& callback_kind,
        std::function<void (const fastdds::dds::DomainParticipant*,
                            const fastdds::dds::DataWriter*)>&& callback_function)
{
    switch (callback_kind)
    {
        case CallbackKind::CREATE_DATAWRITER:
        {
            on_create_datawriter_callbacks_.emplace_back(Callback<
                const fastdds::dds::DomainParticipant*,
                const fastdds::dds::DataWriter*>(std::move(callback_function)));
            break;
        }
        case CallbackKind::DELETE_DATAWRITER:
        {
            on_delete_datawriter_callbacks_.emplace_back(Callback<
                const fastdds::dds::DomainParticipant*,
                const fastdds::dds::DataWriter*>(std::move(callback_function)));
            break;
        }
        default:
        {
            break;
        }
    }
}

template <>
inline void CallbackFactory::FastDDSCallbackFactory::add_callback<
    const fastdds::dds::DomainParticipant*,
    const fastdds::dds::DataReader*>(
        const CallbackKind& callback_kind,
        std::function<void (const fastdds::dds::DomainParticipant*,
                            const fastdds::dds::DataReader*)>&& callback_function)
{
    switch (callback_kind)
    {
        case CallbackKind::CREATE_DATAREADER:
        {
            on_create_datareader_callbacks_.emplace_back(Callback<
                const fastdds::dds::DomainParticipant*,
                const fastdds::dds::DataReader*>(std::move(callback_function)));
            break;
        }
        case CallbackKind::DELETE_DATAREADER:
        {
            on_delete_datareader_callbacks_.emplace_back(Callback<
                const fastdds::dds::DomainParticipant*,
                const fastdds::dds::DataReader*>(std::move(callback_function)));
            break;
        }
        default:
        {
            break;
        }
    }
}

template <>
inline void CallbackFactory::FastDDSCallbackFactory::add_callback<
    const fastdds::dds::DomainParticipant*,
    const fastdds::dds::DataWriter*,
    const fastdds::dds::DataReader*>(
        const CallbackKind& callback_kind,
        std::function<void (const fastdds::dds::DomainParticipant*,
                            const fastdds::dds::DataWriter*,
                            const fastdds::dds::DataReader*)>&& callback_function)
{
    switch (callback_kind)
    {
        case CallbackKind::CREATE_REQUESTER:
        {
            on_create_requester_callbacks_.emplace_back(Callback<
                const fastdds::dds::DomainParticipant*,
                const fastdds::dds::DataWriter*,
                const fastdds::dds::DataReader*>(std::move(callback_function)));
            break;
        }
        case CallbackKind::CREATE_REPLIER:
        {
            on_create_replier_callbacks_.emplace_back(Callback<
                const fastdds::dds::DomainParticipant*,
                const fastdds::dds::DataWriter*,
                const fastdds::dds::DataReader*>(std::move(callback_function)));
            break;
        }
        case CallbackKind::DELETE_REQUESTER:
        {
            on_delete_requester_callbacks_.emplace_back(Callback<
                const fastdds::dds::DomainParticipant*,
                const fastdds::dds::DataWriter*,
                const fastdds::dds::DataReader*>(std::move(callback_function)));
            break;
        }
        case CallbackKind::DELETE_REPLIER:
        {
            on_delete_replier_callbacks_.emplace_back(Callback<
                const fastdds::dds::DomainParticipant*,
                const fastdds::dds::DataWriter*,
                const fastdds::dds::DataReader*>(std::move(callback_function)));
            break;
        }
        default:
        {
            break;
        }
    }
}
#endif  // UAGENT_FAST_PROFILE

#define CALLBACK_FACTORY_ADD_CALLBACK(MIDDLEWARE, CB_FACTORY, ...) \
template <> \
inline void CallbackFactory::add_callback<__VA_ARGS__>( \
        const Middleware::Kind& middleware_kind, \
        const CallbackKind& callback_kind, \
        std::function<void (__VA_ARGS__)>&& callback_function) \
{ \
    switch (middleware_kind) \
    { \
        case MIDDLEWARE: \
        { \
            CB_FACTORY.add_callback(callback_kind, std::move(callback_function)); \
            break; \
        } \
        default: \
        { \
            break; \
        } \
    } \
}

#ifdef UAGENT_FAST_PROFILE
#define CALLBACK_FACTORY_ADD_FASTRTPS_CALLBACK(...) \
    CALLBACK_FACTORY_ADD_CALLBACK(Middleware::Kind::FASTRTPS, fast_callback_factory_, __VA_ARGS__)

CALLBACK_FACTORY_ADD_FASTRTPS_CALLBACK(
    const fastrtps::Participant*)

CALLBACK_FACTORY_ADD_FASTRTPS_CALLBACK(
    const fastrtps::Participant*,
    const fastrtps::Publisher*)

CALLBACK_FACTORY_ADD_FASTRTPS_CALLBACK(
    const fastrtps::Participant*,
    const fastrtps::Subscriber*)

CALLBACK_FACTORY_ADD_FASTRTPS_CALLBACK(
    const fastrtps::Participant*,
    const fastrtps::Publisher*,
    const fastrtps::Subscriber*)

#define CALLBACK_FACTORY_ADD_FASTDDS_CALLBACK(...) \
    CALLBACK_FACTORY_ADD_CALLBACK(Middleware::Kind::FASTDDS, fast_dds_callback_factory_, __VA_ARGS__)

CALLBACK_FACTORY_ADD_FASTDDS_CALLBACK(
    const fastdds::dds::DomainParticipant*)

CALLBACK_FACTORY_ADD_FASTDDS_CALLBACK(
    const fastdds::dds::DomainParticipant*,
    const fastdds::dds::DataWriter*)

CALLBACK_FACTORY_ADD_FASTDDS_CALLBACK(
    const fastdds::dds::DomainParticipant*,
    const fastdds::dds::DataReader*)

CALLBACK_FACTORY_ADD_FASTDDS_CALLBACK(
    const fastdds::dds::DomainParticipant*,
    const fastdds::dds::DataWriter*,
    const fastdds::dds::DataReader*)
#endif  // UAGENT_FAST_PROFILE

} // namespace middleware
} // namespace uxr
} // namespace eprosima

#endif // UXR__AGENT__MIDDLEWARE__UTILS__CALLBACKS_HPP_
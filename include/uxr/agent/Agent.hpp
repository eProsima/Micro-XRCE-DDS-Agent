// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef UXR_AGENT_AGENT_HPP_
#define UXR_AGENT_AGENT_HPP_

#include <uxr/agent/visibility.hpp>
#include <uxr/agent/middleware/Middleware.hpp>

#include <cstdint>
#include <string>
#include <memory>

namespace eprosima {
namespace uxr {
namespace middleware {
/**
 * @brief Forward declarations.
 */
enum class CallbackKind : uint8_t;
class CallbackFactory;
} //  namespace middleware

class Root;

class Agent
{
public:
    /**
     * @brief Indicates the result status of the operation perform over the Agent.
     */
    enum OpResult : uint8_t
    {
        /** Indicates a successful execution of the operation. */
        OK                      = 0x00,
        /** Indicates a successful execution of a create operation
         *  when entity exists and matches with the one requested. */
        OK_MATCHED              = 0x01,
        /** Indicates a failure in the operation caused by an error in the DDS layer. */
        DDS_ERROR               = 0x80,
        /** Indicates a failure in a create operation when the entity does not match with the one requested. */
        MISMATCH_ERROR          = 0x81,
        /** Indicates a failure in a create operation when the entity already exists. */
        ALREADY_EXISTS_ERROR    = 0x82,
        /** Indicates a failure in the operation due to lack of permissions. */
        DENIED_ERROR            = 0x83,
        /** Indicates a failure in the operation due to the fact that the reference does not exist. */
        UNKNOWN_REFERENCE_ERROR = 0x84,
        /** Indicates a failure in the operation due to an incorrect input. */
        INVALID_DATA_ERROR      = 0x85,
        /** Indicates a failure in the operation due to an incompatibility between the Client and the Agent. */
        INCOMPATIBLE_ERROR      = 0x86,
        /** Indicates a failure in the operation due to a resource error on the Agent. */
        RESOURCES_ERROR         = 0x87,
        /** Indicates a failure in a write operation. */
        WRITE_ERROR             = 0xF0
    };

    /**
     * @brief These flags determines the behaviour of the create operation.
     *        According to the DDS-XRCE standart, the following modes are defined:
     *
     *        | REUSE_MODE | REPLACE_MODE | Exists |  Behaviour                                                  |
     *        |:----------:|:------------:|:------:|:------------------------------------------------------------|
     *        | no matter  |   no matter  |   no   | The XRCEObject is created normally.                         |
     *        |   false    |     false    |   yes  | Nothing is done and an ALREADY_EXISTS_ERROR is returned.    |
     *        |   false    |     true     |   yes  | The existing XRCEObject is remove and a new one is creates. |
     *        |   true     |     false    |   yes  | Nothing is done and an OK_MATCHED is returned in case of matching, and a MISMATCH_ERROR in other case. |
     *        |   true     |     true     |   yes  | Nothing is done and an OK_MATCHED is returned in case of matching, and a new XRCEObject and an OK is returned in other case. |
     */
    enum CreationFlag : uint8_t
    {
        REUSE_MODE      = 0x01 << 1,
        REPLACE_MODE    = 0x01 << 2
    };

    /**
     * @brief The identifiers of the XRCEObject types.
     */
    enum ObjectKind : uint8_t
    {
        /** Identifies a Participant. */
        PARTICIPANT_OBJK    = 0x01,
        /** Identifies a Topic. */
        TOPIC_OBJK          = 0x02,
        /** Identifies a Publisher. */
        PUBLISHER_OBJK      = 0x03,
        /** Identifies a Subscriber. */
        SUBSCRIBER_OBJK     = 0x04,
        /** Identifies a DataWriter. */
        DATAWRITER_OBJK     = 0x05,
        /** Identifies a DataReader. */
        DATAREADER_OBJK     = 0x06,
        /** Identifies a Requester. */
        REQUESTER_OBJK      = 0x07,
        /** Identifies a Replier. */
        REPLIER_OBJK        = 0x08
    };

    UXR_AGENT_EXPORT Agent();
    UXR_AGENT_EXPORT ~Agent();

    /**
     * @brief Creates a ProxyClient which can be reused by an external Client.
     * @param key               The identifier of the ProxyClient.
     * @param session           The identifier of the Session attached to the ProxyClient.
     * @param mtu               The Maximum Transmission Unit (MTU) of the Session.
     * @param middleware_kind   The middleware used by the Client. It could be DDS or Centralized Data middleware.
     * @param op_result         The result status of the operation.
     * @return  true in case of success and false in other case.
     */
    UXR_AGENT_EXPORT bool create_client(
            uint32_t key,
            uint8_t session,
            uint16_t mtu,
            Middleware::Kind middleware_kind,
            OpResult& op_result);

    /**
     * @brief Deletes a ProxyClient from the Root.
     * @param key       The identifier of the ProxyClient to be removed.
     * @param op_result The result status of the operation.
     * @return  true in case of success and false in other case.
     */
    UXR_AGENT_EXPORT bool delete_client(
            uint32_t key,
            OpResult& op_result);

    /**
     * @brief Creates a Participant from a reference in the ProxyClient identified by the client_key.
     * @param client_key        The identifier of the ProxyClient.
     * @param participant_id    The identifier of the Participant to be created.
     * @param domain_id         The domain associated with the Participant.
     * @param ref               The reference of the Participant.
     * @param flag              The flag that determines the creation mode.
     * @param op_result         The result status of the operation.
     * @return  true in case of success and false in other case.
     */
    UXR_AGENT_EXPORT bool create_participant_by_ref(
            uint32_t client_key,
            uint16_t participant_id,
            int16_t domain_id,
            const char* ref,
            uint8_t flag,
            OpResult& op_result);

    /**
     * @brief Creates a Participant from an XML in the ProxyClient identified by the client_key.
     * @param client_key        The identifier of the ProxyClient.
     * @param participant_id    The identifier of the Participant to be created.
     * @param domain_id         The domain associated with the Participant.
     * @param xml               The XML that describes the Participant.
     * @param flag              The flag that determines the creation mode.
     * @param op_result         The result status of the operation.
     * @return  true in case of success and false in other case.
     */
    UXR_AGENT_EXPORT bool create_participant_by_xml(
            uint32_t client_key,
            uint16_t participant_id,
            int16_t domain_id,
            const char* xml,
            uint8_t flag,
            OpResult& op_result);

    /**
     * @brief Deletes the Participant from the ProxyClient.
     *        This operation removes all the participant hiearchy.
     * @param client_key        The identifier of the ProxyClient to delete from.
     * @param participant_id    The identifier of the Participant to delete.
     * @param op_result         The result status of the operation.
     * @return true in case of success and false in other case.
     */
    UXR_AGENT_EXPORT bool delete_participant(
            uint32_t client_key,
            uint16_t participant_id,
            OpResult& op_result);

    /**
     * @brief Creates a Topic from a reference in the ProxyClient identified by the client_key.
     *        The Topic is associated with the Participant identifier by the participant_id.
     * @param client_key        The identifier of the ProxyClient.
     * @param topic_id          The identifier of the Topic to be created.
     * @param participant_id    The identifier of the Participant associated with the Topic.
     * @param ref               The reference of the Topic.
     * @param flag              The flag that determines the creation mode.
     * @param op_result         The result status of the operation.
     * @return  true in case of success and false in other case.
     */
    UXR_AGENT_EXPORT bool create_topic_by_ref(
            uint32_t client_key,
            uint16_t topic_id,
            uint16_t participant_id,
            const char* ref,
            uint8_t flag,
            OpResult& op_result);

    /**
     * @brief Creates a Topic from an XML in the ProxyClient identified by the client_key.
     *        The Topic is associated with the Participant identifier by the participant_id.
     * @param client_key        The identifier of the ProxyClient.
     * @param topic_id          The identifier of the Topic to be created.
     * @param participant_id    The identifier of the Participant associated with the Topic.
     * @param xml               The XML that describes the Topic.
     * @param flag              The flag that determines the creation mode.
     * @param op_result         The result status of the operation.
     * @return  true in case of success and false in other case.
     */
    UXR_AGENT_EXPORT bool create_topic_by_xml(
            uint32_t client_key,
            uint16_t topic_id,
            uint16_t participant_id,
            const char* xml,
            uint8_t flag,
            OpResult& op_result);

    /**
     * @brief Deletes the Topic from the ProxyClient.
     *        This operation removes all the DataWriters or DataReaders associated with the Topic.
     * @param client_key    The identifier of the ProxyClient to delete from.
     * @param topic_id      The identifier of the Topic to delete.
     * @param op_result     The result status of the operation.
     * @return true in case of success and false in other case.
     */
    UXR_AGENT_EXPORT bool delete_topic(
            uint32_t client_key,
            uint16_t topic_id,
            OpResult& op_result);

    /**
     * @brief Creates a Publisher from an XML in the ProxyClient identified by the client_key.
     *        The Publisher is associated with the Participant identifier by the participant_id.
     * @param client_key        The identifier of the ProxyClient.
     * @param publisher_id      The identifier of the Publisher to be created.
     * @param participant_id    The identifier of the Participant associated with the Publisher.
     * @param xml               The XML of the Publisher.
     * @param flag              The flag that determines the creation mode.
     * @param op_result         The result status of the operation.
     * @return  true in case of success and false in other case.
     */
    UXR_AGENT_EXPORT bool create_publisher_by_xml(
            uint32_t client_key,
            uint16_t publisher_id,
            uint16_t participant_id,
            const char* xml,
            uint8_t flag,
            OpResult& op_result);

    /**
     * @brief Deletes the publisher from a ProxyClient.
     *        This operation removes all the DataWriters associated with the Publisher.
     * @param client_key    The identifier of the ProxyClient to delete from.
     * @param publisher_id  The identifier of the Publisher to delete.
     * @param op_result     The result status of the operation.
     * @return true in case of success and false in other case.
     */
    UXR_AGENT_EXPORT bool delete_publisher(
            uint32_t client_key,
            uint16_t publisher_id,
            OpResult& op_result);

    /**
     * @brief Creates a Subscriber from an XML in the ProxyClient identified by the client_key.
     *        The Subscriber is associated with the Participant identifier by the participant_id.
     * @param client_key        The identifier of the ProxyClient.
     * @param subscriber_id     The identifier of the Subscriber to be created.
     * @param participant_id    The identifier of the Participant associated with the Publisher.
     * @param xml               The XML that describes the Subscriber.
     * @param flag              The flag that determines the creation mode.
     * @param op_result         The result status of the operation.
     * @return  true in case of success and false in other case.
     */
    UXR_AGENT_EXPORT bool create_subscriber_by_xml(
            uint32_t client_key,
            uint16_t subscriber_id,
            uint16_t participant_id,
            const char* xml,
            uint8_t flag,
            OpResult& op_result);

    /**
     * @brief Deletes the subscriber from a ProxyClient.
     *        This operation removes all the DataReaders associated with the Subscriber.
     * @param client_key    The identifier of the ProxyClient to delete from.
     * @param client_key    The identifier of the ProxyClient to delete from.
     * @param subscriber_id The identifier of the Subscriber to delete.
     * @param op_result     The result status of the operation.
     * @return true in case of success and false in other case.
     */
    UXR_AGENT_EXPORT bool delete_subscriber(
            uint32_t client_key,
            uint16_t subscriber_id,
            OpResult& op_result);

    /**
     * @brief Creates a DataWriter from a reference in the ProxyClient identified by the client_key.
     *        The DataWriter is associated with a Publisher identified by publisher_id.
     * @param client_key        The identifier of the ProxyClient.
     * @param datawriter_id     The identifier of the DataWriter to be created.
     * @param publisher_id      The identifier of the Publisher associated with the DataWriter.
     * @param ref               The reference of the DataWriter.
     * @param flag              The flag that determines the creation mode.
     * @param op_result         The result status of the operation.
     * @return  true in case of success and false in other case.
     */
    UXR_AGENT_EXPORT bool create_datawriter_by_ref(
            uint32_t client_key,
            uint16_t datawriter_id,
            uint16_t publisher_id,
            const char* ref,
            uint8_t flag,
            OpResult& op_result);

    /**
     * @brief Creates a DataWriter from an XML in the ProxyClient identified by the client_key.
     *        The DataWriter is associated with a Publisher identified by publisher_id.
     * @param client_key        The identifier of the ProxyClient.
     * @param datawriter_id     The identifier of the DataWriter to be created.
     * @param publisher_id      The identifier of the Publisher associated with the DataWriter.
     * @param xml               The XML that describes the DataWriter.
     * @param flag              The flag that determines the creation mode.
     * @param op_result         The result status of the operation.
     * @return  true in case of success and false in other case.
     */
    UXR_AGENT_EXPORT bool create_datawriter_by_xml(
            uint32_t client_key,
            uint16_t datawriter_id,
            uint16_t publisher_id,
            const char* xml,
            uint8_t flag,
            OpResult& op_result);

    /**
     * @brief Deletes the DataWriter from a ProxyClient.
     * @param client_key    The identifier of the ProxyClient to delete from.
     * @param datawriter_id The identifier of the DataWriter to delete.
     * @param op_result     The result status of the operation.
     * @return true in case of success and false in other case.
     */
    UXR_AGENT_EXPORT bool delete_datawriter(
            uint32_t client_key,
            uint16_t datawriter_id,
            OpResult& op_result);

    /**
     * @brief Creates a DataReader from a reference in the ProxyClient identified by the client_key.
     *        The DataReader is associated with a Subscriber identified by subscriber_id.
     * @param client_key        The identifier of the ProxyClient.
     * @param datareader_id     The identifier of the DataReader to be created.
     * @param subscriber_id     The identifier of the Subscriber associated with the DataReader.
     * @param ref               The reference of the DataReader.
     * @param flag              The flag that determines the creation mode.
     * @param op_result         The result status of the operation.
     * @return  true in case of success and false in other case.
     */
    UXR_AGENT_EXPORT bool create_datareader_by_ref(
            uint32_t client_key,
            uint16_t datareader_id,
            uint16_t subscriber_id,
            const char* ref,
            uint8_t flag,
            OpResult& op_result);

    /**
     * @brief Creates a DataReader from an XML in the ProxyClient identified by the client_key.
     *        The DataReader is associated with a Subscriber identified by subscriber_id.
     * @param client_key        The identifier of the ProxyClient.
     * @param datareader_id     The identifier of the DataReader to be created.
     * @param subscriber_id     The identifier of the Subscriber associated with the DataReader.
     * @param xml               The XML that describes the DataReader.
     * @param flag              The flag that determines the creation mode.
     * @param op_result         The result status of the operation.
     * @return  true in case of success and false in other case.
     */
    UXR_AGENT_EXPORT bool create_datareader_by_xml(
            uint32_t client_key,
            uint16_t datareader_id,
            uint16_t subscriber_id,
            const char* xml,
            uint8_t flag,
            OpResult& op_result);

    /**
     * @brief Deletes the DataReader from a ProxyClient.
     * @param client_key    The identifier of the ProxyClient to delete from.
     * @param datareader_id The identifier of the DataReader to delete.
     * @param op_result     The result status of the operation.
     * @return true in case of success and false in other case.
     */
    UXR_AGENT_EXPORT bool delete_datareader(
            uint32_t client_key,
            uint16_t datareader_id,
            OpResult& op_result);

    /**
     * @brief Creates a Requester from a reference in the ProxyClient identified by the client_key.
     *        The Requester is associated with a Participant identified by participant_id.
     *
     * @param client_key        The identifier of the ProxyClient.
     * @param requester_id      The identifier of the Requester to be created.
     * @param participant_id    The identifier of the Participant associated with the Requester.
     * @param ref               The reference of the Requester.
     * @param flag              The flag tha determines the creation mode.
     * @param op_result         The result status of the operation.
     * @return true in case of successful creation and false in other case.
     */
    UXR_AGENT_EXPORT bool create_requester_by_ref(
            uint32_t client_key,
            uint16_t requester_id,
            uint16_t participant_id,
            const char* ref,
            uint8_t flag,
            OpResult& op_result);

    /**
     * @brief Creates a Requester from an XML in the ProxyClient identified by the client_key.
     *        The Requester is associated with a Participant identified by participant_id.
     *
     * @param client_key        The identifier of the ProxyClient.
     * @param requester_id      The identifier of the Requester to be created.
     * @param participant_id    The identifier of the Participant associated with the Requester.
     * @param xml               The XML that describes the Requester.
     * @param flag              The flag tha determines the creation mode.
     * @param op_result         The result status of the operation.
     * @return true in case of successful creation and false in other case.
     */
    UXR_AGENT_EXPORT bool create_requester_by_xml(
            uint32_t client_key,
            uint16_t requester_id,
            uint16_t participant_id,
            const char* xml,
            uint8_t flag,
            OpResult& op_result);

    /**
     * @brief Deletes a Requester from a ProxyClient.
     *
     * @param client_key        The identifier of the ProxyClient to delete from.
     * @param requester_id      The identifier of the Requester to delete.
     * @param op_result         The result status of the operation.
     * @return true in case of successful deletion and false in other case.
     */
    UXR_AGENT_EXPORT bool delete_requester(
            uint32_t client_key,
            uint16_t requester_id,
            OpResult& op_result);

    /**
     * @brief Creates a Replier from a reference in the ProxyClient identified by the client_key.
     *        The Replier is associated with a Participant identified by participant_id.
     *
     * @param client_key        The identifier of the ProxyClient.
     * @param replier_id        The identifier of the Replier to be created.
     * @param participant_id    The identifier of the Participant associated with the Replier.
     * @param ref               The reference of the Replier.
     * @param flag              The flag tha determines the creation mode.
     * @param op_result         The result status of the operation.
     * @return true in case of successful creation and false in other case.
     */
    UXR_AGENT_EXPORT bool create_replier_by_ref(
            uint32_t client_key,
            uint16_t replier_id,
            uint16_t participant_id,
            const char* ref,
            uint8_t flag,
            OpResult& op_result);

    /**
     * @brief Creates a Replier from an XML in the ProxyClient identified by the client_key.
     *        The Replier is associated with a Participant identified by participant_id.
     *
     * @param client_key        The identifier of the ProxyClient.
     * @param replier_id        The identifier of the Replier to be created.
     * @param participant_id    The identifier of the Participant associated with the Replier.
     * @param xml               The XML that describes the Requester.
     * @param flag              The flag tha determines the creation mode.
     * @param op_result         The result status of the operation.
     * @return true in case of successful creation and false in other case.
     */
    UXR_AGENT_EXPORT bool create_replier_by_xml(
            uint32_t client_key,
            uint16_t requester_id,
            uint16_t participant_id,
            const char* xml,
            uint8_t flag,
            OpResult& op_result);

    /**
     * @brief Deletes a Replier from a ProxyClient.
     *
     * @param client_key        The identifier of the ProxyClient to delete from.
     * @param requester_id      The identifier of the Replier to delete.
     * @param op_result         The result status of the operation.
     * @return true in case of successful creation and false in other case.
     */
    UXR_AGENT_EXPORT bool delete_replier(
            uint32_t client_key,
            uint16_t replier_id,
            OpResult& op_result);

    /**
     * @brief Loads a configuration file which provides the references use to create XRCE object by reference.
     *        This file shall use the
     *        [Fast RTPS profile syntax](https://fast-rtps.docs.eprosima.com/en/latest/xmlprofiles.html),
     *        where the `profile_name` attributes represent the references.
     * @param file_path The file path relative to the working directory.
     * @return true in case of successful reading, false in other case.
     */
    UXR_AGENT_EXPORT bool load_config_file(const std::string& file_path);

    /**
     * @brief Resets the Root object, that is, removes all the ProxyClients and their entities.
     */
    UXR_AGENT_EXPORT void reset();

    /**
     * @brief Writes data into the middleware using the DataWriter identifier by the datawriter_id.
     * @param client_key        The identifier of the ProxyClient.
     * @param datawriter_id     The identifier of the DataWriter.
     * @param buf               The pointer to the buffer to write.
     * @param len               The lenght of the buffer to write.
     * @param op_result         The result status of the operation.
     * @return true in case of success and false in other case.
     */
    UXR_AGENT_EXPORT bool write(
            uint32_t client_key,
            uint16_t datawriter_id,
            uint8_t* buf,
            size_t len,
            OpResult& op_result);

    /**
     * @brief Sets the verbose level of the logger.
     * @param verbose_level The verbose level of the logger.
     *                      There are 7 different levels:
     *                      * Level 0: no message will be logged (logger is turned off).
     *                      * Level 1: only critical messages will be logged.
     *                      * Level 2: error and critical messages will be logged.
     *                      * Level 3: warning, error and critical messages will be logged.
     *                      * Level 4: info, warning, error and critical message will be logged.
     *                      * Level 5: debug, info, warning, error and critical message will be logged.
     *                      * Level 6: trace, debug, info, warning, error and critical message will be logged.
     */
    UXR_AGENT_EXPORT void set_verbose_level(uint8_t verbose_level);

    /**
     * @brief Sets a callback function for an specific create/delete middleware entity operation.
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
    template<Agent::ObjectKind object_kind, typename U, typename T>
    bool create_object(
            uint32_t client_key,
            uint16_t raw_id,
            T parent_id,
            U rep,
            uint8_t flag,
            Agent::OpResult& op_result);

    template<Agent::ObjectKind object_kind>
    bool delete_object(
            uint32_t client_key,
            uint16_t raw_id,
            Agent::OpResult& op_result);

protected:
    std::unique_ptr<Root> root_;
    middleware::CallbackFactory& callback_factory_;
};

} // uxr
} // eprosima

#endif // UXR_AGENT_AGENT_HPP_

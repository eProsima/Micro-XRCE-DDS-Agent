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


#ifndef UXR_AGENT_MIDDLEWARE_CED_CED_MIDDLEWARE_HPP_
#define UXR_AGENT_MIDDLEWARE_CED_CED_MIDDLEWARE_HPP_

#include <uxr/agent/middleware/Middleware.hpp>
#include <uxr/agent/middleware/ced/CedEntities.hpp>

namespace eprosima {
namespace uxr {

class CedMiddleware : public Middleware
{
public:
    CedMiddleware(uint32_t client_key);

    ~CedMiddleware() override = default;

    /**
     * @startuml
     * !include agent/middleware/ced/create_participant.puml!0
     * @enduml
     *
     * @brief Creates a CedParticipant from a reference.
     *        Currently, only the domain_id is taken into account for the creation.
     *        In a near future, the Middleware interface should change the ref parameter by an "attributes" one.
     *        For this purpose an XML parser and a reference data base are needed in the XRCE Core.
     * @param participant_id    The CedParticipant identifier.
     * @param domain_id         The Domain identifier to which the CedParticipant belongs.
     * @param ref               NOT USED.
     * @return  true in case of creation and false in other case.
     */
    bool create_participant_by_ref(
            uint16_t participant_id,
            int16_t domain_id,
            const std::string&) override;

    /**
     * @brief Creates a CedParticipant from an XML.
     *        Currently, only the domain_id is taken into account for the creation.
     *        In a near future, the Middleware interface should change the xml parameter by an "attributes" one.
     *        For this purpose an XML parser and a reference data base are needed in the XRCE Core.
     * @param participant_id    The CedParticipant identifier.
     * @param domain_id         The Domain identifier to which the CedParticipant belongs.
     * @param xml               NOT USED.
     * @return  true in case of creation and false in other case.
     */
    bool create_participant_by_xml(
            uint16_t participant_id,
            int16_t domain_id,
            const std::string&) override;

    /**
     * @brief Creates a CedParticipant from a binary reference.
     *        Currently, only the domain_id is taken into account for the creation.
     * @param participant_id    The CedParticipant identifier.
     * @param participant_xrce  XRCE Participant binary representation, NOT USED.
     * @return  true in case of creation and false in other case.
     */
    bool create_participant_by_bin(
            uint16_t participant_id,
            const dds::xrce::OBJK_DomainParticipant_Binary& participant_xrce) override;

    /**
     * @startuml
     * !include agent/middleware/ced/create_topic.puml!0
     * @enduml
     *
     * @brief Creates a CedTopic associated to a CedParticipant from a reference.
     *        Currently, the ref paramenter is used as topic name.
     *        In a near future, the Middleware interface should change the ref parameter by an "attributes" one.
     *        For this purpose an XML parser and a reference data base are needed in the XRCE Core.
     *        With the aforementioned modification the topic name will be extracted from the "attributes".
     * @param topic_id          The CedTopic identifier.
     * @param participant_id    The CedParticipant identifier to which the CedTopic is associated.
     * @param ref               The CedTopic reference. Currently, it is used as the topic name.
     * @return  true in case of creation and false in other case.
     */
    bool create_topic_by_ref(
            uint16_t topic_id,
            uint16_t participant_id,
            const std::string& ref) override;

    /**
     * @brief Creates a CedTopic associated to a CedParticipant from an XML.
     *        Currently, the xml paramenter is used as topic name.
     *        In a near future, the Middleware interface should change the xml parameter by an "attributes" one.
     *        For this purpose an XML parser and a reference data base are needed in the XRCE Core.
     *        With the aforementioned modification the topic name will be extracted from the "attributes".
     * @param topic_id          The CedTopic identifier.
     * @param participant_id    The CedParticipant identifier to which the CedTopic is associated.
     * @param xml               The XML that describes the CedTopic. Currently, it is used as the topic name.
     * @return  true in case of creation and false in other case.
     */
    bool create_topic_by_xml(
            uint16_t topic_id,
            uint16_t participant_id,
            const std::string& xml) override;

    /**
     * @brief Creates a CedTopic associated to a CedParticipant from a binary reference.
     * @param topic_id          The CedTopic identifier.
     * @param participant_id    The CedParticipant identifier to which the CedTopic is associated.
     * @param topic_xrce  		XRCE Topic binary representation.
     * @return  true in case of creation and false in other case.
     */
    bool create_topic_by_bin(
            uint16_t topic_id,
            uint16_t participant_id,
            const dds::xrce::OBJK_Topic_Binary& topic_xrce) override;

    /**
     * @startuml
     * !include agent/middleware/ced/create_publisher.puml!0
     * @enduml
     *
     * @brief Creates a CedPublisher associated to a CedParticipant.
     * @param publisher_id      The CedPublisher identifier.
     * @param participant_id    The CedParticipant identifier to which the CedPublisher is associated.
     * @return  true in case of creation and false in other case.
     */
    bool create_publisher_by_xml(
            uint16_t publisher_id,
            uint16_t participant_id,
            const std::string&) override;

    /**
     * @brief Creates a CedPublisher associated to a CedParticipant using a binary reference.
     * @param publisher_id      The CedPublisher identifier.
     * @param participant_id    The CedParticipant identifier to which the CedPublisher is associated.
     * @param publisher_xrce  	XRCE Publisher binary representation, NOT USED.
     * @return  true in case of creation and false in other case.
     */
    bool create_publisher_by_bin(
            uint16_t publisher_id,
            uint16_t participant_id,
            const dds::xrce::OBJK_Publisher_Binary& publisher_xrce) override;

    /**
     * @startuml
     * !include agent/middleware/ced/create_subscriber.puml!0
     * @enduml
     *
     * @brief Creates a CedSubscriber associated to a CedParticipant.
     * @param subscirber_id     The CedSubscriber identifier.
     * @param participant_id    The CedParticipant identifier to which the CedSubscriber is associated.
     * @return  true in case of creation and false in other case.
     */
    bool create_subscriber_by_xml(
            uint16_t subscirber_id,
            uint16_t participant_id,
            const std::string&) override;

    /**
     * @brief Creates a CedSubscriber associated to a CedParticipant using a binary reference.
     * @param subscirber_id     The CedSubscriber identifier.
     * @param participant_id    The CedParticipant identifier to which the CedSubscriber is associated.
     * @param subscriber_xrce   XRCE Subscriber binary representation, NOT USED.
     * @return  true in case of creation and false in other case.
     */
    bool create_subscriber_by_bin(
            uint16_t subscriber_id,
            uint16_t participant_id,
            const dds::xrce::OBJK_Subscriber_Binary& subscriber_xrce) override;
            
    /**
     * @startuml
     * !include agent/middleware/ced/create_datawriter.puml!0
     * @enduml
     *
     * @brief Creates a CedDataWriter associated to a CedPublisher from a reference.
     * @param datawriter_id         The CedDataWriter identifier.
     * @param publisher_id          The CedPublisher identifier.
     * @param ref                   The CedDataWriter reference. Currently, it is used as the topic name.
     * @return  true in case of creation and false in other case.
     */
    bool create_datawriter_by_ref(
            uint16_t datawriter_id,
            uint16_t publisher_id,
            const std::string& ref) override;

    /**
     * @brief Creates a CedDataWriter associated to a CedPublisher from an XML.
     * @param datawriter_id         The CedDataWriter identifier.
     * @param publisher_id          The CedPublisher identifier.
     * @param xml                   The XML that describes the CedDataWriter. Currently, it is used as the topic name.
     * @return  true in case of creation and false in other case.
     */
    bool create_datawriter_by_xml(
            uint16_t datawriter_id,
            uint16_t publisher_id,
            const std::string& xml) override;

    /**
     * @brief Creates a CedDataWriter associated to a CedPublisher from a binary reference.
     * @param datawriter_id  	The CedDataWriter identifier.
     * @param publisher_id  	The CedPublisher identifier.
	 * @param datawriter_xrce  	XRCE DataWriter binary representation.
     * @return  true in case of creation and false in other case.
     */
    bool create_datawriter_by_bin(
            uint16_t datawriter_id,
            uint16_t publisher_id,
            const dds::xrce::OBJK_DataWriter_Binary& datawriter_xrce) override;

    /**
     * @startuml
     * !include agent/middleware/ced/create_datareader.puml!0
     * @enduml
     *
     * @brief Creates a CedDataReader associated to a CedSubscriber from a reference.
     * @param datareader_id         The CedDataReader identifier.
     * @param subscriber_id         The CedSubscriber identifier.
     * @param ref                   The CedDataReader reference. Currently, it is used as the topic name.
     * @return  true in case of creation and false in other case.
     */
    bool create_datareader_by_ref(
            uint16_t datareader_id,
            uint16_t subscriber_id,
            const std::string& ref) override;

    /**
     * @brief Creates a CedDataReader associated to a CedSubscriber from an XML.
     * @param datareader_id         The CedDataReader identifier.
     * @param subscriber_id         The CedSubscriber identifier.
     * @param xml                   The XML that describes the CedDataReader. Currently, it is used as the topic name.
     * @return  true in case of creation and false in other case.
     */
    bool create_datareader_by_xml(
            uint16_t datareader_id,
            uint16_t subscriber_id,
            const std::string& xml) override;

    /**
     * @brief Creates a CedDataReader associated to a CedSubscriber from a binary reference.
     * @param datareader_id   	The CedDataReader identifier.
     * @param subscriber_id  	The CedSubscriber identifier.
     * @param datawriter_xrce 	XRCE DataReader binary representation.
     * @return  true in case of creation and false in other case.
     */
    bool create_datareader_by_bin(
            uint16_t datareader_id,
            uint16_t subscriber_id,
            const dds::xrce::OBJK_DataReader_Binary& datareader_xrce) override;

    /**
     * @brief Not implemented.
     * 
     * @param requester_id 
     * @param participant_id 
     * @param ref 
     * @return false 
     */
    bool create_requester_by_ref(
            uint16_t,
            uint16_t,
            const std::string&) override { return false; };

    /**
     * @brief Not implemented.
     * 
     * @param requester_id 
     * @param participant_id 
     * @param xml
     * @return false 
     */
    bool create_requester_by_xml(
            uint16_t,
            uint16_t,
            const std::string&) override { return false; };

    /**
     * @brief Not implemented.
     * 
     * @param requester_id 
     * @param participant_id 
     * @param requester_xrce
     * @return false 
     */
    bool create_requester_by_bin(
            uint16_t,
            uint16_t,
            const dds::xrce::OBJK_Requester_Binary&) override { return false; };

    /**
     * @brief Not implemented.
     * 
     * @param replier_id 
     * @param participant_id 
     * @param ref 
     * @return false 
     */
    bool create_replier_by_ref(
            uint16_t,
            uint16_t,
            const std::string&) override { return false; };

    /**
     * @brief Not implemented.
     * 
     * @param replier_id 
     * @param participant_id 
     * @param xml
     * @return false 
     */
    bool create_replier_by_xml(
            uint16_t,
            uint16_t,
            const std::string&) override { return false; };

    /**
     * @brief Not implemented.
     * 
     * @param replier_id 
     * @param participant_id 
     * @param replier_xrce
     * @return false 
     */
    bool create_replier_by_bin(
            uint16_t,
            uint16_t,
            const dds::xrce::OBJK_Replier_Binary&) override { return false; };

    /**
     * @brief Removes a CedParticipant from the participants register.
     * @param participant_id    The CedParticipant identifier.
     * @return  true in case of the CedParticipant was found and removed, false in other case.
     */
    bool delete_participant(uint16_t participant_id) override;

    /**
     * @brief Removes a CedTopic from the topic register.
     * @param topic_id  The CedTopic identifier.
     * @return  true in case of the CedTopic was found and removed, false in other case.
     */
    bool delete_topic(uint16_t topic_id) override;

    /**
     * @brief Removes a CedPublisher from the publisher register.
     * @param publisher_id  The CedPublisher identifier.
     * @return  true in case of the CedPublisher was found and removed, false in other case.
     */
    bool delete_publisher(uint16_t publisher_id) override;

    /**
     * @brief Removes a CedSubscriber from the subscriber register.
     * @param subscriber_id The CedSubscriber identifier.
     * @return  true in case of the CedSubscriber was found and removed, false in other case.
     */
    bool delete_subscriber(uint16_t subscriber_id) override;

    /**
     * @brief Removes a CedDataWriter from the datawriters register.
     * @param datawriter_id The CedDataWriter identifier.
     * @return  true in case of the CedDataWriter was found and removed, false in other case.
     */
    bool delete_datawriter(uint16_t datawriter_id) override;

    /**
     * @brief Removes a CedDataReader from the datawriters register.
     * @param datareader_id The CedDataReader identifier.
     * @return  true in case of the CedDataReader was found and removed, false in other case.
     */
    bool delete_datareader(uint16_t datareader_id) override;

    /**
     * @brief Not implemented.
     * 
     * @param requester_id 
     * @return false 
     */
    bool delete_requester(uint16_t) override { return false; };

    /**
     * @brief Not implemented
     * 
     * @param replier_id 
     * @return false 
     */
    bool delete_replier(uint16_t) override { return false; };

    /**
     * @brief Writes data using the CedDataWriter identified by the datawriter_id parameter.
     * @param datawriter_id The CedDataWriter identifier.
     * @param data          The data to be written.
     * @return  true in case of successful writing and false in other case.
     */
    bool write_data(
            uint16_t datawriter_id,
            const std::vector<uint8_t>& data) override;

    /**
     * @brief Not implemented.
     */
    bool write_request(
            uint16_t,
            uint32_t,
            const std::vector<uint8_t>&) override { return false; }

    /**
     * @brief Not implemented.
     */
    bool write_reply(
            uint16_t,
            const std::vector<uint8_t>&) override { return false; }

    /**
     * @brief Read data using the CedDataReader identified by the datareader_id paramenter.
     *        This is a blocking function that will block at most "timeout" milleseconds.
     * @param datareader_id The CedDataReader's identifier.
     * @param data          The data read.
     * @param timeout       The timeout (milliseconds) of the reading.
     * @return  true in case of successful reading and false in other case.
     */
    bool read_data(
            uint16_t datareader_id,
            std::vector<uint8_t>& data,
            std::chrono::milliseconds timeout) override;

    /**
     * @brief Not implemented.
     */
    bool read_request(
            uint16_t,
            std::vector<uint8_t>&,
            std::chrono::milliseconds) override { return false; };

    /**
     * @brief Not implemented.
     */
    bool read_reply(
            uint16_t,
            uint32_t&,
            std::vector<uint8_t>&,
            std::chrono::milliseconds) override { return false; };

    /**
     * @brief Checks whether an existing CedParticipant, identified by the participant_id, matches with a new
     *        CedParticipant that would result from the creation of a new one using the domain_id and the reference
     *        representation.
     *        It is considered that the CedParticipants match if both have the same domain_id.
     * @param participant_id    The existing CedParticipant identifier.
     * @param domain_id         The Domain identifier of the new CedParticipant.
     * @param ref               The reference that represents the new CedParticipant (unused).
     * @return  true if both CedParticipant have the same Domain identifier, and false in other case.
     */
    bool matched_participant_from_ref(
            uint16_t participant_id,
            int16_t domain_id,
            const std::string& ref) const override;

    /**
     * @brief Checks whether an existing CedParticipant, identified by the participant_id, matches with a new
     *        CedParticipant that would result from the creation of a new one using the domain_id and the XML
     *        representation.
     *        It is considered that the CedParticipants match if both have the same domain_id.
     * @param participant_id    The existing CedParticipant identifier.
     * @param domain_id         The Domain identifier of the new CedParticipant.
     * @param xml               The XML that describes the new CedParticipant (unused).
     * @return  true if both CedParticipant have the same Domain identifier, and false in other case.
     */
    bool matched_participant_from_xml(
            uint16_t participant_id,
            int16_t domain_id,
            const std::string& xml) const override;


    /**
     * @brief Checks whether an existing CedParticipant, identified by the participant_id, matches with a new
     *        CedParticipant that would result from the creation of a new one using the domain_id and the binary reference.
     *        It is considered that the CedParticipants match if both have the same domain_id.
     * @param participant_id    The existing CedParticipant identifier.
     * @param domain_id         The Domain identifier of the new CedParticipant.
     * @param xml               XRCE Participant binary representation (unused).
     * @return  true if both CedParticipant have the same Domain identifier, and false in other case.
     */
    bool matched_participant_from_bin(
            uint16_t participant_id,
            int16_t domain_id,
            const dds::xrce::OBJK_DomainParticipant_Binary& participant_xrce) const override;

    /**
     * @brief Checks whether an existing CedTopic, identified by the topic_id, matches with a new CedTopic that would
     *        result from the creation of a new one using the reference representation.
     *        It is considered that the CedTopics match if both have the same topic name.
     * @param topic_id  The existing CedTopic identifier.
     * @param ref       The reference that represents the new CedTopic.
     *                  It defines the CedGlobalTopic name.
     * @return  true if both CedTopic have associated the same CedGlobalTopic, and false in other case.
     */
    bool matched_topic_from_ref(
            uint16_t topic_id,
            const std::string& ref) const override;

    /**
     * @brief Checks whether an existing CedTopic, identified by the topic_id, matches with a new CedTopic that would
     *        result from the creation of a new one using the XML representation.
     *        It is considered that the CedTopics match if both have the same topic name.
     * @param topic_id  The existing CedTopic identifier.
     * @param xml       The XML that describes the new CedTopic.
     *                  It defines the CedGlobalTopic name.
     * @return  true if both CedTopic have associated the same CedGlobalTopic, and false in other case.
     */
    bool matched_topic_from_xml(
            uint16_t topic_id,
            const std::string& xml) const override;

    /**
     * @brief Checks whether an existing CedTopic, identified by the topic_id, matches with a new CedTopic that would
     *        result from the creation of a new one using the binary reference.
     *        It is considered that the CedTopics match if both have the same topic name.
     * @param topic_id  	The existing CedTopic identifier.
	 * @param topic_xrce   	XRCE Topic binary representation.
     *                  	It defines the CedGlobalTopic name.
     * @return  true if both CedTopic have associated the same CedGlobalTopic, and false in other case.
     */
    bool matched_topic_from_bin(
            uint16_t topic_id,
            const dds::xrce::OBJK_Topic_Binary& topic_xrce) const override;

    /**
     * @brief Checks whether an existing CedDataWriter, identified by the datawriter_id, matches with the new
     *        CedDataWriter that would result from the creation of a new one using the reference representation.
     *        It is considered that the CedDataWriters match if both are associated to the same CedGlobalTopic.
     * @param datawriter_id The existing CedDataWriter identifier.
     * @param ref           The reference that represents the new CedDataWriter.
     *                      It defines the name of the associated CedGlobalTopic.
     * @return true if both CedDataWriter have associated the same CedGlobalTopic name
     */
    bool matched_datawriter_from_ref(
            uint16_t datawriter_id,
            const std::string& ref) const override;

    /**
     * @brief Checks whether an existing CedDataWriter, identified by the datawriter_id, matches with the new
     *        CedDataWriter that would result from the creation of a new one using the XML representation.
     *        It is considered that the CedDataWriters match if both are associated to the same CedGlobalTopic.
     * @param datawriter_id The existing CedDataWriter identifier.
     * @param ref           The reference that represents the new CedDataWriter.
     *                      It defines the name of the associated CedGlobalTopic.
     * @return true if both CedDataWriter have associated the same CedGlobalTopic name
     */
    bool matched_datawriter_from_xml(
            uint16_t datawriter_id,
            const std::string& xml) const override;

    /**
     * @brief Checks whether an existing CedDataWriter, identified by the datawriter_id, matches with the new
     *        CedDataWriter that would result from the creation of a new one using the binary reference.
     *        It is considered that the CedDataWriters match if both are associated to the same CedGlobalTopic.
     * @param datawriter_id 	The existing CedDataWriter identifier.
     * @param datawriter_xrce 	XRCE DataWriter binary representation.
     *                      	It defines the name of the associated CedGlobalTopic.
     * @return true if both CedDataWriter have associated the same CedGlobalTopic name
     */
    bool matched_datawriter_from_bin(
            uint16_t datawriter_id,
            const dds::xrce::OBJK_DataWriter_Binary& datawriter_xrce) const override;

    /**
     * @brief Checks whether an existing CedDataReader, identified by the datareader_id, matches with the new
     *        CedDataReader that would result from the creation of a new one using the reference representation.
     *        It is considered that the CedDataReaders match if both are associated to the same CedGlobalTopic.
     * @param datareader_id The existing CedDataReader identifier.
     * @param ref           The reference that represents the new CedDataReader.
     *                      It defines the name of the associated CedGlobalTopic.
     * @return true if both CedDataReader have associated the same CedGlobalTopic name
     */
    bool matched_datareader_from_ref(
            uint16_t datareader_id,
            const std::string& ref) const override;

    /**
     * @brief Checks whether an existing CedDataReader, identified by the datareader_id, matches with the new
     *        CedDataReader that would result from the creation of a new one using the XML representation.
     *        It is considered that the CedDataReaders match if both are associated to the same CedGlobalTopic.
     * @param datareader_id The existing CedDataReader identifier.
     * @param ref           The reference that represents the new CedDataReader.
     *                      It defines the name of the associated CedGlobalTopic.
     * @return true if both CedDataReader have associated the same CedGlobalTopic name
     */
    bool matched_datareader_from_xml(
            uint16_t datareader_id,
            const std::string& xml) const override;

     /**
     * @brief Checks whether an existing CedDataReader, identified by the datareader_id, matches with the new
     *        CedDataReader that would result from the creation of a new one using the binary reference.
     *        It is considered that the CedDataReaders match if both are associated to the same CedGlobalTopic.
     * @param datareader_id 	The existing CedDataReader identifier.
     * @param datawriter_xrce   XRCE DataReader binary representation.
     *                      	It defines the name of the associated CedGlobalTopic.
     * @return true if both CedDataReader have associated the same CedGlobalTopic name
     */
    bool matched_datareader_from_bin(
            uint16_t datareader_id ,
            const dds::xrce::OBJK_DataReader_Binary&  datareader_xrce) const override;

    /**
     * @brief Not implemented.
     * 
     * @param participant_id 
     * @param ref 
     * @return false 
     */
    bool matched_requester_from_ref(
            uint16_t,
            const std::string&) const override { return false; };

    /**
     * @brief Not implemented.
     * 
     * @param participant_id 
     * @param xml 
     * @return false 
     */
    bool matched_requester_from_xml(
            uint16_t,
            const std::string&) const override { return false; };
   
    /**
     * @brief Not implemented.
     * 
     * @param requester_id 
     * @param requester_xrce 
     * @return false 
     */
    bool matched_requester_from_bin(
            uint16_t,
            const dds::xrce::OBJK_Requester_Binary&) const override { return false; };

    /**
     * @brief Not implemented.
     * 
     * @param participant_id 
     * @param ref 
     * @return false 
     */
    bool matched_replier_from_ref(
            uint16_t,
            const std::string&) const override { return false; };

    /**
     * @brief Not implemented.
     * 
     * @param participant_id 
     * @param xml 
     * @return false 
     */
    bool matched_replier_from_xml(
            uint16_t,
            const std::string&) const override { return false; };
    
    /**
     * @brief Not implemented.
     * 
     * @param replier_id 
     * @param replier_xrce 
     * @return false 
     */
    bool matched_replier_from_bin(
            uint16_t,
            const dds::xrce::OBJK_Replier_Binary&) const override { return false; };

private:
    std::unordered_map<uint16_t, std::shared_ptr<CedParticipant>> participants_;
    std::unordered_map<uint16_t, std::shared_ptr<CedTopic>> topics_;
    std::unordered_map<uint16_t, std::shared_ptr<CedPublisher>> publishers_;
    std::unordered_map<uint16_t, std::shared_ptr<CedSubscriber>> subscribers_;
    std::unordered_map<uint16_t, std::shared_ptr<CedDataWriter>> datawriters_;
    std::unordered_map<uint16_t, std::shared_ptr<CedDataReader>> datareaders_;

    TopicSource topics_src_;
    WriteAccess write_access_;
    ReadAccess read_access_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_MIDDLEWARE_CED_CED_MIDDLEWARE_HPP_

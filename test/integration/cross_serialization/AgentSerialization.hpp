#ifndef IN_TEST_AGENT_CROSS_SERIALIZATION_HPP
#define IN_TEST_AGENT_CROSS_SERIALIZATION_HPP

#include <cstdint>
#include <vector>

struct AgentSerialization
{
    static std::vector<uint8_t> create_client_payload();
    static std::vector<uint8_t> create_payload();
    static std::vector<uint8_t> get_info_payload();
    static std::vector<uint8_t> delete_payload();
    static std::vector<uint8_t> status_agent_payload();
    static std::vector<uint8_t> status_payload();
    static std::vector<uint8_t> info_payload();
    static std::vector<uint8_t> read_data_payload();
    static std::vector<uint8_t> write_data_payload_data();
    static std::vector<uint8_t> write_data_payload_sample();
    static std::vector<uint8_t> write_data_payload_data_seq();
    static std::vector<uint8_t> write_data_payload_sample_seq();
    static std::vector<uint8_t> write_data_payload_packed_samples();
    static std::vector<uint8_t> data_payload_data();
    static std::vector<uint8_t> data_payload_sample();
    static std::vector<uint8_t> data_payload_data_seq();
    static std::vector<uint8_t> data_payload_sample_seq();
    static std::vector<uint8_t> data_payload_packed_samples();
    static std::vector<uint8_t> acknack_payload();
    static std::vector<uint8_t> heartbeat_payload();
};

#endif //IN_TEST_AGENT_CROSS_SERIALIZATION_HPP

// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#include <MessageHeader.h>
#include <SubMessageHeader.h>
#include <Serializer.h>
#include <Payloads.h>
#include <XRCEFactory.h>
#include <XRCEParser.h>
#include <DDSXRCETypes.h>

#include <ddsxrce_transport.h>

#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <functional>
#include <unistd.h>

using namespace eprosima::micrortps;

void print(const MessageHeader& header)
{
    std::cout << "=MESSAGE HEADER=" << std::endl;
    std::cout << "  client key:  " << std::hex << header.client_key()[0] << header.client_key()[1] << header.client_key()[2] << header.client_key()[3] << std::endl;
    std::cout << "  session id:  " << std::hex << +header.session_id() << std::endl;
    std::cout << "  stream id:   " << std::hex << +header.stream_id() << std::endl;
    std::cout << "  sequence nr: " << std::hex << header.sequence_nr() << std::endl;
    std::cout << "===============" << std::endl;
}

void print(const SubmessageHeader& subheader)
{
    std::cout << "=SUBMESSAGE HEADER=" << std::endl;
    std::cout << "  submessage id:     " << std::hex << static_cast<int>(subheader.submessage_id()) << std::endl;
    std::cout << "  flags:             " << std::hex << static_cast<int>(subheader.flags()) << std::endl;
    std::cout << "  submessage length: " << std::hex << static_cast<int>(subheader.submessage_length()) << std::endl;
    std::cout << "=====================" << std::endl;
}

void onMessage()
{

}

size_t fill_buffer(octet* out_buffer, size_t buffer_len)
{
    const ClientKey client_key = {{0xF1, 0xF2, 0xF3, 0xF4}};
    const uint8_t session_id = 0x01;
    const uint8_t stream_id = 0x04;
    const uint16_t sequence_nr = 0x0200;

    XRCEFactory newMessage{reinterpret_cast<char*>(out_buffer), (uint32_t)buffer_len};
    MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);
    message_header.sequence_nr(sequence_nr);
    newMessage.header(message_header);

    // OBJK_DATAWRITER_Status data_writer;
    // data_writer.stream_seq_num(0x7FFF);
    // data_writer.sample_seq_num(0x1234567890ABCDEF);
    // StatusVariant variant;
    // variant.data_writer(data_writer);
    // ResultStatus result;
    // result.request_id({ 0x10, 0x09, 0x08, 0x07 });
    // result.status(STATUS_OK);
    // result.implementation_status(STATUS_ERR_INCOMPATIBLE);
    // Status status;
    // status.status(variant);
    // status.result(result);
    // status.object_id({ 10,20,30 });

    // RESOURCE_STATUS_PAYLOAD resource_status;
    // resource_status.request_id({ 0x00, 0x01, 0x02, 0x03 });
    // resource_status.request_status(status);

    // SubmessageHeader submessage_header;
    // submessage_header.submessage_id(STATUS);
    // submessage_header.flags(0x07);
    // submessage_header.submessage_length(static_cast<uint16_t>(resource_status.getCdrSerializedSize(resource_status)));

    // newMessage.status(resource_status);

    // RT_Data data_reader;
    // SampleData sample_data;
    // sample_data.serialized_data({0x0A, 0x0B, 0x0C, 0x0D});
    // data_reader.data(sample_data);
    
    // DATA_PAYLOAD data;
    // data.data_reader(data_reader);
    // data.resource_id({ 0xF0,0xF1,0xF2 });
    // data.request_id({ 0x00,0x11,0x22, 0x33 });

    // newMessage.data(data);

    return newMessage.get_total_size();
}

class PoC_Listener : public XRCEListener
{
public:
    PoC_Listener() {};
    ~PoC_Listener() {};

    virtual void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const CREATE_Payload& create_payload)
    {

    }

    virtual void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const DELETE_RESOURCE_Payload& delete_payload)
    {

    }

    virtual void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const WRITE_DATA_Payload&  write_payload)
    {

    }

    virtual void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const READ_DATA_Payload&   read_payload)
    {

    }
};

int main(int args, char** argv)
{
    // Mesage creation
    octet out_buffer[1024] = {};
    octet in_buffer[1024] = {};
    size_t buffer_len = 1024;
    size_t message_size = fill_buffer(out_buffer, buffer_len);

    // Listener for parser
    PoC_Listener my_xrce_listener;

    // Init transport

    int ret = 0;
    locator_id_t loc_id = add_serial_locator("/dev/ttyACM0");

    int loops = 1000;
    while (loops--)
    {
        if (0 < (ret = send_data(out_buffer, message_size, loc_id)))
        {
            printf("SEND: %d bytes\n", ret);
        }
        else
        {
            printf("SEND ERROR: %d\n", ret);
        }

        usleep(2000000);

        if (0 < (ret = receive_data(in_buffer, buffer_len, loc_id)))
        {
            printf("RECV: %d bytes\n", ret);
            XRCEParser myParser{reinterpret_cast<char*>(in_buffer), ret, &my_xrce_listener};
            myParser.parse();
        }
        else
        {
            printf("RECV ERROR: %d\n", ret);
        }

        usleep(2000000);
    }

    printf("exiting...\n");
    return 0;
}

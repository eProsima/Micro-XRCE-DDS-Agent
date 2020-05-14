// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef UXR_AGENT_DATAREADER_DATAREADER_HPP_
#define UXR_AGENT_DATAREADER_DATAREADER_HPP_

#include <uxr/agent/object/XRCEObject.hpp>
#include <uxr/agent/reader/Reader.hpp>

namespace eprosima {
namespace uxr {

class ProxyClient;
class Subscriber;
class Topic;

/**
 * @brief The DataReader class
 */
class DataReader : public XRCEObject
{
public:
    static std::unique_ptr<DataReader> create(
        const dds::xrce::ObjectId& object_id,
        uint16_t subscriber_id,
        const std::shared_ptr<ProxyClient>& proxy_client,
        const dds::xrce::DATAREADER_Representation& representation);

    virtual ~DataReader() noexcept override;

    DataReader(DataReader&&) = delete;
    DataReader(const DataReader&) = delete;
    DataReader& operator=(DataReader&&) = delete;
    DataReader& operator=(const DataReader&) = delete;

    bool matched(
            const dds::xrce::ObjectVariant& new_object_rep) const final;

    bool read(
        const dds::xrce::READ_DATA_Payload& read_data,
        Reader<bool>::WriteFn write_fn,
        WriteFnArgs& cb_args);

private:
    DataReader(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<ProxyClient>& proxy_client);

    bool read_fn(
        bool,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout);

private:
    std::shared_ptr<ProxyClient> proxy_client_;
    Reader<bool> reader_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_DATAREADER_DATAREADER_HPP_

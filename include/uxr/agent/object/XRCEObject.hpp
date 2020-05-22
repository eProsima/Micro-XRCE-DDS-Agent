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

#ifndef XRC_OBJECT_HPP_
#define XRC_OBJECT_HPP_

#include <uxr/agent/types/XRCETypes.hpp>
#include <uxr/agent/types/MessageHeader.hpp>
#include <uxr/agent/types/SubMessageHeader.hpp>
#include <uxr/agent/middleware/Middleware.hpp>
#include <uxr/agent/utils/Conversion.hpp>

#include <unordered_map>
#include <memory>

namespace eprosima {
namespace uxr {

class XRCEObject
{
private:
    struct ObjectIdHash
    {
        uint16_t operator()(const dds::xrce::ObjectId& object_id) const
        {
            return conversion::objectid_to_raw(object_id);
        }
    };

public:
    typedef std::unordered_map<dds::xrce::ObjectId, std::shared_ptr<XRCEObject>, ObjectIdHash> ObjectContainer;

    explicit XRCEObject(const dds::xrce::ObjectId& object_id)
        : id_(object_id)
    {}

    virtual ~XRCEObject() = 0;

    XRCEObject(XRCEObject &&) = delete;
    XRCEObject(const XRCEObject &) = delete;
    XRCEObject& operator=(XRCEObject &&) = delete;
    XRCEObject& operator=(const XRCEObject &) = delete;

    dds::xrce::ObjectId get_id() const;
    uint16_t get_raw_id() const { return conversion::objectid_to_raw(id_); }
    virtual bool matched(const dds::xrce::ObjectVariant& new_object_rep) const = 0;

private:
    dds::xrce::ObjectId id_;
};

} // namespace uxr
} // namespace eprosima

#endif // XRC_OBJECT_HPP_

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

#ifndef _XRCE_OBJECT_H_
#define _XRCE_OBJECT_H_

#include <micrortps/agent/types/XRCETypes.hpp>
#include <micrortps/agent/types/MessageHeader.hpp>
#include <micrortps/agent/types/SubMessageHeader.hpp>
#include <unordered_map>
#include <memory>

namespace eprosima {
namespace micrortps {

class XRCEObject;

struct ObjectIdHash
{
    uint16_t operator()(const dds::xrce::ObjectId& object_id) const
    {
        return object_id.at(1) + (object_id.at(0) << 8);
    }
};

typedef std::unordered_map<dds::xrce::ObjectId, std::shared_ptr<XRCEObject>, ObjectIdHash> ObjectContainer;

class XRCEObject
{
  public:
    explicit XRCEObject(const dds::xrce::ObjectId& object_id) : id_{object_id} {}
    XRCEObject(XRCEObject &&) = default;
    XRCEObject(const XRCEObject &) = default;
    XRCEObject& operator=(XRCEObject &&) = default;
    XRCEObject& operator=(const XRCEObject &) = default;
    virtual ~XRCEObject() = 0;

    dds::xrce::ObjectId get_id() const;
    virtual void release(ObjectContainer& root_objects) = 0;

  private:
    dds::xrce::ObjectId id_;
};

} // namespace micrortps
} // namespace eprosima

#endif //_XRCE_OBJECT_H_

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

#ifndef _MICRORTPS_AGENT_DLL_HPP_
#define _MICRORTPS_AGENT_DLL_HPP_

#if defined(_WIN32)
#if define(micrortps_agent_SHARED)
#if define(micrortps_agent_EXPORT)
#define micrortps_agent_DllAPI __declspec( dllexport )
#else
#define micrortps_agent_DllAPI __declspec( dllimport )
#endif // micrortps_agent_EXPORT
#else
#define micrortps_agent_DllAPI
#endif // micrortps_agent_SHARED
#else
#define micrortps_agent_DllAPI
#endif //_WIN32

#endif //_MICRORTPS_AGENT_DLL_HPP_

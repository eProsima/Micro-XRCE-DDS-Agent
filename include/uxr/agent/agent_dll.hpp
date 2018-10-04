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

#ifndef _UXR_AGENT_DLL_HPP_
#define _UXR_AGENT_DLL_HPP_

#if defined(_WIN32)
#if defined(microxrcedds_agent_SHARED)
#if defined(microxrcedds_agent_EXPORTS)
#define microxrcedds_agent_DllAPI __declspec( dllexport )
#else
#define microxrcedds_agent_DllAPI __declspec( dllimport )
#endif // microxrcedds_agent_EXPORTS
#else
#define microxrcedds_agent_DllAPI
#endif // microxrcedds_agent_SHARED
#else
#define microxrcedds_agent_DllAPI
#endif //_WIN32

#endif //_UXR_AGENT_DLL_HPP_

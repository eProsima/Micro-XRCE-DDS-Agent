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

#ifndef UXR_AGENT_DLL_HPP_
#define UXR_AGENT_DLL_HPP_

#if defined(_WIN32)
#if defined(microxrcedds_agent_SHARED)
#if defined(microxrcedds_agent_EXPORTS)
#define UXR_AGENT_LIB __declspec( dllexport )
#else
#define UXR_AGENT_LIB __declspec( dllimport )
#endif // microxrcedds_agent_EXPORTS
#else
#define UXR_AGENT_LIB
#endif // microxrcedds_agent_SHARED
#else
#define UXR_AGENT_LIB
#endif // WIN32

#endif // UXR_AGENT_DLL_HPP_

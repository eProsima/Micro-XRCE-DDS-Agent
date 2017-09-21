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
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <functional>
#include <unistd.h>

#include <agent/datareader/DataReader.h>
#include <agent/client/ProxyClient.h>

using namespace eprosima::micrortps;

int main(int args, char** argv)
{

    ProxyClient client;
    DataReader reader(&client);
    READ_DATA_PAYLOAD read_config;
    read_config.read_mode(READM_DATA);
    read_config.max_samples(1);
    printf("read\n");
    reader.read(read_config);

    while(1)sleep(1);

    printf("exiting...\n");
    return 0;
}

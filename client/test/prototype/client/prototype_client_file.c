#include <micrortps/client/client.h>

#include "prototype/shape_topic.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

typedef struct UserData
{
    uint32_t recieved_topics;

} UserData;

typedef struct SharedFile
{
    char out_file_name[256];
    char in_file_name[256];

} SharedFile;


void compute_command(char* file_name, UserData* user, Topic* topic);

//callbacks for send and recieved from io.
void send_data_file(uint8_t* buffer, uint32_t length, void* data);
uint32_t recieved_data_file(uint8_t* buffer, uint32_t size, void* data);

//callbacks for listening topics
void on_listener_shape_topic(const void* topic_data, void* callback_object);

char command_file[] = "command.io";


int main(int args, char** argv)
{
    // For internal test only
    SharedFile shared_file = {"client_to_agent.bin", "agent_to_client.bin"};
    fclose(fopen(shared_file.in_file_name, "wb"));
    fclose(fopen(command_file, "w"));

    // User data
    UserData user = {0};

    init_client(1024, send_data_file, recieved_data_file, &shared_file, &user);

    Topic topic =
    {
        "SQUARE",
        serialize_shape_topic,
        deserialize_shape_topic,
        size_of_shape_topic
    };

    uint32_t dt = 2;
    char time_string[80];
    while(user.recieved_topics < 10)
    {
        time_t t = time(NULL);
        struct tm* timeinfo = localtime(&t);
        strftime(time_string, 80, "%T", timeinfo);
        printf("============================ CLIENT ========================> %s\n", time_string);

        // user code here
        compute_command(command_file, &user, &topic);

        // this function does all comunnications
        update_communication();

        usleep(1000000 * dt);
    }

    return 0;
}

void on_listener_shape_topic(const void* topic_data, void* callback_object)
{
    const ShapeTopic* shape_topic = (const ShapeTopic*)topic_data;

    print_shape_topic(shape_topic);

    UserData* user = (UserData*)callback_object;
    user->recieved_topics++;
}


// -------------------------------------------------------------------------------
//                               IN OUT FROM FILE
// -------------------------------------------------------------------------------
void send_data_file(uint8_t* buffer, uint32_t length, void* callback_object)
{
    SharedFile* shared_file = (SharedFile*)callback_object;

    FILE* out_file = fopen(shared_file->out_file_name, "wb");
    fwrite(buffer, 1, length, out_file);
    fclose(out_file);
}

uint32_t recieved_data_file(uint8_t* buffer, uint32_t size, void* callback_object)
{
    SharedFile* shared_file = (SharedFile*)callback_object;

    FILE* in_file = fopen(shared_file->in_file_name, "rb");
    fseek(in_file, 0L, SEEK_END);
    uint32_t in_file_size = ftell(in_file);
    fseek(in_file, 0L, SEEK_SET);

    if(in_file_size > size)
        in_file_size = size;

    fread(buffer, 1, in_file_size, in_file);
    fclose(in_file);
    fclose(fopen(shared_file->in_file_name,"wb"));

    return in_file_size;
}


/*
COMMANDS:
    > create_pub
    > create_sub
    > config_read sub_id
    > send pub_id color x y size
    > delete_pub pub_id
    > delete_sub sub_id
*/

void compute_command(char* file_name, UserData* user, Topic* topic)
{
    FILE* commands_file = fopen(file_name, "r");
    char command[256];
    uint32_t id;
    char color[256];
    uint32_t x;
    uint32_t y;
    uint32_t size;
    int valid_command = 0;
    int command_size = fscanf(commands_file, "%s %u %s %u %u %u", command, &id, color, &x, &y, &size);

    if(command_size == -1)
    {
        valid_command = 1;
    }
    else if(command_size == 1)
    {
        if(strcmp(command, "create_pub") == 0)
        {
            create_publisher(topic);
            valid_command = 1;
        }
        else if(strcmp(command, "create_sub") == 0)
        {
            Subscriber* subscriber = create_subscriber(topic);
            add_listener_topic(subscriber, on_listener_shape_topic);
            valid_command = 1;
        }
    }
    else if(command_size == 2)
    {
        if(strcmp(command, "delete") == 0)
        {
            void* object;
            int kind = get_xrce_object(id, &object);
            if(kind == OBJECT_PUBLISHER)
            {
                delete_publisher(object);
                valid_command = 1;
            }
            else if(kind == OBJECT_SUBSCRIBER)
            {
                delete_subscriber(object);
                valid_command = 1;
            }
        }
    }
    else if(command_size == 3)
    {
        if(strcmp(command, "read_data") == 0)
        {
            void* object;
            if(get_xrce_object(id, &object) == OBJECT_SUBSCRIBER)
            {
                read_data(object, atoi(color));
                valid_command = 1;
            }
        }
    }
    else if(command_size == 6)
    {
        if(strcmp(command, "send") == 0)
        {
            void* object;
            if(get_xrce_object(id, &object) == OBJECT_PUBLISHER)
            {
                ShapeTopic shape_topic = {strlen(color) + 1, color, x, y, size};
                if(send_topic(object, &shape_topic))
                    print_shape_topic(&shape_topic);
                valid_command = 1;
            }
        }
    }

    if(!valid_command)
        printf("Unknown command or incorrect input \n");

    fclose(commands_file);
    fclose(fopen(file_name, "w"));

    //TODO
    //delete_publisher(publisher);
    //delete_subscriber(subscriber);
}
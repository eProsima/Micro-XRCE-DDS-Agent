/*
typedef struct SerialPort
{
    locator_t locator;
    channel_id_t channel_id;

} SerialPort;
*/

// Initialization
/*
SerialPort port;
port.locator = {LOC_SERIAL, "/dev/ttyACM0"};
oort.channel_id = add_locator(&loc);
*/

// -------------------------------------------------------------------------------
                            // IN OUT FROM SERIAL PORT
// -------------------------------------------------------------------------------
void send_data_io(uint8_t* buffer, uint32_t length, void* data)
{
    SerialPort* port = (SerialPort*)data;
    uint32_t bytes = send(buffer, length, port->locate.kind, port->channel_id);

    if(bytes > 0)
        printf("SEND: %d bytes\n", bytes)
    else
        printf("SEND ERROR: %d\n", bytes);
}

uint32_t recieved_data_io(uint8_t* buffer, uint32_t size, void* data)
{
    SerialPort* port = (SerialPort*)data;
    uint32_t bytes = receive(buffer, size, port->locate.kind, port->channel_id);

    if(bytes > 0)
        printf("RECV: %d bytes\n", bytes)
    else
        printf("RECV ERROR: %d\n", bytes);

    return bytes;
}
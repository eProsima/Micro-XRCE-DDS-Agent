#ifndef _DDS_XRCE_XRCE_SPEC_H_
#define _DDS_XRCE_XRCE_SPEC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

// --------------------------------------------------------------------
//                               DEFINES
// --------------------------------------------------------------------
#define SUBMESSAGE_CREATE        0x01
#define SUBMESSAGE_DELETE        0x03
#define SUBMESSAGE_WRITE_DATA    0x06
#define SUBMESSAGE_READ_DATA     0x07
#define SUBMESSAGE_DATA          0x08

#define OBJECT_ID_INVALID    0xFFFFFF
#define OBJECT_ID_CLIENT     0xFFFFF0
#define OBJECT_ID_SESSION    0xFFFFF1

#define OBJECT_KIND_DATAWRITER   0x03
#define OBJECT_KIND_DATAREADER   0x07
#define OBJECT_KIND_SUBSCRIBER   0x08
#define OBJECT_KIND_PUBLISHER    0x09

#define READ_MODE_DATA              0x00
#define READ_MODE_DATA_SEQ          0x01
#define READ_MODE_SAMPLE            0x02
#define READ_MODE_SAMPLE_SEQ        0x03
#define READ_MODE_PACKED_SAMPLE_SEQ 0x04

typedef uint32_t uint_least24_t;
typedef uint8_t boolean_t;

// --------------------------------------------------------------------
//                            OBJECT VARIANT
// --------------------------------------------------------------------
typedef struct DataWriterSpec
{
    uint_least24_t participant_id;
    uint_least24_t publisher_id;

} DataWriterSpec;

typedef struct DataReaderSpec
{
    uint_least24_t participant_id;
    uint_least24_t subscriber_id;

} DataReaderSpec;

typedef struct PublisherSpec
{
    uint_least24_t participant_id;

} PublisherSpec;

typedef struct SubscriberSpec
{
    uint_least24_t participant_id;

} SubscriberSpec;

typedef union ObjectVariantSpec
{
    DataWriterSpec data_writer;
    DataReaderSpec data_reader;
    PublisherSpec publisher;
    SubscriberSpec subscriber;

} ObjectVariantSpec;

typedef struct ObjectKindSpec
{
    uint32_t string_size;
    char* string;
    uint8_t kind;
    ObjectVariantSpec variant;

} ObjectKindSpec;


// --------------------------------------------------------------------
//                               SAMPLES
// --------------------------------------------------------------------
typedef struct SampleInfoSpec
{
    uint8_t state;
    uint64_t sequence_number;
    uint32_t session_time_offset;
} SampleInfoSpec;

typedef struct SampleDataSpec
{
    uint32_t serialized_data_size;
    uint8_t* serialized_data;

} SampleDataSpec;

typedef struct SampleSpec
{
    SampleInfoSpec info;
    SampleDataSpec data;

} SampleSpec;

typedef union SampleKindSpec
{
    SampleDataSpec data;
    SampleSpec sample;

} SampleKindSpec;

typedef struct DataModeSpec
{
    uint8_t read_mode;
    SampleKindSpec sample_kind;

} DataModeSpec;

// --------------------------------------------------------------------
//                               PAYLOADS
// --------------------------------------------------------------------
typedef struct CreatePayloadSpec
{
    uint32_t request_id;
    uint_least24_t object_id;
    ObjectKindSpec object;

} CreatePayloadSpec;

typedef struct DeletePayloadSpec
{
    uint32_t request_id;
    uint_least24_t object_id;

} DeletePayloadSpec;

typedef struct WriteDataPayloadSpec
{
    uint32_t request_id;
    uint_least24_t object_id;
    DataModeSpec data_writer;

} WriteDataPayloadSpec;

typedef struct ReadDataPayloadSpec
{
    uint32_t request_id;
    uint16_t max_messages;
    uint32_t max_elapsed_time;
    uint32_t max_rate;
    uint32_t expression_size;
    char* content_filter_expression;
    uint16_t max_samples;
    boolean_t include_sample_info;

} ReadDataPayloadSpec;

typedef struct DataPayloadSpec
{
    uint32_t request_id;
    uint_least24_t object_id;
    DataModeSpec data_reader;

} DataPayloadSpec;

typedef union PayloadSpec
{
    CreatePayloadSpec create_resource;
    DeletePayloadSpec delete_resource;
    WriteDataPayloadSpec write_data;
    ReadDataPayloadSpec read_data;
    DataPayloadSpec data;

} PayloadSpec;

// --------------------------------------------------------------------
//                               MESSAGE
// --------------------------------------------------------------------
typedef struct MessageHeaderSpec
{
    uint32_t client_key;
    uint8_t session_id;
    uint8_t stream_id;
    uint16_t sequence_nr;

} MessageHeaderSpec;

typedef struct SubmessageHeaderSpec
{
    uint8_t id;
    uint8_t flags;
    uint16_t length;

} SubmessageHeaderSpec;


#ifdef __cplusplus
}
#endif

#endif //_DDS_XRCE_XRCE_SPEC_H_
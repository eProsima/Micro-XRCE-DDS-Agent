//typedef octet ClientKey[4];
//typedef octet ObjectKind;
//
//const ObjectKind OBJK_INVALID     = 0x0;
//// const ObjectKind OBJK_SESSION  = 0x1;
//const ObjectKind OBJK_PARTICIPANT = 0x1; // Matches RTPS EntityKind
//const ObjectKind OBJK_TOPIC       = 0x4;
//const ObjectKind OBJK_DATAWRITER  = 0x3; // Matches RTPS EntityKind
//const ObjectKind OBJK_DATAREADER  = 0x7; // Matches RTPS EntityKind
//const ObjectKind OBJK_SUBSCRIBER  = 0x8;
//const ObjectKind OBJK_PUBLISHER   = 0x9;
//const ObjectKind OBJK_TYPE        = 0x10;
//const ObjectKind OBJK_QOSPROFILE  = 0x11;
//const ObjectKind OBJK_APPLICATION = 0x20;
//const ObjectKind OBJK_CLIENT      = 0x21;
//
//typedef octet ObjectId[3];
//
//#define  OBJECTID_INVALID {0xFF,0xFF,0xFF}
//#define  OBJECTID_CLIENT  {0xFF,0xFF,0xF0}
//#define  OBJECTID_SESSION {0xFF,0xFF,0xF1}
//#define  OBJECTID_CLIENT  {0xFF,0xFF,0xF2}
//
//#define XRCE_COOKIE {0x58, 0x52, 0x43, 0x45}
//#define XRCE_VESION_MAJOR 0x01
//#define XRCE_VESION_MINOR 0x00
//#define XRCE_VERSION  {XRCE_VESION_MAJOR, XRCE_VESION_MINOR}
//
//struct Time_t
//{
//    long           seconds;
//    unsigned long  nanoseconds;
//};
//
//typedef octet SessionId;
//const octet SESSIONID_INVALID = 0;
//struct OBJK_CLIENT_Representation
//{
//    octet[4]   xrce_cookie;  // XRCE_COOKIE
//    octet[2]   xrce_version;
//    octet[2]   xrce_vendor_id;
//    Time_t     client_timestamp;
//    SessionId  session_id;
//};
//
//struct OBJK_CommonString_Representation
//{
//    string as_string;
//};
//struct OBJK_QOSPROFILE_Representation : OBJK_CommonString_Representation
//{
//};
//struct OBJK_APPLICATION_Representation : OBJK_CommonString_Representation
//{
//};
//struct OBJK_PARTICIPANT_Representation : OBJK_CommonString_Representation
//{
//};
//struct OBJK_TYPE_Representation : OBJK_CommonString_Representation
//{
//    ObjectId participant_id;
//    string registered_type_name;
//};
//struct OBJK_TOPIC_Representation : OBJK_CommonString_Representation
//{
//    ObjectId participant_id;
//};
//
//struct OBJK_PUBLISHER_Representation : OBJK_CommonString_Representation
//{
//    ObjectId participant_id;
//};
//struct OBJK_SUBSCRIBER_Representation : OBJK_CommonString_Representation
//{
//    ObjectId participant_id;
//};
//struct OBJK_DATAWRITER_Representation : OBJK_CommonString_Representation
//{
//    ObjectId participant_id;
//    ObjectId publisher_id;
//};
//struct OBJK_DATAREADER_Representation : OBJK_CommonString_Representation
//{
//    ObjectId participant_id;
//    ObjectId subscriber_id;
//};
//
//union ObjectVariant switch(ObjectKind)
//{
//    // case OBJK_INVALID : indicates default or selected by Agent. No data.
//    case OBJK_CLIENT :
//        OBJK_CLIENT_Representation client;
//    case OBJK_APP :
//        OBJK_APPLICATION_Representation application;
//    case OBJK_PARTICIPANT :
//        OBJK_PARTICIPANT_Representation participant;
//    case OBJK_QOSPROFILE :
//        OBJK_QOSPROFILE_Representation qos_profile;
//    case OBJK_TYPE :
//        OBJK_TYPE_Representation type;
//    case OBJK_TOPIC :
//        OBJK_TOPIC_Representation topic;
//    case OBJK_PUBLISHER :
//        OBJK_PUB_Representation publisher;
//    case OBJK_SUBSCRIBER :
//        OBJK_SUB_Representation subscriber;
//    case OBJK_DATAWRITER :
//        OBJK_DW_Representation data_writer;
//    case OBJK_DATAREADER :
//        OBJK_DR_Representation data_reader;
//};
//
//struct CreationMode
//{
//   boolean reuse;
//   boolean replace;
//};
//
//
//typedef octet RequestId[2];
//
//struct ResultStatus
//{
//    RequestId     request_id;
//    octet         status;
//    octet         implementation_status;
//};
//
//const octet STATUS_OK                    = 0x00;
//const octet STATUS_OK_MATCHED            = 0x01;
//const octet STATUS_ERR_DDS_ERROR         = 0x80;
//const octet STATUS_ERR_MISMATCH          = 0x81;
//const octet STATUS_ERR_ALREADY_EXISTS    = 0x82;
//const octet STATUS_ERR_DENIED            = 0x83;
//const octet STATUS_ERR_UNKNOWN_REFERENCE = 0x84;
//const octet STATUS_ERR_INVALID_DATA      = 0x85;
//const octet STATUS_ERR_INCOMPATIBLE      = 0x86;
//const octet STATUS_ERR_RESOURCES         = 0x87;
//
//const octet STATUS_LAST_OP_NONE      = 0;
//const octet STATUS_LAST_OP_CREATE    = 1;
//const octet STATUS_LAST_OP_UPDATE    = 2;
//const octet STATUS_LAST_OP_DELETE    = 3;
//const octet STATUS_LAST_OP_LOOKUP    = 4;
//const octet STATUS_LAST_OP_READ      = 5;
//const octet STATUS_LAST_OP_WRITE     = 6;
//
//struct OBJK_DATAREADER_Status
//{
//    short highest_acked_num;
//};
//
//struct OBJK_DATAWRITER_Status
//{
//    short stream_seq_num;
//    unsigned long long sample_seq_num;
//};
//
//union StatusVariant (ObjectKind)
//{
//    case OBJK_DATAWRITER :
//        OBJK_DW_Status data_writer;
//    case OBJK_DATAREADER :
//        OBJK_DR_Status data_reader;
//};
//
//struct Status
//{
//    ResultStatus  result;
//    ObjectId      object_id;
//    StatusVariant status;
//};
//
//struct Info
//{
//    ResultStatus  result;
//    ObjectId      object_id;
//    ObjectVariant info;
//};
//
//
//
//
//
//
//// ****** CREATE ******
//struct CREATE_PAYLOAD
//{
//    RequestId     request_id;
//    ObjectId      object_id;
//    ObjectVariant object_representation;
//};
//
//// ****** DELETE_RESOURCE ******
//struct RESOURCE_STATUS_PAYLOAD
//{
//    RequestId     request_id;
//    Status        request_status;
//};
//
//// ****** GET_INFO ******
//struct GET_INFO_PAYLOAD
//{
//    RequestId     request_id;
//};
//
//// ****** INFO ******
//struct INFO_PAYLOAD
//{
//    RequestId     request_id;
//    Info          resource_info;
//};
//
//// ****** READ_RESOURCE ******
//struct DataReaderReadeSpec {
//    short    max_samples;
//    boolean  include_info;
//    string   content_filter;
//};
//
//struct READ_RESOURCE_PAYLOAD {
//  RequestId            request_id;
//  DataReaderReadeSpec  datareader_spec;
//};
//
//// ****** READ_DATA ******
//typedef octet ReadMode;
//const ReadMode READM_DATA              = 0x00;
//const ReadMode READM_DATA_SEQ          = 0x01;
//const ReadMode READM_SAMPLE            = 0x02;
//const ReadMode READM_SAMPLE_SEQ        = 0x03;
//const ReadMode READM_PACKED_SAMPLE_SEQ = 0x04;
//
//struct CONFIG_READ_DATA_PAYLOAD
//{
//    RequestId               request_id;
//    unsigned short          max_messages;     // Maximum numbers of samples
//    long                    max_elapsed_time; // In milliseconds
//    long                    max_rate;         // Bytes per second
//    string                  content_filter_expression;
//    unsigned short          max_samples;      // Maximum numbers of samples
//    boolean                 include_sample_info;
//};
//
//
//// ****** DATA ******
//struct SampleInfo
//{
//    octet               state;  // Combines SampleState, InstanceState, ViewState
//    unsigned long long  sequence_number;
//    unsigned long       session_time_offset; // milliseconds up to 53 days
//};
//
//struct SampleInfoDelta
//{
//    octet           state;  // Combines SampleState, InstanceState, ViewState
//    octet           info_seq_number_delta;
//    unsigned short  info_timestamp_delta; // milliseconds
//};
//
//struct SampleData
//{
//    sequence<octet> serialized_data;
//};
//
//typedef sequence<SampleData> SampleDataSeq;
//struct Sample
//{
//    SampleInfo info;
//    SampleData data;
//};
//
//typedef sequence<Sample> SampleSeq;
//struct SampleDelta
//{
//    SampleInfoDelta   info_delta;
//    SampleData        data;
//};
//
//struct SamplePacked
//{
//    SampleInfo             info_base;
//    sequence<SampleDelta>  sample;
//};
//
//typedef sequence<SamplePacked> SamplePackedSeq;
//union RT_Data switch(ReadMode)
//{
//    case READM_DATA : // Read data with no sample info
//        SampleData        data;
//    case READM_DATA_SEQ : // Read data with no sample info
//        SampleDataSeq     data_seq;
//    case READM_SAMPLE :   // Read data with the SampleInfo
//        Sample            sample;
//    case READM_SAMPLE_SEQ :   // Read data with the SampleInfo
//        SampleSeq         sample_seq;
//    case READM_PACKED_SAMPLE_SEQ : // Read data with compact sample info
//        SamplePackedSeq   sample_packed_seq;
//};
//
//struct DATA_PAYLOAD
//{
//    RequestId     request_id;
//    ObjectId      resource_id;
//    RT_Data       data_reader;
//};
//
//// ****** WRITE_DATA ******
//struct WRITE_DATA_PAYLOAD
//{
//    RequestId  request_id;
//    ObjectId   object_id;
//    RT_Data    data_writer;
//};
//
//// ****** ACKNACK ******
//struct ACKNACK_PAYLOAD
//{
//    short firstUnackedSeqNr;
//    octet[2] nackBitmap;
//};
//
//// ****** HEARTBEAT ******
//struct HEARTBEAT_PAYLOAD
//{
//    short first_unacked_seq_nr;
//    short last_unacked_seq_nr;
//};
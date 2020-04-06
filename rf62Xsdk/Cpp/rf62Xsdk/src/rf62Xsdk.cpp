#include "rf62Xsdk.h"
#include "rf62Xtypes.h"
#include "rf62Xcore.h"
#include <memory>
#include <iostream>

extern "C"{
#include <rf62X_sdk.h>
}


#ifdef _WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif


extern BOOL EnumAdapterAddresses();
extern void FreeAdapterAddresses();
extern int GetAdaptersCount();
extern const char* GetAdapterAddress(int index);
/* windows sockets tweaks */
extern BOOL WinSockInit();


int SDK::SCANNERS::RF62X::sdk_version()
{
    /*
     * Get rf627 core version
     */
    return SDK::CORES::RF62X::version();
}

bool SDK::SCANNERS::RF62X::sdk_init()
{
    SDK::CORES::RF62X::init();
    return true;
}

namespace SDK {
namespace SCANNERS {
namespace RF62X {



std::vector<rf627old*> rf627old::search(PROTOCOLS protocol)
{
    switch (protocol) {
    case PROTOCOLS::SERVICE:
    {
        /*
         * Create value for scanners vector's type
         */
        vector_t* scanners = (vector_t*)calloc(1, sizeof (vector_t));
        /*
         * Initialization vector
         */
        vector_init(&scanners);


        /*
         * Iterate over all available network adapters in the current operating
         * system to send "Hello" requests.
         */
        for (int i=0; i<GetAdaptersCount(); i++)
        {
            // get another IP Addr and set this changes in network adapter settings.
            uint32_t host_ip_addr = inet_addr(GetAdapterAddress(i));
            uint32_t host_mask = inet_addr("255.255.255.0");
            // call the function to change adapter settings inside the library.
            set_platform_adapter_settings(host_mask, host_ip_addr);

            // Search for RF627-old devices over network by Service Protocol.
            search_scanners(scanners, kRF627_OLD, kSERVICE);
        }

        static std::vector<rf627old*> result;

        /*
         * Iterate over all discovered rf627-old in network and push into list.
         */
        for(size_t i = 0; i < vector_count(scanners); i++)
        {
            result.push_back(new rf627old((void*)vector_get(scanners,i)));
            result[i]->current_protocol = PROTOCOLS::SERVICE;
        }
        return result;
        break;
    }
    default:
    {
        static std::vector<rf627old*> result;
        return result;
        break;
    }
    }

}

rf627old::rf627old(void* base)
{
    this->scanner_base = base;
}

rf627old::~rf627old()
{

}

bool rf627old::connect(PROTOCOLS protocol)
{
    PROTOCOLS p;
    if (protocol == PROTOCOLS::CURRENT)
        p = this->current_protocol;
    else
        p = protocol;

    switch (p) {
    case PROTOCOLS::SERVICE:
    {
        // Establish connection to the RF627 device by Service Protocol.
        bool result = false;
        result = connect_to_scanner(
                    ((scanner_base_t*)this->scanner_base), kSERVICE);

        return result;
        break;
    }
    default:
        break;
    }

    return false;

}

bool rf627old::disconnect(PROTOCOLS protocol)
{
    PROTOCOLS p;
    if (protocol == PROTOCOLS::CURRENT)
        p = this->current_protocol;
    else
        p = protocol;

    switch (p) {
    case PROTOCOLS::SERVICE:
    {
        // Establish connection to the RF627 device by Service Protocol.
        bool result = false;
        result = disconnect_from_scanner(
                    (scanner_base_t*)scanner_base, kSERVICE);
        return result;
        break;
    }
    default:
        break;
    }

    return false;
}

profile2D_t* rf627old::get_profile2D(
        bool zero_points,
        PROTOCOLS protocol)
{
    PROTOCOLS p;
    if (protocol == PROTOCOLS::CURRENT)
        p = this->current_protocol;
    else
        p = protocol;

    switch (p) {
    case PROTOCOLS::SERVICE:
    {
        // Get profile from scanner's data stream by Service Protocol.
        rf627_profile2D_t* profile_from_scanner = get_profile2D_from_scanner(
                    (scanner_base_t*)scanner_base, zero_points, kSERVICE);

        profile2D_t* result = new profile2D_t;

        if(profile_from_scanner->rf627_profile2D != NULL)
        {
            result->header.data_type =
                    profile_from_scanner->rf627_profile2D->header.data_type;
            result->header.flags =
                    profile_from_scanner->rf627_profile2D->header.flags;
            result->header.device_type =
                    profile_from_scanner->rf627_profile2D->header.device_type;
            result->header.serial_number =
                    profile_from_scanner->rf627_profile2D->header.serial_number;
            result->header.system_time =
                    profile_from_scanner->rf627_profile2D->header.system_time;

            result->header.proto_version_major =
                    profile_from_scanner->rf627_profile2D->header.proto_version_major;
            result->header.proto_version_minor =
                    profile_from_scanner->rf627_profile2D->header.proto_version_minor;
            result->header.hardware_params_offset =
                    profile_from_scanner->rf627_profile2D->header.hardware_params_offset;
            result->header.data_offset =
                    profile_from_scanner->rf627_profile2D->header.data_offset;
            result->header.packet_count =
                    profile_from_scanner->rf627_profile2D->header.packet_count;
            result->header.measure_count =
                    profile_from_scanner->rf627_profile2D->header.measure_count;

            result->header.zmr =
                    profile_from_scanner->rf627_profile2D->header.zmr;
            result->header.xemr =
                    profile_from_scanner->rf627_profile2D->header.xemr;
            result->header.discrete_value =
                    profile_from_scanner->rf627_profile2D->header.discrete_value;

            result->header.exposure_time =
                    profile_from_scanner->rf627_profile2D->header.exposure_time;
            result->header.laser_value =
                    profile_from_scanner->rf627_profile2D->header.laser_value;
            result->header.step_count =
                    profile_from_scanner->rf627_profile2D->header.step_count;
            result->header.dir =
                    profile_from_scanner->rf627_profile2D->header.dir;

            switch (result->header.data_type) {
            case DTY_PixelsNormal:
            case DTY_PixelsInterpolated:
            {
                result->pixels.resize(profile_from_scanner->
                                      rf627_profile2D->pixels_format.pixels_count);

                for(size_t i = 0; i < result->pixels.size(); i++)
                {
                    result->pixels[i] = profile_from_scanner->
                            rf627_profile2D->pixels_format.pixels[i];
                }

                if(profile_from_scanner->rf627_profile2D->intensity_count > 0)
                {
                    result->intensity.resize(
                                profile_from_scanner->rf627_profile2D->intensity_count);
                    for (size_t i = 0; i < result->intensity.size(); i++)
                        result->intensity[i] =
                                profile_from_scanner->rf627_profile2D->intensity[i];
                }

                break;
            }
            case DTY_ProfileNormal:
            case DTY_ProfileInterpolated:
            {
                result->points.resize(profile_from_scanner->
                                      rf627_profile2D->profile_format.points_count);

                for(size_t i = 0; i < result->points.size(); i++)
                {
                    result->points[i].x = profile_from_scanner->rf627_profile2D->
                            profile_format.points[i].x;
                    result->points[i].z = profile_from_scanner->rf627_profile2D->
                            profile_format.points[i].z;
                }

                if(profile_from_scanner->rf627_profile2D->intensity_count > 0)
                {
                    result->intensity.resize(
                                profile_from_scanner->rf627_profile2D->intensity_count);
                    for (size_t i = 0; i < result->intensity.size(); i++)
                        result->intensity[i] =
                                profile_from_scanner->rf627_profile2D->intensity[i];
                }
                break;
            }
            default:
                break;
            }
            free(profile_from_scanner->rf627_profile2D->intensity);
            free(profile_from_scanner->rf627_profile2D->pixels_format.pixels);
            free(profile_from_scanner->rf627_profile2D);
            free(profile_from_scanner);
            return result;
        }

        free(profile_from_scanner->rf627_profile2D);
        free(profile_from_scanner);
        delete result;
    }
    default:
        break;
    }

    return NULL;

}

profile3D_t* rf627old::get_profile3D(float step_size, float k,
                                     COUNT_TYPES count_type,
                                     bool zero_points,
                                     PROTOCOLS protocol)
{
    PROTOCOLS p;
    if (protocol == PROTOCOLS::CURRENT)
        p = this->current_protocol;
    else
        p = protocol;

//    switch (p) {
//    case PROTOCOLS::SERVICE:
//    {
//        // Get profile from scanner's data stream by Service Protocol.
//        rf627_profile3D_t* profile_from_scanner = get_profile3D_from_scanner(
//                    (scanner_base_t*)scanner_base, step_size, k, (count_types_t)count_type, zero_points, kSERVICE);

//        profile3D_t* result = new profile3D_t;

//        if(profile_from_scanner->rf627_profile3D != NULL)
//        {
//            result->header.data_type =
//                    profile_from_scanner->rf627_profile3D->header.data_type;
//            result->header.flags =
//                    profile_from_scanner->rf627_profile3D->header.flags;
//            result->header.device_type =
//                    profile_from_scanner->rf627_profile3D->header.device_type;
//            result->header.serial_number =
//                    profile_from_scanner->rf627_profile3D->header.serial_number;
//            result->header.system_time =
//                    profile_from_scanner->rf627_profile3D->header.system_time;

//            result->header.proto_version_major =
//                    profile_from_scanner->rf627_profile3D->header.proto_version_major;
//            result->header.proto_version_minor =
//                    profile_from_scanner->rf627_profile3D->header.proto_version_minor;
//            result->header.hardware_params_offset =
//                    profile_from_scanner->rf627_profile3D->header.hardware_params_offset;
//            result->header.data_offset =
//                    profile_from_scanner->rf627_profile3D->header.data_offset;
//            result->header.packet_count =
//                    profile_from_scanner->rf627_profile3D->header.packet_count;
//            result->header.measure_count =
//                    profile_from_scanner->rf627_profile3D->header.measure_count;

//            result->header.zmr =
//                    profile_from_scanner->rf627_profile3D->header.zmr;
//            result->header.xemr =
//                    profile_from_scanner->rf627_profile3D->header.xemr;
//            result->header.discrete_value =
//                    profile_from_scanner->rf627_profile3D->header.discrete_value;

//            result->header.exposure_time =
//                    profile_from_scanner->rf627_profile3D->header.exposure_time;
//            result->header.laser_value =
//                    profile_from_scanner->rf627_profile3D->header.laser_value;
//            result->header.step_count =
//                    profile_from_scanner->rf627_profile3D->header.step_count;
//            result->header.dir =
//                    profile_from_scanner->rf627_profile3D->header.dir;

//            switch (result->header.data_type) {
//            case DTY_PixelsNormal:
//            case DTY_PixelsInterpolated:
//            {
//                result->pixels.resize(profile_from_scanner->
//                                      rf627_profile3D->pixels_format.pixels_count);

//                for(size_t i = 0; i < result->pixels.size(); i++)
//                {
//                    result->pixels[i] = profile_from_scanner->
//                            rf627_profile3D->pixels_format.pixels[i];
//                }

//                if(profile_from_scanner->rf627_profile3D->intensity_count > 0)
//                {
//                    result->intensity.resize(
//                                profile_from_scanner->rf627_profile3D->intensity_count);
//                    for (size_t i = 0; i < result->intensity.size(); i++)
//                        result->intensity[i] =
//                                profile_from_scanner->rf627_profile3D->intensity[i];
//                }

//                break;
//            }
//            case DTY_ProfileNormal:
//            case DTY_ProfileInterpolated:
//            {
//                result->points.resize(profile_from_scanner->
//                                      rf627_profile3D->profile_format.points_count);

//                for(size_t i = 0; i < result->points.size(); i++)
//                {
//                    result->points[i].x = profile_from_scanner->rf627_profile3D->
//                            profile_format.points[i].x;
//                    result->points[i].y = profile_from_scanner->rf627_profile3D->
//                            profile_format.points[i].y;
//                    result->points[i].z = profile_from_scanner->rf627_profile3D->
//                            profile_format.points[i].z;
//                }

//                if(profile_from_scanner->rf627_profile3D->intensity_count > 0)
//                {
//                    result->intensity.resize(
//                                profile_from_scanner->rf627_profile3D->intensity_count);
//                    for (size_t i = 0; i < result->intensity.size(); i++)
//                        result->intensity[i] =
//                                profile_from_scanner->rf627_profile3D->intensity[i];
//                }
//                break;
//            }
//            default:
//                break;
//            }
//            free(profile_from_scanner->rf627_profile3D->intensity);
//            free(profile_from_scanner->rf627_profile3D->pixels_format.pixels);
//            free(profile_from_scanner->rf627_profile3D);
//            free(profile_from_scanner);
//            return result;
//        }

//        free(profile_from_scanner->rf627_profile3D);
//        free(profile_from_scanner);
//        delete result;
//    }
//    default:
//        break;
//    }

    return NULL;

}

bool rf627old::read_params(PROTOCOLS protocol)
{
    PROTOCOLS p;
    if (protocol == PROTOCOLS::CURRENT)
        p = this->current_protocol;
    else
        p = protocol;

    switch (p) {
    case PROTOCOLS::SERVICE:
    {
        // Establish connection to the RF627 device by Service Protocol.
        bool result = false;
        result = read_params_from_scanner(
                    (scanner_base_t*)scanner_base, kSERVICE);
        return result;
        break;
    }
    default:
        break;
    }

    return false;
}

bool rf627old::write_params(PROTOCOLS protocol)
{
    PROTOCOLS p;
    if (protocol == PROTOCOLS::CURRENT)
        p = this->current_protocol;
    else
        p = protocol;

    switch (p) {
    case PROTOCOLS::SERVICE:
    {
        // Establish connection to the RF627 device by Service Protocol.
        bool result = false;
        result = write_params_to_scanner(
                    (scanner_base_t*)scanner_base, kSERVICE);
        return result;
        break;
    }
    default:
        break;
    }

    return false;
}

param_t* create_param_from_type(std::string type)
{
    param_t* p = nullptr;
    if(type == pvtKey[PVT_UINT])
    {
        p = new value_uint32();
        p->type = type;
    }else if(type == pvtKey[PVT_UINT64])
    {
        p = new value_uint64();
        p->type = type;
    }else if(type == pvtKey[PVT_INT])
    {
        p = new value_int32();
        p->type = type;
    }else if(type == pvtKey[PVT_INT64])
    {
        p = new value_int64();
        p->type = type;
    }else if(type == pvtKey[PVT_FLOAT])
    {
        p = new value_flt();
        p->type = type;
    }else if(type == pvtKey[PVT_DOUBLE])
    {
        p = new value_dbl();
        p->type = type;
    }else if(type == pvtKey[PVT_ARRAY_UINT32])
    {
        p = new array_uint();
        p->type = type;
    }else if(type == pvtKey[PVT_ARRAY_UINT64])
    {
        p = new array_uint64();
        p->type = type;
    }else if(type == pvtKey[PVT_ARRAY_INT32])
    {
        p = new array_int32();
        p->type = type;
    }else if(type == pvtKey[PVT_ARRAY_INT64])
    {
        p = new array_int64();
        p->type = type;
    }else if(type == pvtKey[PVT_ARRAY_FLT])
    {
        p = new array_flt();
        p->type = type;
    }else if(type == pvtKey[PVT_ARRAY_DBL])
    {
        p = new array_dbl();
        p->type = type;
    }else if(type == pvtKey[PVT_STRING])
    {
        p = new value_str();
        p->type = type;
    }
    return p;
}

param_t *rf627old::get_param(PARAM_NAME_KEY param_name)
{
    return get_param(parameter_names[(uint8_t)param_name]);
}

param_t *rf627old::get_param(std::string param_name)
{
    parameter_t* p = get_parameter(
                (scanner_base_t*)this->scanner_base, param_name.c_str());
    if (p != NULL)
    {
        param_t* result = create_param_from_type(std::string(p->base.type));


        if (result->type == pvtKey[PVT_UINT])
        {
            result->set_value<value_uint32_t>(p->val_uint->value);
            result->name = p->base.name;
            result->access = p->base.access;
            result->index = p->base.index;
            result->offset = p->base.offset;
            result->size = p->base.size;
            result->units = p->base.units;

            ((value_uint32*)result)->defaultValue = p->val_uint->defValue;
            ((value_uint32*)result)->value = p->val_uint->value;
            ((value_uint32*)result)->min = p->val_uint->min;
            ((value_uint32*)result)->max = p->val_uint->max;

            if(p->val_uint->enumValues != NULL)
            for(int i = 0; i < p->val_uint->enumValues->recCount; i++)
                ((value_uint32*)result)->valuesEnum.push_back(std::make_pair(
                            p->val_uint->enumValues->rec[i].value,
                            p->val_uint->enumValues->rec[i].key));
        }
        else if (result->type == pvtKey[PVT_UINT64])
        {
            result->set_value<value_uint64_t>(p->val_uint64->value);
            result->name = p->base.name;
            result->access = p->base.access;
            result->index = p->base.index;
            result->offset = p->base.offset;
            result->size = p->base.size;
            result->units = p->base.units;

            ((value_uint64*)result)->defaultValue = p->val_uint64->defValue;
            ((value_uint64*)result)->value = p->val_uint64->value;
            ((value_uint64*)result)->min = p->val_uint64->min;
            ((value_uint64*)result)->max = p->val_uint64->max;

            if(p->val_uint64->enumValues != NULL)
            for(int i = 0; i < p->val_uint64->enumValues->recCount; i++)
                ((value_uint64*)result)->valuesEnum.push_back(std::make_pair(
                            p->val_uint64->enumValues->rec[i].value,
                            p->val_uint64->enumValues->rec[i].key));
        }
        else if (result->type == pvtKey[PVT_INT])
        {
            result->set_value<value_int32_t>(p->val_int->value);
            result->name = p->base.name;
            result->access = p->base.access;
            result->index = p->base.index;
            result->offset = p->base.offset;
            result->size = p->base.size;
            result->units = p->base.units;

            ((value_int32*)result)->defaultValue = p->val_int->defValue;
            ((value_int32*)result)->value = p->val_int->value;
            ((value_int32*)result)->min = p->val_int->min;
            ((value_int32*)result)->max = p->val_int->max;

            if(p->val_int->enumValues != NULL)
            for(int i = 0; i < p->val_int->enumValues->recCount; i++)
                ((value_int32*)result)->valuesEnum.push_back(std::make_pair(
                            p->val_int->enumValues->rec[i].value,
                            p->val_int->enumValues->rec[i].key));

        }
        else if (result->type == pvtKey[PVT_INT64])
        {
            result->set_value<value_int64_t>(p->val_int64->value);
            result->name = p->base.name;
            result->access = p->base.access;
            result->index = p->base.index;
            result->offset = p->base.offset;
            result->size = p->base.size;
            result->units = p->base.units;

            ((value_int64*)result)->defaultValue = p->val_int64->defValue;
            ((value_int64*)result)->value = p->val_int64->value;
            ((value_int64*)result)->min = p->val_int64->min;
            ((value_int64*)result)->max = p->val_int64->max;

            if(p->val_int64->enumValues != NULL)
            for(int i = 0; i < p->val_int64->enumValues->recCount; i++)
                ((value_int64*)result)->valuesEnum.push_back(std::make_pair(
                            p->val_int64->enumValues->rec[i].value,
                            p->val_int64->enumValues->rec[i].key));

        }
        else if (result->type == pvtKey[PVT_FLOAT])
        {
            result->set_value<value_flt_t>(p->val_flt->value);
            result->name = p->base.name;
            result->access = p->base.access;
            result->index = p->base.index;
            result->offset = p->base.offset;
            result->size = p->base.size;
            result->units = p->base.units;

            ((value_flt*)result)->defaultValue = p->val_flt->defValue;
            ((value_flt*)result)->value = p->val_flt->value;
            ((value_flt*)result)->min = p->val_flt->min;
            ((value_flt*)result)->max = p->val_flt->max;
        }
        else if (result->type == pvtKey[PVT_DOUBLE])
        {
            result->set_value<value_dbl_t>(p->val_dbl->value);
            result->name = p->base.name;
            result->access = p->base.access;
            result->index = p->base.index;
            result->offset = p->base.offset;
            result->size = p->base.size;
            result->units = p->base.units;

            ((value_dbl*)result)->defaultValue = p->val_dbl->defValue;
            ((value_dbl*)result)->value = p->val_dbl->value;
            ((value_dbl*)result)->min = p->val_dbl->min;
            ((value_dbl*)result)->max = p->val_dbl->max;
        }
        else if (result->type == pvtKey[PVT_ARRAY_UINT32])
        {
//            parse_string_param(p, array_uint_t, name);
//            parse_string_param(p, array_uint_t, type);
//            parse_string_param(p, array_uint_t, access);
//            parse_uint16_param(p, array_uint_t, index);
//            parse_uint32_param(p, array_uint_t, offset);
//            parse_uint32_param(p, array_uint_t, size);
//            parse_string_param(p, array_uint_t, units);

//            parse_uint32_param(p, array_uint_t, maxCount);
//            parse_uint32_param(p, array_uint_t, defCount);
//            parse_uint32_param(p, array_uint_t, count);

//            parse_arr_param(p, array_uint_t, defaultValue, uint32_t);
//            parse_arr_param(p, array_uint_t, value, uint32_t);

//            parse_uint32_param(p, array_uint_t, min);
//            parse_uint32_param(p, array_uint_t, max);
        }
        else if (result->type == pvtKey[PVT_ARRAY_UINT64])
        {
//            parse_string_param(p, array_uint64_t, name);
//            parse_string_param(p, array_uint64_t, type);
//            parse_string_param(p, array_uint64_t, access);
//            parse_uint16_param(p, array_uint64_t, index);
//            parse_uint32_param(p, array_uint64_t, offset);
//            parse_uint32_param(p, array_uint64_t, size);
//            parse_string_param(p, array_uint64_t, units);

//            parse_uint32_param(p, array_uint64_t, maxCount);
//            parse_uint32_param(p, array_uint64_t, defCount);
//            parse_uint32_param(p, array_uint64_t, count);

//            parse_arr_param(p, array_uint64_t, defaultValue, uint64_t);
//            parse_arr_param(p, array_uint64_t, value, uint64_t);

//            parse_uint64_param(p, array_uint64_t, min);
//            parse_uint64_param(p, array_uint64_t, max);
        }
        else if (result->type == pvtKey[PVT_ARRAY_INT32])
        {
//            parse_string_param(p, array_int32_t, name);
//            parse_string_param(p, array_int32_t, type);
//            parse_string_param(p, array_int32_t, access);
//            parse_uint16_param(p, array_int32_t, index);
//            parse_uint32_param(p, array_int32_t, offset);
//            parse_uint32_param(p, array_int32_t, size);
//            parse_string_param(p, array_int32_t, units);

//            parse_uint32_param(p, array_int32_t, maxCount);
//            parse_uint32_param(p, array_int32_t, defCount);
//            parse_uint32_param(p, array_int32_t, count);

//            parse_arr_param(p, array_int32_t, defaultValue, int32_t);
//            parse_arr_param(p, array_int32_t, value, int32_t);

//            parse_int32_param(p, array_int32_t, min);
//            parse_int32_param(p, array_int32_t, max);
        }
        else if (result->type == pvtKey[PVT_ARRAY_INT64])
        {
//            parse_string_param(p, array_int64_t, name);
//            parse_string_param(p, array_int64_t, type);
//            parse_string_param(p, array_int64_t, access);
//            parse_uint16_param(p, array_int64_t, index);
//            parse_uint32_param(p, array_int64_t, offset);
//            parse_uint32_param(p, array_int64_t, size);
//            parse_string_param(p, array_int64_t, units);

//            parse_uint32_param(p, array_int64_t, maxCount);
//            parse_uint32_param(p, array_int64_t, defCount);
//            parse_uint32_param(p, array_int64_t, count);

//            parse_arr_param(p, array_int64_t, defaultValue, int64_t);
//            parse_arr_param(p, array_int64_t, value, int64_t);

//            parse_int64_param(p, array_int64_t, min);
//            parse_int64_param(p, array_int64_t, max);
        }
        else if (result->type == pvtKey[PVT_ARRAY_FLT])
        {
//            parse_string_param(p, array_flt_t, name);
//            parse_string_param(p, array_flt_t, type);
//            parse_string_param(p, array_flt_t, access);
//            parse_uint16_param(p, array_flt_t, index);
//            parse_uint32_param(p, array_flt_t, offset);
//            parse_uint32_param(p, array_flt_t, size);
//            parse_string_param(p, array_flt_t, units);

//            parse_uint32_param(p, array_flt_t, maxCount);
//            parse_uint32_param(p, array_flt_t, defCount);
//            parse_uint32_param(p, array_flt_t, count);

//            parse_arr_param(p, array_flt_t, defaultValue, float);
//            parse_arr_param(p, array_flt_t, value, float);

//            parse_float_param(p, array_flt_t, min);
//            parse_float_param(p, array_flt_t, max);
        }
        else if (result->type == pvtKey[PVT_ARRAY_DBL])
        {
//            parse_string_param(p, array_dbl_t, name);
//            parse_string_param(p, array_dbl_t, type);
//            parse_string_param(p, array_dbl_t, access);
//            parse_uint16_param(p, array_dbl_t, index);
//            parse_uint32_param(p, array_dbl_t, offset);
//            parse_uint32_param(p, array_dbl_t, size);
//            parse_string_param(p, array_dbl_t, units);

//            parse_uint32_param(p, array_dbl_t, maxCount);
//            parse_uint32_param(p, array_dbl_t, defCount);
//            parse_uint32_param(p, array_dbl_t, count);

//            parse_arr_param(p, array_dbl_t, defaultValue, double);
//            parse_arr_param(p, array_dbl_t, value, double);

//            parse_double_param(p, array_dbl_t, min);
//            parse_double_param(p, array_dbl_t, max);
        }
        else if (result->type == pvtKey[PVT_STRING])
        {
            result->set_value<value_str_t>(p->val_str->value);
            result->name = p->base.name;
            result->access = p->base.access;
            result->index = p->base.index;
            result->offset = p->base.offset;
            result->size = p->base.size;
            result->units = p->base.units;

            ((value_str*)result)->defaultValue = p->val_str->defValue;
            ((value_str*)result)->value = p->val_str->value;
            ((value_str*)result)->maxLen = p->val_str->maxLen;

//            parse_string_param(p, value_str_t, defaultValue);
//            parse_string_param(p, value_str_t, value);
//            parse_uint16_param(p, value_str_t, maxLen);

        }else if (result->type == pvtKey[PVT_UNKN])
        {
//            parse_string_param(p, value_raw_t, name);
//            parse_string_param(p, value_raw_t, type);
//            parse_string_param(p, value_raw_t, access);
//            parse_uint16_param(p, value_raw_t, index);
//            parse_uint32_param(p, value_raw_t, offset);
//            parse_uint32_param(p, value_raw_t, size);

//            parse_raw_param(p, value_raw_t);


        }

        return result;
    }
    return nullptr;
}

parameter_t* create_parameter_from_type(std::string type)
{
    parameter_t* p = NULL;
    if (type == pvtKey[PVT_STRING])
    {
        p = (parameter_t*)calloc(1, sizeof (parameter_t));
        p->val_uint = (value_uint32_t*)calloc(1, sizeof (p->val_uint));
        p->base.type = param_value_types[(uint8_t)PARAM_VALUE_TYPE::UINT_PARAM_TYPE].c_str();
    }else if (type == pvtKey[PVT_UINT64])
    {
        p = (parameter_t*)calloc(1, sizeof (parameter_t));
        p->val_uint64 = (value_uint64_t*)calloc(1, sizeof (p->val_uint64));
        p->base.type = param_value_types[(uint8_t)PARAM_VALUE_TYPE::UINT64_PARAM_TYPE].c_str();
    }else if (type ==  pvtKey[PVT_INT])
    {
        p = (parameter_t*)calloc(1, sizeof (parameter_t));
        p->val_int = (value_int32_t*)calloc(1, sizeof (p->val_int));
        p->base.type = param_value_types[(uint8_t)PARAM_VALUE_TYPE::INT_PARAM_TYPE].c_str();
    }else if (type ==  pvtKey[PVT_INT64])
    {
        p = (parameter_t*)calloc(1, sizeof (parameter_t));
        p->val_int64 = (value_int64_t*)calloc(1, sizeof (p->val_int64));
        p->base.type = param_value_types[(uint8_t)PARAM_VALUE_TYPE::INT64_PARAM_TYPE].c_str();
    }else if (type ==  pvtKey[PVT_FLOAT])
    {
        p = (parameter_t*)calloc(1, sizeof (parameter_t));
        p->val_flt = (value_flt_t*)calloc(1, sizeof (p->val_flt));
        p->base.type = param_value_types[(uint8_t)PARAM_VALUE_TYPE::FLOAT_PARAM_TYPE].c_str();
    }else if(type ==  pvtKey[PVT_DOUBLE])
    {
        p = (parameter_t*)calloc(1, sizeof (parameter_t));
        p->val_dbl = (value_dbl_t*)calloc(1, sizeof (p->val_dbl));
        p->base.type = param_value_types[(uint8_t)PARAM_VALUE_TYPE::DOUBLE_PARAM_TYPE].c_str();
    }else if (type ==  pvtKey[PVT_ARRAY_UINT32])
    {
        p = (parameter_t*)calloc(1, sizeof (parameter_t));
        p->arr_uint = (array_uint32_t*)calloc(1, sizeof (p->arr_uint));
        p->base.type = param_value_types[(uint8_t)PARAM_VALUE_TYPE::UINT32_ARRAY_PARAM_TYPE].c_str();
    }else if (type ==  pvtKey[PVT_ARRAY_UINT64])
    {
        p = (parameter_t*)calloc(1, sizeof (parameter_t));
        p->arr_uint64 = (array_uint64_t*)calloc(1, sizeof (p->arr_uint64));
        p->base.type = param_value_types[(uint8_t)PARAM_VALUE_TYPE::UINT64_ARRAY_PARAM_TYPE].c_str();
    }else if (type ==  pvtKey[PVT_ARRAY_INT32])
    {
        p = (parameter_t*)calloc(1, sizeof (parameter_t));
        p->arr_int = (array_int32_t*)calloc(1, sizeof (p->arr_int));
        p->base.type = param_value_types[(uint8_t)PARAM_VALUE_TYPE::INT32_ARRAY_PARAM_TYPE].c_str();
    }else if (type ==  pvtKey[PVT_ARRAY_INT64])
    {
        p = (parameter_t*)calloc(1, sizeof (parameter_t));
        p->arr_int64 = (array_int64_t*)calloc(1, sizeof (p->arr_int64));
        p->base.type = param_value_types[(uint8_t)PARAM_VALUE_TYPE::INT64_ARRAY_PARAM_TYPE].c_str();
    }else if (type ==  pvtKey[PVT_ARRAY_FLT])
    {
        p = (parameter_t*)calloc(1, sizeof (parameter_t));
        p->arr_flt = (array_flt_t*)calloc(1, sizeof (p->arr_flt));
        p->base.type = param_value_types[(uint8_t)PARAM_VALUE_TYPE::FLT_ARRAY_PARAM_TYPE].c_str();
    }else if (type ==  pvtKey[PVT_ARRAY_DBL])
    {
        p = (parameter_t*)calloc(1, sizeof (parameter_t));
        p->arr_dbl = (array_dbl_t*)calloc(1, sizeof (p->arr_dbl));
        p->base.type = param_value_types[(uint8_t)PARAM_VALUE_TYPE::DBL_ARRAY_PARAM_TYPE].c_str();
    }else if (type ==  pvtKey[PVT_STRING])
    {
        p = (parameter_t*)calloc(1, sizeof (parameter_t));
        p->val_str = (value_str_t*)calloc(1, sizeof (p->val_str));
        p->base.type = param_value_types[(uint8_t)PARAM_VALUE_TYPE::STRING_PARAM_TYPE].c_str();
    }
    return p;
}

bool rf627old::set_param(param_t* param)
{
    parameter_t* p = create_parameter_from_type(param->type.c_str());
    if (p != NULL)
    {
        p->base.name = param->name.c_str();
        p->base.type = param->type.c_str();
        p->base.access = param->access.c_str();
        p->base.units = param->units.c_str();
        if (param->type == pvtKey[PVT_STRING])
        {
            p->val_str->value = (char*)param->get_value<value_str>().c_str();
            p->base.size = param->get_value<value_str>().size() + 1;
        }
        else if (param->type == pvtKey[PVT_INT])
        {
            p->val_int->value = param->get_value<value_int32_t>();
        }
        else if (param->type == pvtKey[PVT_INT64])
        {
            p->val_int64->value = param->get_value<value_int64_t>();
        }
        else if (param->type == pvtKey[PVT_UINT])
        {
            p->val_uint->value = param->get_value<value_uint32_t>();
        }
        else if (param->type == pvtKey[PVT_UINT64])
        {
            p->val_uint64->value = param->get_value<value_uint64_t>();
        }
        else if (param->type == pvtKey[PVT_FLOAT])
        {
            p->val_flt->value = param->get_value<value_flt_t>();
        }
        else if (param->type ==  pvtKey[PVT_DOUBLE])
        {
            p->val_dbl->value = param->get_value<value_dbl_t>();
        }
        set_parameter((scanner_base_t*)this->scanner_base, p);
        return true;
    }
    return false;
}
bool rf627old::set_param(const char* param_name, ...)
{
    va_list valist;
    va_start(valist, param_name);

    bool result = set_parameter_by_name(
                (scanner_base_t*)this->scanner_base, param_name, valist);

    va_end(valist);

    return result;
}
bool rf627old::set_param(int param_id, ...)
{
    va_list valist;
    va_start(valist, param_id);

    bool result = set_parameter_by_name(
                (scanner_base_t*)this->scanner_base, parameter_names[param_id].c_str(), valist);

    va_end(valist);

    return result;
}

bool rf627old::send_cmd(const char* command_name, ...)
{
    va_list valist;
    va_start(valist, command_name);

    //int arg1 = va_arg(valist, int);
    //int arg2 = va_arg(valist, int);

    command_t cmd = {0};
    cmd.name = command_name;
    cmd.arg_list = valist;


    int arg3 = va_arg(valist, int);
    int arg4 = va_arg(valist, int);



    bool result = send_command((scanner_base_t*)this->scanner_base, &cmd);

    va_end(valist);

    return result;
}

//typedef enum
//{
//    PAT_UNKN				= 0,
//    PAT_READ_ONLY,
//    PAT_WRITE,
//    PAT_RESTRICTED,
//}params_name_offset_t;

//template <typename  T>
//auto rf627old::get_param (std::string name)->decltype( std::declval<T>().value )
//{
//    return 0;
//}





}
}
}
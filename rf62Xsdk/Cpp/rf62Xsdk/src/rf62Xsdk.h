#pragma once

#include"rf62Xtypes.h"

#include <vector>
#include <memory>
#include <iostream>

#if (defined _WIN32 && defined RF627_LIBRARY)
#define API_EXPORT __declspec(dllexport)
#else
#define API_EXPORT
#endif

namespace SDK {
namespace SCANNERS {
namespace RF62X {



/**
 * @brief sdk_version - Return info about SDK version
 * @return SDK version
 */
API_EXPORT int sdk_version();

/**
 * @brief sdk_init - Initialize sdk library
 * Must be called once before further calls to any library functions
 * @return true if success.
 */
API_EXPORT bool sdk_init();

/**
 * @brief sdk_cleanup - Cleanup resources allocated with sdk_init() function
 */
API_EXPORT void sdk_cleanup();


class API_EXPORT rf627old
{

public:
    /**
     * @brief search - Search for RF627old devices over network
     * @param protocol - protocol's type (Service Protocol, ENIP, Modbus-TCP)
     * @return vector of rf627old devices
     */
    static std::vector<rf627old*> search(PROTOCOLS protocol);

    /**
     * @brief connect - Establish connection to the RF627old device
     * @param protocol - protocol's type (Service Protocol, ENIP, Modbus-TCP)
     * @return true on success
     */
    bool connect(PROTOCOLS protocol = PROTOCOLS::CURRENT);

    /**
     * @brief disconnect_from_scanner - Close connection to the device
     * @param protocol - protocol's type (Service Protocol, ENIP, Modbus-TCP)
     * @return true on success
     */
    bool disconnect(PROTOCOLS protocol = PROTOCOLS::CURRENT);

    /**
     * @brief get_profile2D  - Get 2D measurement from scanner's data stream
     * @param zero_points - include zero points in return profile2D
     * @param protocol - protocol's type (Service Protocol, ENIP, Modbus-TCP)
     * @return ptr to profile2D_t structure if success, else - null
     */
    profile2D_t* get_profile2D(
            bool zero_points = true,
            PROTOCOLS protocol = PROTOCOLS::CURRENT);

    /**
     * @brief get_profile3D - Get 3D measurement from scanner's data stream
     * where y is calculated based on the next equation: y = k * x + b
     * where b - y-intercept of the line, calculates by the next equation: b = step_size * count_value
     * @param step_size - step size in real units (mm, sm, etc.)
     * @param k - slope or gradient of the line , where
     * @param count_type - type of counter (STEP, MEASURE, PACKET)
     * @param zero_points - include zero points in return profile2D
     * @param protocol - protocol's type (Service Protocol, ENIP, Modbus-TCP)
     * @return ptr to profile3D_t structure if success, else - null
     */
    profile3D_t* get_profile3D(float step_size, float k = 0,
                               COUNT_TYPES count_type = COUNT_TYPES::MEASURE,
                               bool zero_points = true,
                               PROTOCOLS protocol = PROTOCOLS::CURRENT);

    /**
     * @brief read_params - Read parameters from device to internal structure.
     * This structure is accessible via get_params() function
     * @param protocol - protocol's type (Service Protocol, ENIP, Modbus-TCP)
     * @return true on success
     */
    bool read_params(PROTOCOLS protocol = PROTOCOLS::CURRENT);

    /**
     * @brief write_params - Write current parameters to device's memory
     * @param protocol - protocol's type (Service Protocol, ENIP, Modbus-TCP)
     * @return true on success
     */
    bool write_params(PROTOCOLS protocol = PROTOCOLS::CURRENT);

    /**
     * @brief get_param - Search parameters by his name
     * @param param_name - name of parameter
     * @return param on success, else - null
     */
    param_t* get_param(std::string param_name);
    param_t* get_param(PARAM_NAME_KEY param_name);
    /**
     * @brief set_param - set parameter
     * @param param - prt to parameter
     * @return true on success, else - false
     */
    bool set_param(param_t* param);
    bool set_param(const char* param_name, ...);
    bool set_param(int param_id, ...);

    bool send_cmd(const char* command_name, ...);

    rf627old(void* scanner_base);
    ~rf627old();

private:
    void* scanner_base = NULL;
    PROTOCOLS current_protocol;

};

}
}
}

#ifndef HYCOMM_ECHOCONFIG_HPP
#define HYCOMM_ECHOCONFIG_HPP

#include <string>

namespace hy::configs
{
    struct EchoConfig
    {
        std::string echo_id;
        bool uppercase_mode = false;
    };
}

#endif //HYCOMM_ECHOCONFIG_HPP


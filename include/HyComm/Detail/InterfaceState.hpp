#ifndef HYCOMM_INTERFACESTATE_HPP
#define HYCOMM_INTERFACESTATE_HPP

namespace hy::detail
{
    enum class InterfaceState
    {
        Closed,
        Opening,
        Open,
        Closing,
        Error
    };
}

#endif //HYCOMM_INTERFACESTATE_HPP

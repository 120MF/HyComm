#ifndef HYCOMM_RESPONSE_HPP
#define HYCOMM_RESPONSE_HPP

#include <variant>

#include <tl/expected.hpp>

#include <HyComm/Common/Error.hpp>

namespace hy::ipc
{
    struct GenericSuccessResponse
    {
    };

    using SuccessResponse = std::variant<
        GenericSuccessResponse
    >;

    using Response = tl::expected<SuccessResponse, common::Error>;
}


#endif //HYCOMM_RESPONSE_HPP

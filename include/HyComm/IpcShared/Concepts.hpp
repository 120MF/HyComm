#ifndef HYCOMM_CONCEPT_HPP
#define HYCOMM_CONCEPT_HPP

#include <string>
#include <concepts>

template <typename T>
concept RequestWithUdsPath = requires(T t)
{
    t.uds_path = std::string{};
};

#endif //HYCOMM_CONCEPT_HPP

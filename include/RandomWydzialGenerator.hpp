#ifndef P1_CPP_LAB_RANDOMWYDZIALGENERATOR_HPP
#define P1_CPP_LAB_RANDOMWYDZIALGENERATOR_HPP

#include <array>
#include <string>
#include <random>

inline constexpr std::array<const char*,15> Wydzial{
    "MEIL",
    "CHEM",
    "GIK",
    "AiNS",
    "ARCH",
    "EITI",
    "EL",
    "FIZ",
    "ICHIP",
    "IBHIS",
    "WIM",
    "MINI",
    "WMT",
    "MECH",
    "SIMR",
};

inline auto getRandomWydzial() -> std::string {
    static auto prng = std::mt19937{std::random_device{}()};
    auto dist = std::uniform_int_distribution<std::size_t>{0, Wydzial.size() - 1};
    const auto randomIndex = dist(prng);
    return std::string(Wydzial[randomIndex]);
}

#endif //P1_CPP_LAB_RANDOMWYDZIALGENERATOR_HPP

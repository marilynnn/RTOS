#include <cstdint>

#include <unistd.h>
#include <devctl.h>
#include <sys/types.h>

namespace bbs {

	struct BBSParams
	{
    std::uint32_t seed;
    std::uint32_t p;
    std::uint32_t q;
    BBSParams() = default;
    BBSParams(std::uint32_t seed_, std::uint32_t p_, std::uint32_t q_)
    : seed(seed_), p(p_), q(q_) {};
    ~BBSParams() = default;
	};

}

#define _SET_PARAMS __DIOT(_DCMD_MISC, 1, bbs::BBSParams)
//__DIOT Passes parameters
#define _GET_ELEM __DIOF(_DCMD_MISC, 2, std::uint32_t)
//__DIOF Get an element of prs

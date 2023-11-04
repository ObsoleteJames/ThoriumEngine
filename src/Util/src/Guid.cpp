
#include "../Include/Util/Guid.h"
#include <random>

static std::random_device _randomd;
static std::mt19937_64 _engine(_randomd());
static std::uniform_int_distribution<SizeType> _UniformDist;

FGuid::FGuid() : id(_UniformDist(_engine))
{
}


#include "AudioSource.h"
#include "Math/Vectors.h"

void IAudioSource::SetPosition(FVector* target, bool bUpdatePosition /*= true*/)
{
	position = target;
	bUpdatePos = bUpdatePosition;
}

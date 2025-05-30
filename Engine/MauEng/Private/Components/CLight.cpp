#include "Components/CLight.h"

#include "InternalServiceLocator.h"

namespace MauEng
{
	CLight::CLight()
	{
		lightID = RENDERER.CreateLight();
	}
}

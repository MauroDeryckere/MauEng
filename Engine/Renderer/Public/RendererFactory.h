#ifndef RENDERRENDERERFACTORY_H
#define RENDERRENDERERFACTORY_H

#include <memory>
#include "Renderer.h"

namespace MauRen
{
	std::unique_ptr<Renderer> CreateRenderer();
}

#endif // RENDERRENDERERFACTORY_H
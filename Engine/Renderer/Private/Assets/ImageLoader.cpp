#include "ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace MauRen
{
	Image::Image(std::string const& filepath, int desiredChannels)
	{
		pixels = stbi_load(filepath.c_str(), &width, &height, &channels, desiredChannels);

		if (!pixels or width == 0 or height == 0 )
		{
			throw std::runtime_error("Failed to load texture image!");
		}

		if (desiredChannels > 0)
		{
			channels = desiredChannels;
		}
	}

	Image::Image(const uint8_t* data, size_t size, bool compressed, int desiredChannels)
	{
		if (compressed)
		{
			pixels = stbi_load_from_memory(data, static_cast<int>(size), &width, &height, &channels, desiredChannels);
			ownsPixels = true;
		}
		else
		{
			width = 0;
			height = 0;
			pixels = new uint8_t[size];
			std::memcpy(pixels, data, size);
			ownsPixels = false;
		}

		if (!pixels or width == 0 or height == 0 )
		{
			throw std::runtime_error("Failed to load texture image!");
		}

		if (desiredChannels > 0)
		{
			channels = desiredChannels;
		}
	}

	Image::~Image()
	{
		if (pixels)
		{
			if (ownsPixels)
			{
				stbi_image_free(pixels);
			}
			else
			{
				delete[] pixels;
			}
			pixels = nullptr;
		}
	}

	HDRI_Image::HDRI_Image(std::string const& filepath, int desiredChannels)
	{
		pixels = stbi_loadf(filepath.c_str(), &width, &height, &channels, desiredChannels);

		if (!pixels or width == 0 or height == 0)
		{
			throw std::runtime_error("Failed to load texture image!");
		}

		if (desiredChannels > 0)
		{
			channels = desiredChannels;
		}
	}

	HDRI_Image::~HDRI_Image()
	{
		if (pixels)
		{
			delete[] pixels;
			pixels = nullptr;
		}
	}
}

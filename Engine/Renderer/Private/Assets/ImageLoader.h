#ifndef MAUREN_IMAGELOADER_H
#define MAUREN_IMAGELOADER_H

namespace MauRen
{
	struct Image final
	{
		int width{ 0 };
		int height{ 0 };
		int channels{ 0 };
		uint8_t* pixels{ nullptr };
		bool ownsPixels = true;

		Image() = default;
		Image(std::string const& filepath, int desiredChannels = 4);
		Image(const uint8_t* data, size_t size, bool compressed = true, int desiredChannels = 4);
		~Image();

		[[nodiscard]] bool isValid() const noexcept { return pixels != nullptr; }

		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
		Image(Image&& other) noexcept = delete;
		Image& operator=(Image&& other) noexcept = delete;
	};
}

#endif
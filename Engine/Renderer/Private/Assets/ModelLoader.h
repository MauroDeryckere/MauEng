#ifndef MAUREN_MODELLOADER_H
#define MAUREN_MODELLOADER_H

namespace MauRen
{
	class ModelLoader
	{
	public:
		ModelLoader() = default;
		~ModelLoader() = default;

		ModelLoader(ModelLoader const&) = delete;
		ModelLoader(ModelLoader&&) = delete;
		ModelLoader& operator=(ModelLoader const&) = delete;
		ModelLoader& operator=(ModelLoader const&&) = delete;

		static void LoadModel(const std::string& path);
	private:
	};
}

#endif
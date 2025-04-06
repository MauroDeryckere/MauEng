#ifndef MAUREN_DEBUGRENDERER_H
#define MAUREN_DEBUGRENDERER_H

namespace MauRen
{
	class Renderer;

	class DebugRenderer
	{
	public:
		explicit DebugRenderer() {}
		virtual ~DebugRenderer() = default;

		DebugRenderer(DebugRenderer const&) = delete;
		DebugRenderer(DebugRenderer&&) = delete;
		DebugRenderer& operator=(DebugRenderer const&) = delete;
		DebugRenderer& operator=(DebugRenderer&&) = delete;
	};

	class NullDebugRenderer final : public DebugRenderer
	{
	public:
		explicit NullDebugRenderer() :
			DebugRenderer{ }
		{ }

		virtual ~NullDebugRenderer() override = default;

		NullDebugRenderer(NullDebugRenderer const&) = delete;
		NullDebugRenderer(NullDebugRenderer&&) = delete;
		NullDebugRenderer& operator=(NullDebugRenderer const&) = delete;
		NullDebugRenderer& operator=(NullDebugRenderer&&) = delete;
	};
}

#endif // MAUREN_DEBUGRENDERER_H
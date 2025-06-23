#ifndef MAUREN_MESHINSTANCE_H
#define MAUREN_MESHINSTANCE_H

namespace MauRen
{
	struct MeshInstance final
	{
		MeshInstance(uint32_t meshID, uint32_t materialID):
			m_MeshID{ meshID },
			m_MaterialID{ materialID }
		{
			ME_ASSERT(m_MeshID != UINT32_MAX);
			ME_ASSERT(m_MaterialID != UINT32_MAX);
		}

		MeshInstance() = default;
		~MeshInstance() = default;

		MeshInstance(MeshInstance const&) = default;
		MeshInstance(MeshInstance&&) = default;
		MeshInstance& operator=(MeshInstance const&) = default;
		MeshInstance& operator=(MeshInstance&&) = default;

		uint32_t m_MeshID{ UINT32_MAX };
		uint32_t m_MaterialID { UINT32_MAX };
	};
}

#endif
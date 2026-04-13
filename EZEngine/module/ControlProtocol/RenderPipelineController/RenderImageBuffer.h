#pragma once
#ifndef __RENDER_IMAGE_BUFFER_H__
#define __RENDER_IMAGE_BUFFER_H__

#include <array>

#include "core/Render/RenderHandles.h"
#include "core/Render/RenderImageDesc.h"

namespace ControlProtocol
{
	struct RenderImageSlot
	{
		EZ::RenderImageDesc desc{};
		EZ::RenderImageHandle imageHandle = 0;
		EZ::RenderTargetHandle renderTargetHandle = 0;
	};

	class RenderImageBuffer
	{
	public:
		static constexpr EZ::u32 SlotCount =
			static_cast<EZ::u32>(EZ::RenderImageTag::Count);

	public:
		void ResetFrame()
		{
			for (auto& slot : m_Slots)
			{
				if (!slot.desc.persistent && !slot.desc.imported)
				{
					slot = {};
				}
			}
		}

		RenderImageSlot& GetOrCreate(EZ::RenderImageTag tag)
		{
			auto& slot = m_Slots[static_cast<EZ::u32>(tag)];
			slot.desc.tag = tag;
			slot.desc.valid = true;
			return slot;
		}

		RenderImageSlot* TryGet(EZ::RenderImageTag tag)
		{
			auto& slot = m_Slots[static_cast<EZ::u32>(tag)];
			return slot.desc.valid ? &slot : nullptr;
		}

		const RenderImageSlot* TryGet(EZ::RenderImageTag tag) const
		{
			const auto& slot = m_Slots[static_cast<EZ::u32>(tag)];
			return slot.desc.valid ? &slot : nullptr;
		}

		bool Has(EZ::RenderImageTag tag) const
		{
			return TryGet(tag) != nullptr;
		}

		void Remove(EZ::RenderImageTag tag)
		{
			m_Slots[static_cast<EZ::u32>(tag)] = {};
		}

		void ClearAll()
		{
			for (auto& slot : m_Slots)
			{
				slot = {};
			}
		}

	private:
		std::array<RenderImageSlot, SlotCount> m_Slots{};
	};
}

#endif
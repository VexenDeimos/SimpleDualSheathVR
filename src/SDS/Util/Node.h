#pragma once

#include "PCH.h"

namespace SDS::Util::Node
{
	[[nodiscard]] RE::NiAVObject* GetNiObject(
		RE::NiNode* a_root,
		const RE::BSFixedString& a_name);

	void AttachToNode(
		RE::NiAVObject* a_object,
		RE::NiNode* a_node);
}
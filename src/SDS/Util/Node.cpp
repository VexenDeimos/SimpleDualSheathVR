#include "PCH.h"

#include "SDS/Util/Node.h"

namespace SDS::Util::Node
{
	RE::NiAVObject* GetNiObject(
		RE::NiNode* a_root,
		const RE::BSFixedString& a_name)
	{
		if (!a_root) {
			return nullptr;
		}

		return a_root->GetObjectByName(a_name);
	}

	void AttachToNode(
		RE::NiAVObject* a_object,
		RE::NiNode* a_node)
	{
		if (!a_object || !a_node) {
			return;
		}

		if (a_object->parent != a_node) {
			a_node->AttachChild(a_object, true);
		}
	}
}
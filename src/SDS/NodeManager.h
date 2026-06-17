#pragma once

#include "PCH.h"

namespace SDS::NodeManager
{
	RE::NiAVObject* FindObject(RE::NiNode* a_root, const char* a_nodeName);
	RE::NiNode* FindNode(RE::NiNode* a_root, const char* a_nodeName);

	void ProbeNode(RE::NiNode* a_root, const char* a_rootName, const char* a_nodeName);
	bool EnsureChildNode(RE::NiNode* a_parent, const char* a_nodeName);
	void EnsureFallbackSDSNodes(RE::NiNode* a_root, const char* a_rootLabel);

	void RunPlayerNodeProbe(const char* a_reason);
}
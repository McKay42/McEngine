//================ Copyright (c) 2020, PG, All rights reserved. =================//
//
// Purpose:		real-time hierarchical profiling
//
// $NoKeywords: $vprof
//===============================================================================//

#include "Profiler.h"

#include "Engine.h"
#include "ConVar.h"

#include <string.h>

ProfilerProfile g_profCurrentProfile(true);

ProfilerProfile::ProfilerProfile(bool manualStartViaMain) : m_root("Root", VPROF_BUDGETGROUP_ROOT, NULL)
{
	m_bManualStartViaMain = manualStartViaMain;

	m_iEnabled = 0;
	m_bEnableScheduled = false;
	m_bAtRoot = true;
	m_curNode = &m_root;

	m_iNumGroups = 0;
	for (size_t i=0; i<VPROF_MAX_NUM_BUDGETGROUPS; i++)
	{
		m_groups[i].name = NULL;
	}

	m_iNumNodes = 0;

	// create all groups in predefined order
	groupNameToID(VPROF_BUDGETGROUP_ROOT); // NOTE: the root group must always be the first group to be created here
	groupNameToID(VPROF_BUDGETGROUP_SLEEP);
	groupNameToID(VPROF_BUDGETGROUP_WNDPROC);
	groupNameToID(VPROF_BUDGETGROUP_UPDATE);
	groupNameToID(VPROF_BUDGETGROUP_DRAW);
	groupNameToID(VPROF_BUDGETGROUP_DRAW_SWAPBUFFERS);
}

double ProfilerProfile::sumTimes(int groupID)
{
	return sumTimes(&m_root, groupID);
}

double ProfilerProfile::sumTimes(ProfilerNode *node, int groupID)
{
	if (node == NULL) return 0.0;

	double sum = 0.0;

	ProfilerNode *sibling = node;
	while (sibling != NULL)
	{
		if (sibling->m_iGroupID == groupID)
		{
			if (sibling == &m_root)
				return (m_root.m_child != NULL ? m_root.m_child->m_fTimeLastFrame : m_root.m_fTimeLastFrame); // special case: early return for the root group (total duration of the entire frame)
			else
				return (sum + sibling->m_fTimeLastFrame);
		}
		else
		{
			if (sibling->m_child != NULL)
				sum += sumTimes(sibling->m_child, groupID);
		}

		sibling = sibling->m_sibling;
	}

	return sum;
}

int ProfilerProfile::groupNameToID(const char *group)
{
	if (m_iNumGroups >= VPROF_MAX_NUM_BUDGETGROUPS)
	{
		engine->showMessageErrorFatal("Engine", "Increase VPROF_MAX_NUM_BUDGETGROUPS");
		engine->shutdown();
		return -1;
	}

	for (int i=0; i<m_iNumGroups; i++)
	{
		if (m_groups[i].name != NULL && group != NULL && strcmp(m_groups[i].name, group) == 0)
			return i;
	}

	const int newID = m_iNumGroups;
	m_groups[m_iNumGroups++].name = group;
	return newID;
}



int ProfilerNode::s_iNodeCounter = 0;

ProfilerNode::ProfilerNode() : ProfilerNode(NULL, NULL, NULL) {}

ProfilerNode::ProfilerNode(const char *name, const char *group, ProfilerNode *parent)
{
	constructor(name, group, parent);
}

void ProfilerNode::constructor(const char *name, const char *group, ProfilerNode *parent)
{
	m_name = name;
	m_parent = parent;
	m_child = NULL;
	m_sibling = NULL;

	m_iNumRecursions = 0;
	m_fTime = 0.0;
	m_fTimeCurrentFrame = 0.0;
	m_fTimeLastFrame = 0.0;

	m_iGroupID = (s_iNodeCounter++ > 0 ? g_profCurrentProfile.groupNameToID(group) : 0);
}

void ProfilerNode::enterScope()
{
	if (m_iNumRecursions++ == 0)
	{
		m_fTime = engine->getTimeReal();
	}
}

bool ProfilerNode::exitScope()
{
	if (--m_iNumRecursions == 0)
	{
		m_fTime = engine->getTimeReal() - m_fTime;
		m_fTimeCurrentFrame = m_fTime;
	}

	return (m_iNumRecursions == 0);
}

ProfilerNode *ProfilerNode::getSubNode(const char *name, const char *group)
{
	// find existing node
	ProfilerNode *child = m_child;
	while (child != NULL)
	{
		if (child->m_name == name) // NOTE: pointer comparison
			return child;

		child = child->m_sibling;
	}

	// "add" new node
	if (g_profCurrentProfile.m_iNumNodes >= VPROF_MAX_NUM_NODES)
	{
		engine->showMessageErrorFatal("Engine", "Increase VPROF_MAX_NUM_NODES");
		engine->shutdown();
		return NULL;
	}

	ProfilerNode *node = &(g_profCurrentProfile.m_nodes[g_profCurrentProfile.m_iNumNodes++]);
	node->constructor(name, group, this);
	node->m_sibling = m_child;
	m_child = node;

	return node;
}



//************************//
//	Profiler ConCommands  //
//************************//

void _vprof(UString oldValue, UString newValue)
{
	const bool enable = (newValue.toFloat() > 0.0f);

	if (enable != g_profCurrentProfile.isEnabled())
	{
		if (enable)
			g_profCurrentProfile.start();
		else
			g_profCurrentProfile.stop();
	}
}

ConVar _vprof_("vprof", false, "enables/disables the visual profiler", _vprof);

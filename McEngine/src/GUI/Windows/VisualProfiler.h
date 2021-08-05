//================ Copyright (c) 2020, PG, All rights reserved. =================//
//
// Purpose:		vprof gui overlay
//
// $NoKeywords: $vprof
//===============================================================================//

#ifndef VISUALPROFILER_H
#define VISUALPROFILER_H

#include "CBaseUIElement.h"

class ConVar;
class ProfilerNode;
class ProfilerProfile;

class McFont;
class VertexArrayObject;

class VisualProfiler : public CBaseUIElement
{
public:
	VisualProfiler();
	virtual ~VisualProfiler();

	void draw(Graphics *g);
	void update();

	void setProfile(ProfilerProfile *profile);

private:
	static ConVar *m_vprof_ref;

	struct NODE
	{
		const ProfilerNode *node;
		int depth;
	};

	struct SPIKE
	{
		NODE node;
		double timeLastFrame;
		uint32_t id;
	};

	struct GROUP
	{
		const char *name;
		int id;
		Color color;
	};

	void drawStringWithShadow(Graphics *g, const UString string, Color color);

	static void collectProfilerNodesRecursive(const ProfilerNode *node, int depth, std::vector<NODE> &nodes, SPIKE &spike);
	static void collectProfilerNodesSpikeRecursive(const ProfilerNode *node, int depth, std::vector<SPIKE> &spikeNodes);

	static int getGraphWidth();
	static int getGraphHeight();

	int m_iPrevVaoWidth;
	int m_iPrevVaoHeight;
	int m_iPrevVaoGroups;
	float m_fPrevVaoMaxRange;
	float m_fPrevVaoAlpha;

	int m_iCurLinePos;

	int m_iDrawGroupID;
	int m_iDrawSwapBuffersGroupID;

	ProfilerProfile *m_profile;
	std::vector<GROUP> m_groups;
	std::vector<NODE> m_nodes;
	std::vector<SPIKE> m_spikes;

	SPIKE m_spike;
	std::vector<SPIKE> m_spikeNodes;
	uint32_t m_spikeIDCounter;

	McFont *m_font;
	VertexArrayObject *m_lineVao;

	bool m_bScheduledForceRebuildLineVao;
};

#endif

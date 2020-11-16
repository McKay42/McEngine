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

class McFont;
class VertexArrayObject;

class VisualProfiler : public CBaseUIElement
{
public:
	VisualProfiler();
	virtual ~VisualProfiler();

	void draw(Graphics *g);
	void update();

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
	};

	struct GROUP
	{
		const char *name;
		int id;
		Color color;
	};

	void drawStringWithShadow(Graphics *g, const UString string, Color color);
	void collectProfilerNodesRecursive(const ProfilerNode *node, int depth, std::vector<NODE> &nodes, SPIKE &spike);
	void collectProfilerNodesSpikeRecursive(const ProfilerNode *node, int depth, std::vector<SPIKE> &spikeNodes);

	int getGraphWidth();
	int getGraphHeight();

	int m_iPrevVaoWidth;
	int m_iPrevVaoHeight;
	int m_iPrevVaoGroups;
	float m_fPrevVaoMaxRange;
	float m_fPrevVaoAlpha;

	int m_iCurLinePos;

	std::vector<GROUP> m_groups;
	std::vector<NODE> m_nodes;
	std::vector<SPIKE> m_spikes;

	SPIKE m_spike;
	std::vector<SPIKE> m_spikeNodes;

	McFont *m_font;
	VertexArrayObject *m_lineVao;
};

#endif

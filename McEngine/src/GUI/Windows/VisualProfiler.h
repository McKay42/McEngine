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

	virtual void draw(Graphics *g);
	virtual void update();

	void incrementInfoBladeDisplayMode();
	void decrementInfoBladeDisplayMode();

	void addInfoBladeAppTextLine(const UString &text);

	void setProfile(ProfilerProfile *profile);
	void setRequiresAltShiftKeysToFreeze(bool requiresAltShiftKeysToFreeze) {m_bRequiresAltShiftKeysToFreeze = requiresAltShiftKeysToFreeze;}

	virtual bool isEnabled();

private:
	enum INFO_BLADE_DISPLAY_MODE
	{
		INFO_BLADE_DISPLAY_MODE_DEFAULT = 0,

		INFO_BLADE_DISPLAY_MODE_GPU_INFO = 1,
		INFO_BLADE_DISPLAY_MODE_ENGINE_INFO = 2,
		INFO_BLADE_DISPLAY_MODE_APP_INFO = 3,

		INFO_BLADE_DISPLAY_MODE_COUNT = 4
	};

	struct TEXT_LINE
	{
		UString text;
		int width;
	};

private:
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

private:
	static ConVar *m_vprof_ref;

	static void collectProfilerNodesRecursive(const ProfilerNode *node, int depth, std::vector<NODE> &nodes, SPIKE &spike);
	static void collectProfilerNodesSpikeRecursive(const ProfilerNode *node, int depth, std::vector<SPIKE> &spikeNodes);

	static int getGraphWidth();
	static int getGraphHeight();

	static void addTextLine(const UString &text, McFont *font, std::vector<TEXT_LINE> &textLines);

	static void drawStringWithShadow(Graphics *g, McFont *font, const UString &string, Color color);

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
	McFont *m_fontConsole;
	VertexArrayObject *m_lineVao;

	bool m_bScheduledForceRebuildLineVao;
	bool m_bRequiresAltShiftKeysToFreeze;

	std::vector<TEXT_LINE> m_textLines;
	std::vector<UString> m_appTextLines;
};

extern VisualProfiler *vprof;

#endif

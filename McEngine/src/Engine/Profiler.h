//================ Copyright (c) 2020, PG, All rights reserved. =================//
//
// Purpose:		real-time hierarchical profiling
//
// $NoKeywords: $vprof
//===============================================================================//

#ifndef PROFILER_H
#define PROFILER_H

#define VPROF_MAIN()						g_profCurrentProfile.main(); VPROF("Main")

#define VPROF(name)							VPROF_(name, VPROF_BUDGETGROUP_ROOT)
#define VPROF_(name, group)					ProfilerScope Prof_(name, group);

#define VPROF_BUDGET(name, group)			VPROF_(name, group)

#define VPROF_SCOPE_BEGIN(name)				do { VPROF(name)
#define VPROF_SCOPE_END()					} while (0)

#define VPROF_ENTER_SCOPE(name)				g_profCurrentProfile.enterScope(name, VPROF_BUDGETGROUP_ROOT)
#define VPROF_EXIT_SCOPE()					g_profCurrentProfile.exitScope()

#define VPROF_BUDGETGROUP_ROOT				"Root"
#define VPROF_BUDGETGROUP_SLEEP				"Sleep"
#define VPROF_BUDGETGROUP_WNDPROC			"WndProc"
#define VPROF_BUDGETGROUP_UPDATE			"Update"
#define VPROF_BUDGETGROUP_DRAW				"Draw"
#define VPROF_BUDGETGROUP_DRAW_SWAPBUFFERS	"SwapBuffers"

#define VPROF_MAX_NUM_BUDGETGROUPS			32
#define VPROF_MAX_NUM_NODES					32



class ProfilerNode
{
	friend class ProfilerProfile;

public:
	ProfilerNode();
	ProfilerNode(const char *name, const char *group, ProfilerNode *parent);

	void enterScope();
	bool exitScope();

	inline const char *getName() const {return m_name;}
	inline int getGroupID() const {return m_iGroupID;}

	inline ProfilerNode *getParent() const {return m_parent;}
	inline ProfilerNode *getChild() const {return m_child;}
	inline ProfilerNode *getSibling() const {return m_sibling;}

	inline double getTimeCurrentFrame() const {return m_fTimeCurrentFrame;} // NOTE: this is incomplete if retrieved within engine update(), use getTimeLastFrame() instead
	inline double getTimeLastFrame() const {return m_fTimeLastFrame;}

private:
	inline void constructor(const char *name, const char *group, ProfilerNode *parent);

	ProfilerNode *getSubNode(const char *name, const char *group);

	static int s_iNodeCounter;

	const char *m_name;
	int m_iGroupID;

	ProfilerNode *m_parent;
	ProfilerNode *m_child;
	ProfilerNode *m_sibling;

	int m_iNumRecursions;
	double m_fTime;
	double m_fTimeCurrentFrame;
	double m_fTimeLastFrame;
};



class ProfilerProfile
{
	friend class ProfilerNode;

public:
	ProfilerProfile(bool manualStartViaMain = false);

	inline void main()
	{
		if (m_bEnableScheduled)
		{
			m_bEnableScheduled = false;
			m_root.enterScope();
		}

		// collect all durations from the last frame and store them as a complete set
		if (m_iEnabled > 0)
		{
			for (int i=0; i<m_iNumNodes; i++)
			{
				m_nodes[i].m_fTimeLastFrame = m_nodes[i].m_fTimeCurrentFrame;
			}
		}
	}

	inline void start()
	{
		if (++m_iEnabled == 1)
		{
			if (m_bManualStartViaMain)
				m_bEnableScheduled = true;
			else
				m_root.enterScope();
		}
	}

	inline void stop()
	{
		if (--m_iEnabled == 0)
		{
			if (!m_bEnableScheduled)
				m_root.exitScope();

			m_bEnableScheduled = false;
		}
	}

	inline void enterScope(const char *name, const char *group)
	{
		if ((m_iEnabled != 0 && !m_bEnableScheduled) || !m_bAtRoot)
		{
			if (name != m_curNode->m_name) // NOTE: pointer comparison
				m_curNode = m_curNode->getSubNode(name, group);

			m_curNode->enterScope();

			m_bAtRoot = (m_curNode == &m_root);
		}
	}

	inline void exitScope()
	{
		if (!m_bAtRoot || (m_iEnabled != 0 && !m_bEnableScheduled))
		{
			if (!m_bAtRoot && m_curNode->exitScope())
				m_curNode = m_curNode->m_parent;

			m_bAtRoot = (m_curNode == &m_root);
		}
	}

	inline bool isEnabled() const {return (m_iEnabled != 0 || m_bEnableScheduled);}
	inline bool isAtRoot() const {return m_bAtRoot;}

	inline int getNumGroups() const {return m_iNumGroups;}
	inline int getNumNodes() const {return m_iNumNodes;}

	inline const ProfilerNode *getRoot() const {return &m_root;}

	inline const char *getGroupName(int groupID)
	{
		return m_groups[groupID < 0 ? 0 : (groupID > m_iNumGroups - 1 ? m_iNumGroups - 1 : groupID)].name;
	}

	double sumTimes(int groupID);
	double sumTimes(ProfilerNode *node, int groupID);

private:
	struct BUDGETGROUP
	{
		const char *name;
	};

	int groupNameToID(const char *group);

	int m_iNumGroups;
	BUDGETGROUP m_groups[VPROF_MAX_NUM_BUDGETGROUPS];

	bool m_bManualStartViaMain;

	int m_iEnabled;
	bool m_bEnableScheduled;
	bool m_bAtRoot;
	ProfilerNode m_root;
	ProfilerNode *m_curNode;

	int m_iNumNodes;
	ProfilerNode m_nodes[VPROF_MAX_NUM_NODES];
};

extern ProfilerProfile g_profCurrentProfile;



class ProfilerScope
{
public:
	inline ProfilerScope(const char *name, const char *group) {g_profCurrentProfile.enterScope(name, group);}
	inline ~ProfilerScope() {g_profCurrentProfile.exitScope();}
};

#endif

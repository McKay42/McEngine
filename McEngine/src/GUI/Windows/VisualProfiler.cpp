//================ Copyright (c) 2020, PG, All rights reserved. =================//
//
// Purpose:		vprof gui overlay
//
// $NoKeywords: $vprof
//===============================================================================//

#include "VisualProfiler.h"

#include "Engine.h"
#include "ConVar.h"
#include "Profiler.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "Environment.h"
#include "ResourceManager.h"
#include "AnimationHandler.h"
#include "SoundEngine.h"

#include <string.h>

ConVar vprof_graph("vprof_graph", true, FCVAR_NONE, "whether to draw the graph when the overlay is enabled");
ConVar vprof_graph_height("vprof_graph_height", 250.0f, FCVAR_NONE);
ConVar vprof_graph_width("vprof_graph_width", 800.0f, FCVAR_NONE);
ConVar vprof_graph_margin("vprof_graph_margin", 40.0f, FCVAR_NONE);
ConVar vprof_graph_range_max("vprof_graph_range_max", 16.666666f, FCVAR_NONE, "max value of the y-axis in milliseconds");
ConVar vprof_graph_alpha("vprof_graph_alpha", 1.0f, FCVAR_NONE, "line opacity");
ConVar vprof_graph_draw_overhead("vprof_graph_draw_overhead", false, FCVAR_NONE, "whether to draw the profiling overhead time in white (usually negligible)");

ConVar vprof_spike("vprof_spike", 0, FCVAR_NONE, "measure and display largest spike details (1 = small info, 2 = extended info)");

ConVar vprof_display_mode("vprof_display_mode", 0, FCVAR_NONE, "which info blade to show on the top right (gpu/engine/app/etc. info), use CTRL + TAB to cycle through, 0 = disabled");

ConVar debug_vprof("debug_vprof", false, FCVAR_NONE);

ConVar *VisualProfiler::m_vprof_ref = NULL;

VisualProfiler *vprof = NULL;

VisualProfiler::VisualProfiler() : CBaseUIElement(0, 0, 0, 0, "")
{
	vprof = this;

	if (m_vprof_ref == NULL)
		m_vprof_ref = convar->getConVarByName("vprof");

	m_spike.node.depth = -1;
	m_spike.node.node = NULL;
	m_spike.timeLastFrame = 0.0;

	m_iPrevVaoWidth = 0;
	m_iPrevVaoHeight = 0;
	m_iPrevVaoGroups = 0;
	m_fPrevVaoMaxRange = 0.0f;
	m_fPrevVaoAlpha = -1.0f;

	m_iCurLinePos = 0;

	m_iDrawGroupID = -1;
	m_iDrawSwapBuffersGroupID = -1;

	m_spikeIDCounter = 0;

	setProfile(&g_profCurrentProfile); // by default we look at the standard full engine-wide profile

	m_font = engine->getResourceManager()->getFont("FONT_DEFAULT");
	m_fontConsole = engine->getResourceManager()->getFont("FONT_CONSOLE");
	m_lineVao = engine->getResourceManager()->createVertexArrayObject(Graphics::PRIMITIVE::PRIMITIVE_LINES, Graphics::USAGE_TYPE::USAGE_DYNAMIC, true);

	m_bScheduledForceRebuildLineVao = false;
	m_bRequiresAltShiftKeysToFreeze = false;
}

VisualProfiler::~VisualProfiler()
{
	vprof = NULL;
}

void VisualProfiler::draw(Graphics *g)
{
	VPROF_BUDGET("VisualProfiler::draw", VPROF_BUDGETGROUP_DRAW);
	if (!m_vprof_ref->getBool() || !m_bVisible) return;

	// draw miscellaneous debug infos
	const int displayMode = vprof_display_mode.getInt();
	if (displayMode != INFO_BLADE_DISPLAY_MODE::INFO_BLADE_DISPLAY_MODE_DEFAULT)
	{
		McFont *textFont = m_font;
		float textScale = 1.0f;
		{
			switch (displayMode)
			{
			case INFO_BLADE_DISPLAY_MODE::INFO_BLADE_DISPLAY_MODE_GPU_INFO:
				{
					const int vramTotalMB = g->getVRAMTotal() / 1024;
					const int vramRemainingMB = g->getVRAMRemaining() / 1024;

					UString vendor = g->getVendor();
					UString model = g->getModel();
					UString version = g->getVersion();

					UString line1 = "GPU Vendor: "; line1.append(vendor);
					addTextLine(line1, textFont, m_textLines);
					UString line2 = "Model: "; line2.append(model);
					addTextLine(line2, textFont, m_textLines);
					UString line3 = "Version: "; line3.append(version);
					addTextLine(line3, textFont, m_textLines);
					addTextLine(UString::format("Resolution: %i x %i", (int)g->getResolution().x, (int)g->getResolution().y), textFont, m_textLines);
					addTextLine(UString::format("NativeRes: %i x %i", (int)env->getNativeScreenSize().x, (int)env->getNativeScreenSize().y), textFont, m_textLines);
					addTextLine(UString::format("Env DPI Scale: %f", env->getDPIScale()), textFont, m_textLines);
					addTextLine(UString::format("Env DPI: %i", (int)env->getDPI()), textFont, m_textLines);
					//addTextLine(UString::format("Renderer: %s", typeid(g).name()), textFont, m_textLines); // TODO: add g->getName() or something
					addTextLine(UString::format("VRAM: %i MB / %i MB", vramRemainingMB, vramTotalMB), textFont, m_textLines);
				}
				break;

			case INFO_BLADE_DISPLAY_MODE::INFO_BLADE_DISPLAY_MODE_ENGINE_INFO:
				{
					textFont = m_fontConsole;
					textScale = std::round(env->getDPIScale() + 0.255f);

					const double time = engine->getTime();
					const double timeRunning = engine->getTimeRunning();
					const double dilation = (timeRunning - time);
					const Vector2 envMousePos = env->getMousePos();

					addTextLine(UString::format("ConVars: %zu", convar->getConVarArray().size()), textFont, m_textLines);
					addTextLine(UString::format("Monitor: [%i] of %zu", env->getMonitor(), env->getMonitors().size()), textFont, m_textLines);
					addTextLine(UString::format("Env Mouse Pos: %i x %i", (int)envMousePos.x, (int)envMousePos.y), textFont, m_textLines);
					addTextLine(UString::format("Sound Device: %s", engine->getSound()->getOutputDevice().toUtf8()), textFont, m_textLines);
					addTextLine(UString::format("Sound Volume: %f", engine->getSound()->getVolume()), textFont, m_textLines);
					addTextLine(UString::format("RM Threads: %zu", engine->getResourceManager()->getNumThreads()), textFont, m_textLines);
					addTextLine(UString::format("RM LoadingWork: %zu", engine->getResourceManager()->getNumLoadingWork()), textFont, m_textLines);
					addTextLine(UString::format("RM LoadingWorkAD: %zu", engine->getResourceManager()->getNumLoadingWorkAsyncDestroy()), textFont, m_textLines);
					addTextLine(UString::format("RM Named Resources: %zu", engine->getResourceManager()->getResources().size()), textFont, m_textLines);
					addTextLine(UString::format("Animations: %zu", anim->getNumActiveAnimations()), textFont, m_textLines);
					addTextLine(UString::format("Frame: %lu", engine->getFrameCount()), textFont, m_textLines);
					addTextLine(UString::format("Time: %f", time), textFont, m_textLines);
					addTextLine(UString::format("Realtime: %f", timeRunning), textFont, m_textLines);
					addTextLine(UString::format("Time Dilation: %f", dilation), textFont, m_textLines);

					for (size_t i=0; i<m_engineTextLines.size(); i++)
					{
						addTextLine(m_engineTextLines[i], textFont, m_textLines);
					}
				}
				break;

			case INFO_BLADE_DISPLAY_MODE::INFO_BLADE_DISPLAY_MODE_APP_INFO:
				{
					textFont = m_fontConsole;
					textScale = std::round(env->getDPIScale() + 0.255f);

					for (size_t i=0; i<m_appTextLines.size(); i++)
					{
						addTextLine(m_appTextLines[i], textFont, m_textLines);
					}

					if (m_appTextLines.size() < 1)
						addTextLine("(Empty)", textFont, m_textLines);
				}
				break;
			}
		}

		if (m_textLines.size() > 0)
		{
			const Color textColor = 0xffffffff;
			const int margin = vprof_graph_margin.getFloat() * env->getDPIScale();
			const int marginBox = 6 * env->getDPIScale();

			int largestLineWidth = 0;
			for (const TEXT_LINE &textLine : m_textLines)
			{
				if (textLine.width > largestLineWidth)
					largestLineWidth = textLine.width;
			}
			largestLineWidth *= textScale;

			const int boxX = -margin + (engine->getScreenWidth() - largestLineWidth) - marginBox;
			const int boxY = margin - marginBox;
			const int boxWidth = largestLineWidth + 2*marginBox;
			const int boxHeight = marginBox*2 + textFont->getHeight()*textScale + (textFont->getHeight()*textScale*1.5f)*(m_textLines.size() - 1);

			// draw background
			g->setColor(0xaa000000);
			g->fillRect(boxX - 1, boxY - 1, boxWidth + 1, boxHeight + 1);
			g->setColor(0xff777777);
			g->drawRect(boxX - 1, boxY - 1, boxWidth + 1, boxHeight + 1);

			// draw text
			g->pushTransform();
			{
				g->scale(textScale, textScale);
				g->translate(-margin, (int)(textFont->getHeight()*textScale + margin));

				for (size_t i=0; i<m_textLines.size(); i++)
				{
					if (i > 0)
						g->translate(0, (int)(textFont->getHeight()*textScale*1.5f));

					g->pushTransform();
					{
						g->translate(engine->getScreenWidth() - m_textLines[i].width*textScale, 0);
						drawStringWithShadow(g, textFont, m_textLines[i].text, textColor);
					}
					g->popTransform();
				}
			}
			g->popTransform();
		}

		m_textLines.clear();
		m_engineTextLines.clear();
		m_appTextLines.clear();
	}

	// draw profiler node tree extended details
	if (debug_vprof.getBool())
	{
		VPROF_BUDGET("DebugText", VPROF_BUDGETGROUP_DRAW);

		g->setColor(0xffcccccc);
		g->pushTransform();
		{
			g->translate(0, m_font->getHeight());

			g->drawString(m_font, UString::format("%i nodes", m_profile->getNumNodes()));
			g->translate(0, m_font->getHeight()*1.5f);

			g->drawString(m_font, UString::format("%i groups", m_profile->getNumGroups()));
			g->translate(0, m_font->getHeight()*1.5f);

			g->drawString(m_font, "----------------------------------------------------");
			g->translate(0, m_font->getHeight()*1.5f);

			for (int i=m_nodes.size()-1; i>=0; i--)
			{
				g->pushTransform();
				{
					g->translate(m_font->getHeight()*3*(m_nodes[i].depth - 1), 0);
					g->drawString(m_font, UString::format("[%s] - %s = %f ms", m_nodes[i].node->getName(), m_profile->getGroupName(m_nodes[i].node->getGroupID()), m_nodes[i].node->getTimeLastFrame() * 1000.0));
				}
				g->popTransform();

				g->translate(0, m_font->getHeight()*1.5f);
			}

			g->drawString(m_font, "----------------------------------------------------");
			g->translate(0, m_font->getHeight()*1.5f);

			for (int i=0; i<m_profile->getNumGroups(); i++)
			{
				const char *groupName = m_profile->getGroupName(i);
				const double sum = m_profile->sumTimes(i);

				g->drawString(m_font, UString::format("%s = %f ms", groupName, sum*1000.0));
				g->translate(0, m_font->getHeight()*1.5f);
			}
		}
		g->popTransform();
	}

	// draw extended spike details tree (profiler node snapshot)
	if (vprof_spike.getBool() && !debug_vprof.getBool())
	{
		if (m_spike.node.node != NULL)
		{
			if (vprof_spike.getInt() == 2)
			{
				VPROF_BUDGET("DebugText", VPROF_BUDGETGROUP_DRAW);

				g->setColor(0xffcccccc);
				g->pushTransform();
				{
					g->translate(0, m_font->getHeight());

					for (int i=m_spikeNodes.size()-1; i>=0; i--)
					{
						g->pushTransform();
						{
							g->translate(m_font->getHeight()*3*(m_spikeNodes[i].node.depth - 1), 0);
							g->drawString(m_font, UString::format("[%s] - %s = %f ms", m_spikeNodes[i].node.node->getName(), m_profile->getGroupName(m_spikeNodes[i].node.node->getGroupID()), m_spikeNodes[i].timeLastFrame * 1000.0));
						}
						g->popTransform();

						g->translate(0, m_font->getHeight()*1.5f);
					}
				}
				g->popTransform();
			}
		}
	}

	// draw graph
	if (vprof_graph.getBool())
	{
		VPROF_BUDGET("LineGraph", VPROF_BUDGETGROUP_DRAW);

		const int width = getGraphWidth();
		const int height = getGraphHeight();
		const int margin = vprof_graph_margin.getFloat() * env->getDPIScale();

		const int xPos = engine->getScreenWidth() - width - margin;
		const int yPos = engine->getScreenHeight() - height - margin + (engine->getMouse()->isMiddleDown() ? engine->getMouse()->getPos().y - engine->getScreenHeight() : 0);

		// draw background
		g->setColor(0xaa000000);
		g->fillRect(xPos - 1, yPos - 1, width + 1, height + 1);
		g->setColor(0xff777777);
		g->drawRect(xPos - 1, yPos - 1, width + 1, height + 1);

		// draw lines
		g->setColor(0xff00aa00);
		g->pushTransform();
		{
			const int stride = 2*m_iPrevVaoGroups;

			// behind
			m_lineVao->setDrawRange(m_iCurLinePos*stride, m_iPrevVaoWidth*stride);
			g->translate(xPos + 1 - m_iCurLinePos, yPos + height);
			g->drawVAO(m_lineVao);

			// forward
			m_lineVao->setDrawRange(0, m_iCurLinePos*stride);
			g->translate(m_iPrevVaoWidth, 0);
			g->drawVAO(m_lineVao);
		}
		g->popTransform();

		// draw labels
		if (engine->getKeyboard()->isControlDown())
		{
			const int margin = 3 * env->getDPIScale();

			// y-axis range
			g->pushTransform();
			{
				g->translate((int)(xPos + margin), (int)(yPos + m_font->getHeight() + margin));
				drawStringWithShadow(g, m_font, UString::format("%g ms", vprof_graph_range_max.getFloat()), 0xffffffff);

				g->translate(0, (int)(height - m_font->getHeight() - 2*margin));
				drawStringWithShadow(g, m_font, "0 ms", 0xffffffff);
			}
			g->popTransform();

			// colored group names
			g->pushTransform();
			{
				const int padding = 6 * env->getDPIScale();

				g->translate((int)(xPos - 3*margin), (int)(yPos + height - padding));

				for (size_t i=1; i<m_groups.size(); i++)
				{
					const int stringWidth = (int)(m_font->getStringWidth(m_groups[i].name));
					g->translate(-stringWidth, 0);
					drawStringWithShadow(g, m_font, m_groups[i].name, m_groups[i].color);
					g->translate(stringWidth, (int)(-m_font->getHeight() - padding));
				}
			}
			g->popTransform();
		}

		// draw top spike text above graph
		if (vprof_spike.getBool() && !debug_vprof.getBool())
		{
			if (m_spike.node.node != NULL)
			{
				if (vprof_spike.getInt() == 1)
				{
					const int margin = 6 * env->getDPIScale();

					g->setColor(0xffcccccc);
					g->pushTransform();
					{
						g->translate((int)(xPos + margin), (int)(yPos - 2*margin));
						///drawStringWithShadow(g, UString::format("[%s] = %g ms", m_spike.node.node->getName(), m_spike.timeLastFrame * 1000.0), m_groups[m_spike.node.node->getGroupID()].color);
						g->drawString(m_font, UString::format("Spike = %g ms", m_spike.timeLastFrame * 1000.0));
					}
					g->popTransform();
				}
			}
		}
	}
}

void VisualProfiler::drawStringWithShadow(Graphics *g, McFont *font, const UString &string, Color color)
{
	if (font == NULL) return;

	const int shadowOffset = 1 * env->getDPIScale();

	g->translate(shadowOffset, shadowOffset);
	{
		g->setColor(0xff000000);
		g->drawString(font, string);
		g->translate(-shadowOffset, -shadowOffset);
		{
			g->setColor(color);
			g->drawString(font, string);
		}
	}
}

void VisualProfiler::update()
{
	VPROF_BUDGET("VisualProfiler::update", VPROF_BUDGETGROUP_UPDATE);
	CBaseUIElement::update();
	if (!m_vprof_ref->getBool() || !m_bVisible) return;

	const bool isFrozen = (engine->getKeyboard()->isShiftDown() && (!m_bRequiresAltShiftKeysToFreeze || engine->getKeyboard()->isAltDown()));

	if (debug_vprof.getBool() || vprof_spike.getBool())
	{
		if (!isFrozen)
		{
			SPIKE spike;
			{
				spike.node.depth = -1;
				spike.node.node = NULL;
				spike.timeLastFrame = 0.0;
				spike.id = m_spikeIDCounter++;
			}



			// run regular debug node collector
			m_nodes.clear();
			collectProfilerNodesRecursive(m_profile->getRoot(), 0, m_nodes, spike);



			// run spike collector and updater
			if (vprof_spike.getBool())
			{
				const int graphWidth = getGraphWidth();

				m_spikes.push_back(spike);

				if (m_spikes.size() > graphWidth)
					m_spikes.erase(m_spikes.begin());

				SPIKE &newSpike = m_spikes[0];

				for (size_t i=0; i<m_spikes.size(); i++)
				{
					if (m_spikes[i].timeLastFrame > newSpike.timeLastFrame)
						newSpike = m_spikes[i];
				}

				if (newSpike.id != m_spike.id)
				{
					const bool isNewSpikeLarger = (newSpike.timeLastFrame > m_spike.timeLastFrame);

					m_spike = newSpike;



					// NOTE: since we only store 1 spike snapshot, once that is erased (m_spikes.size() > graphWidth) and we have to "fall back" to a "lower" spike, we don't have any data on that lower spike anymore
					// so, we simply only create a new snapshot if we have a new larger spike (since that is guaranteed to be the currently active one, i.e. going through node data in m_profile will return its data)
					// (storing graphWidth amounts of snapshots seems unnecessarily wasteful, and I like this solution)
					if (isNewSpikeLarger)
					{
						m_spikeNodes.clear();
						collectProfilerNodesSpikeRecursive(m_spike.node.node, 1, m_spikeNodes);
					}
				}
			}
		}
	}

	// lazy rebuild group/color list
	if (m_groups.size() < m_profile->getNumGroups())
	{
		// reset
		m_iDrawGroupID = -1;
		m_iDrawSwapBuffersGroupID = -1;

		const int curNumGroups = m_groups.size();
		const int actualNumGroups = m_profile->getNumGroups();

		for (int i=curNumGroups; i<actualNumGroups; i++)
		{
			GROUP group;

			group.name = m_profile->getGroupName(i);
			group.id = i;

			// hardcoded colors for some groups
			if (strcmp(group.name, VPROF_BUDGETGROUP_ROOT) == 0) // NOTE: VPROF_BUDGETGROUP_ROOT is used for drawing the profiling overhead time if vprof_graph_draw_overhead is enabled
				group.color = 0xffffffff;
			else if (strcmp(group.name, VPROF_BUDGETGROUP_SLEEP) == 0)
				group.color = 0xff5555bb;
			else if (strcmp(group.name, VPROF_BUDGETGROUP_WNDPROC) == 0)
				group.color = 0xffffff00;
			else if (strcmp(group.name, VPROF_BUDGETGROUP_UPDATE) == 0)
				group.color = 0xff00bb00;
			else if (strcmp(group.name, VPROF_BUDGETGROUP_DRAW) == 0)
			{
				group.color = 0xffbf6500;
				m_iDrawGroupID = group.id;
			}
			else if (strcmp(group.name, VPROF_BUDGETGROUP_DRAW_SWAPBUFFERS) == 0)
			{
				group.color = 0xffff0000;
				m_iDrawSwapBuffersGroupID = group.id;
			}
			else
				group.color = 0xff00ffff; // default to turquoise

			m_groups.push_back(group);
		}
	}

	// and handle line updates
	{
		const int numGroups = m_groups.size();
		const int graphWidth = getGraphWidth();
		const int graphHeight = getGraphHeight();
		const float maxRange = vprof_graph_range_max.getFloat();
		const float alpha = vprof_graph_alpha.getFloat();

		// lazy rebuild line vao if parameters change
		if (m_bScheduledForceRebuildLineVao
			|| m_iPrevVaoWidth != graphWidth
			|| m_iPrevVaoHeight != graphHeight
			|| m_iPrevVaoGroups != numGroups
			|| m_fPrevVaoMaxRange != maxRange
			|| m_fPrevVaoAlpha != alpha)
		{
			m_bScheduledForceRebuildLineVao = false;

			m_iPrevVaoWidth = graphWidth;
			m_iPrevVaoHeight = graphHeight;
			m_iPrevVaoGroups = numGroups;
			m_fPrevVaoMaxRange = maxRange;
			m_fPrevVaoAlpha = alpha;

			m_lineVao->release();

			// preallocate 2 vertices per line
			for (int x=0; x<graphWidth; x++)
			{
				for (int g=0; g<numGroups; g++)
				{
					const Color color = COLOR((unsigned char)(m_fPrevVaoAlpha * 255.0f), COLOR_GET_Ri(m_groups[g].color), COLOR_GET_Gi(m_groups[g].color), COLOR_GET_Bi(m_groups[g].color));

					//m_lineVao->addVertex(x, -(((float)graphHeight)/(float)numGroups)*g, 0);
					m_lineVao->addVertex(x, 0, 0);
					m_lineVao->addColor(color);

					//m_lineVao->addVertex(x, -(((float)graphHeight)/(float)numGroups)*(g + 1), 0);
					m_lineVao->addVertex(x, 0, 0);
					m_lineVao->addColor(color);
				}
			}

			// and bake
			engine->getResourceManager()->loadResource(m_lineVao);
		}

		// regular line update
		if (!isFrozen)
		{
			if (m_lineVao->isReady())
			{
				// one new multi-line per frame
				m_iCurLinePos = m_iCurLinePos % graphWidth;

				// if enabled, calculate and draw overhead
				// the overhead is the time spent between not having any profiler node active/alive, and should always be <= 0
				// it is usually slightly negative (in the order of 10 microseconds, includes rounding errors and timer inaccuracies)
				// if the overhead ever gets positive then either there are no nodes covering all paths below VPROF_MAIN(), or there is a serious problem with measuring time via engine->getTimeReal()
				double profilingOverheadTime = 0.0;
				if (vprof_graph_draw_overhead.getBool())
				{
					const int rootGroupID = 0;
					double sumGroupTimes = 0.0;
					{
						for (size_t i=1; i<m_groups.size(); i++) // NOTE: start at 1, ignore rootGroupID
						{
							sumGroupTimes += m_profile->sumTimes(m_groups[i].id);
						}
					}
					profilingOverheadTime = std::max(0.0, m_profile->sumTimes(rootGroupID) - sumGroupTimes);
				}

				// go through every group and build the new multi-line
				int heightCounter = 0;
				for (int i=0; i<numGroups && (size_t)i<m_groups.size(); i++)
				{
					const double rawDuration = (i == 0 ? profilingOverheadTime : m_profile->sumTimes(m_groups[i].id));
					const double duration = (i == m_iDrawGroupID ? rawDuration - m_profile->sumTimes(m_iDrawSwapBuffersGroupID) : rawDuration); // special case: hardcoded fix for nested groups (only draw + swap atm)
					const int lineHeight = (int)(((duration * 1000.0) / (double)maxRange) * (double)graphHeight);

					m_lineVao->setVertex(m_iCurLinePos*numGroups*2 + i*2, m_iCurLinePos, heightCounter);
					m_lineVao->setVertex(m_iCurLinePos*numGroups*2 + i*2 + 1, m_iCurLinePos, heightCounter - lineHeight);

					heightCounter -= lineHeight;
				}

				// re-bake
				engine->getResourceManager()->loadResource(m_lineVao);

				m_iCurLinePos++;
			}
		}
	}
}

void VisualProfiler::incrementInfoBladeDisplayMode()
{

	vprof_display_mode.setValue((vprof_display_mode.getInt() + 1) % INFO_BLADE_DISPLAY_MODE::INFO_BLADE_DISPLAY_MODE_COUNT);
}

void VisualProfiler::decrementInfoBladeDisplayMode()
{
	if (vprof_display_mode.getInt() - 1 < INFO_BLADE_DISPLAY_MODE::INFO_BLADE_DISPLAY_MODE_DEFAULT)
		vprof_display_mode.setValue(INFO_BLADE_DISPLAY_MODE::INFO_BLADE_DISPLAY_MODE_COUNT - 1);
	else
		vprof_display_mode.setValue(vprof_display_mode.getInt() - 1);
}

void VisualProfiler::addInfoBladeEngineTextLine(const UString &text)
{
	if (!m_vprof_ref->getBool() || !m_bVisible || vprof_display_mode.getInt() != INFO_BLADE_DISPLAY_MODE::INFO_BLADE_DISPLAY_MODE_ENGINE_INFO) return;

	m_engineTextLines.push_back(text);
}

void VisualProfiler::addInfoBladeAppTextLine(const UString &text)
{
	if (!m_vprof_ref->getBool() || !m_bVisible || vprof_display_mode.getInt() != INFO_BLADE_DISPLAY_MODE::INFO_BLADE_DISPLAY_MODE_APP_INFO) return;

	m_appTextLines.push_back(text);
}

void VisualProfiler::setProfile(ProfilerProfile *profile)
{
	m_profile = profile;

	// force everything to get re-built for the new profile with the next frame
	{
		m_groups.clear();

		m_bScheduledForceRebuildLineVao = true;
	}
}

bool VisualProfiler::isEnabled()
{
	return m_vprof_ref->getBool();
}

void VisualProfiler::collectProfilerNodesRecursive(const ProfilerNode *node, int depth, std::vector<NODE> &nodes, SPIKE &spike)
{
	if (node == NULL) return;

	// recursive call
	ProfilerNode *child = node->getChild();
	while (child != NULL)
	{
		collectProfilerNodesRecursive(child, depth + 1, nodes, spike);
		child = child->getSibling();
	}

	// add node (ignore root 0)
	if (depth > 0)
	{
		NODE entry;

		entry.node = node;
		entry.depth = depth;

		nodes.push_back(entry);



		const double timeLastFrame = node->getTimeLastFrame();
		if (spike.node.node == NULL || timeLastFrame > spike.timeLastFrame)
		{
			spike.node = entry;
			spike.timeLastFrame = timeLastFrame;
		}
	}
}

void VisualProfiler::collectProfilerNodesSpikeRecursive(const ProfilerNode *node, int depth, std::vector<SPIKE> &spikeNodes)
{
	if (node == NULL) return;

	// recursive call
	ProfilerNode *child = node->getChild();
	while (child != NULL)
	{
		collectProfilerNodesSpikeRecursive(child, depth + 1, spikeNodes);
		child = child->getSibling();
	}

	// add spike node (ignore root 0)
	if (depth > 0)
	{
		SPIKE spike;

		spike.node.node = node;
		spike.node.depth = depth;
		spike.timeLastFrame = node->getTimeLastFrame();

		spikeNodes.push_back(spike);
	}
}

int VisualProfiler::getGraphWidth()
{
	return (vprof_graph_width.getFloat() * env->getDPIScale());
}

int VisualProfiler::getGraphHeight()
{
	return (vprof_graph_height.getFloat() * env->getDPIScale());
}

void VisualProfiler::addTextLine(const UString &text, McFont *font, std::vector<TEXT_LINE> &textLines)
{
	TEXT_LINE textLine;
	{
		textLine.text = text;
		textLine.width = font->getStringWidth(text);
	}
	textLines.push_back(textLine);
}

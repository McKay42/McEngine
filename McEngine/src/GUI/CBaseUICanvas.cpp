/*
 * CBaseUICanvas.cpp
 *
 *  Created on: May 28, 2017
 *      Author: Psy
 */

#include "CBaseUICanvas.h"
#include "CBaseUIElement.h"
#include "Engine.h"

CBaseUICanvas::CBaseUICanvas(float xPos, float yPos, float xSize, float ySize) {
	if (xSize != 0){
		m_vSize = engine->getScreenSize();
	}

	else{
		m_vSize.x = xSize;
		m_vSize.y = ySize;
	}

	m_vPos.x = xPos;
	m_vPos.y = yPos;
	m_vAnchor.x = 0;
	m_vAnchor.y = 0;

	m_bVisible = true;

}

CBaseUICanvas::~CBaseUICanvas() {
	clear();
}

void CBaseUICanvas::clear()
{
	for (int i=0; i<m_vSlots.size(); i++)
	{
		delete m_vSlots[i]->element;
		delete m_vSlots[i];
	}

	m_vSlots = std::vector<Slot*>();
}

void CBaseUICanvas::empty(){
	m_vSlots = std::vector<Slot*>();
}

void CBaseUICanvas::addSlot(Slot *slot){
	if (slot == NULL) return;

	m_vSlots.push_back(slot);
}

void CBaseUICanvas::addSlot(CBaseUIElement *element, bool scaleByHeightOnly){
	if (element == NULL) return;

	Slot *newSlot = new Slot;
	newSlot->element = element;
	newSlot->scaleByHeightOnly = scaleByHeightOnly;

	m_vSlots.push_back(newSlot);
}

void CBaseUICanvas::addSlotBack(Slot *slot){
	if (slot == NULL) return;

	m_vSlots.insert(m_vSlots.begin(), slot);
}

void CBaseUICanvas::addSlotBack(CBaseUIElement *element, bool scaleByHeightOnly){
	if (element == NULL) return;

	Slot *newSlot = new Slot;
	newSlot->element = element;
	newSlot->scaleByHeightOnly = scaleByHeightOnly;

	m_vSlots.insert(m_vSlots.begin(), newSlot);
}

void CBaseUICanvas::insertSlot(Slot *slot, Slot *index){
	if (slot == NULL || index == NULL) return;

	for (int i=0; i<m_vSlots.size(); i++){
		if (m_vSlots[i] == index)
		{
			m_vSlots.insert(m_vSlots.begin() + clamp<int>(i, 0, m_vSlots.size()), slot);
			return;
		}
	}

	debugLog("Warning: CBaseUICanvas::insertSlot() couldn't find index\n");
}


void CBaseUICanvas::insertSlot(CBaseUIElement *element, Slot *index){
	if (element == NULL || index == NULL) return;

	Slot *newSlot = new Slot;
	newSlot->element = element;

	for (int i=0; i<m_vSlots.size(); i++){
		if (m_vSlots[i] == index){
			m_vSlots.insert(m_vSlots.begin() + clamp<int>(i, 0, m_vSlots.size()), newSlot);
			return;
		}
	}

	debugLog("Warning: CBaseUICanvas::insertSlot() couldn't find index\n");
}

void CBaseUICanvas::insertSlot(CBaseUIElement* element, bool scaleByHeightOnly, Slot *index){
	if (element == NULL || index == NULL) return;

	Slot *newSlot = new Slot;
	newSlot->element = element;
	newSlot->scaleByHeightOnly = scaleByHeightOnly;

	for (int i=0; i<m_vSlots.size(); i++){
		if (m_vSlots[i] == index){
			m_vSlots.insert(m_vSlots.begin() + clamp<int>(i, 0, m_vSlots.size()), newSlot);
			return;
		}
	}

	debugLog("Warning: CBaseUICanvas::insertSlot() couldn't find index\n");
}

void CBaseUICanvas::insertSlotBack(Slot *slot, Slot *index){
	if (slot == NULL || index == NULL) return;

	for (int i=0; i<m_vSlots.size(); i++){
		if (m_vSlots[i] == index){
			m_vSlots.insert(m_vSlots.begin() + clamp<int>(i+1, 0, m_vSlots.size()), slot);
			return;
		}
	}

	debugLog("Warning: CBaseUICanvas::insertSlotBack() couldn't find index\n");
}

void CBaseUICanvas::insertSlotBack(CBaseUIElement *element, Slot *index){
	if (element == NULL || index == NULL) return;

	Slot *newSlot = new Slot;
	newSlot->element = element;

	for (int i=0; i<m_vSlots.size(); i++){
		if (m_vSlots[i] == index){
			m_vSlots.insert(m_vSlots.begin() + clamp<int>(i+1, 0, m_vSlots.size()), newSlot);
			return;
		}
	}

	debugLog("Warning: CBaseUICanvas::insertSlotBack() couldn't find index\n");
}

void CBaseUICanvas::insertSlotBack(CBaseUIElement *element, bool scaleByHeightOnly, Slot *index){
	if (element == NULL || index == NULL) return;

	Slot *newSlot = new Slot;
	newSlot->element = element;
	newSlot->scaleByHeightOnly = scaleByHeightOnly;

	for (int i=0; i<m_vSlots.size(); i++){
		if (m_vSlots[i] == index){
			m_vSlots.insert(m_vSlots.begin() + clamp<int>(i+1, 0, m_vSlots.size()), newSlot);
			return;
		}
	}

	debugLog("Warning: CBaseUICanvas::insertSlotBack() couldn't find index\n");
}

void CBaseUICanvas::removeSlot(Slot *slot){
	for (int i=0; i<m_vSlots.size(); i++)
	{
		if (m_vSlots[i] == slot){
			m_vSlots.erase(m_vSlots.begin()+i);
			return;
		}
	}

	debugLog("Warning: CBaseUICanvas::removeSlot() couldn't find element\n");
}

void CBaseUICanvas::deleteSlot(Slot *slot){
	for (int i=0; i<m_vSlots.size(); i++){
		if (m_vSlots[i] == slot){
			delete m_vSlots[i]->element;
			delete m_vSlots[i];
			m_vSlots.erase(m_vSlots.begin()+i);
			return;
		}
	}

	debugLog("Warning: CBaseUICanvas::removeSlot() couldn't find element\n");
}

Slot *CBaseUICanvas::getSlotByElementName(UString name){
	for (int i=0; i<m_vSlots.size(); i++){
		if (m_vSlots[i]->element->getName() == name)
			return m_vSlots[i];
	}

	debugLog("Error: CBaseUICanvas::getSlotByElementName() \"%s\" does not exist!!!\n", name.toUtf8());
	return NULL;
}

std::vector<CBaseUIElement*> CBaseUICanvas::getAllElements(){
	std::vector<CBaseUIElement*> elements;

	for (int i=0; i<m_vSlots.size(); i++){
		elements.push_back(m_vSlots[i]->element);
	}

	return elements;
}

void CBaseUICanvas::draw(Graphics *g){
	if (!m_bVisible) return;

	Vector2 posNormal;
	Vector2 sizeNormal;
	for (int i=0; i<m_vSlots.size(); i++){
		if (!m_vSlots[i]->scaleByHeightOnly){
			sizeNormal = m_vSlots[i]->element->getSize();
			m_vSlots[i]->element->setSizeAbsolute(sizeNormal * m_vSize);
		}

		posNormal = m_vSlots[i]->element->getPos();
		m_vSlots[i]->element->setPosAbsolute(m_vPos + posNormal * m_vSize);

		m_vSlots[i]->element->draw(g);

		m_vSlots[i]->element->setSizeAbsolute(sizeNormal);
		m_vSlots[i]->element->setPosAbsolute(posNormal);
	}
}

void CBaseUICanvas::update(){
	if (!m_bVisible) return;

	for (int i=0; i<m_vSlots.size(); i++){
		m_vSlots[i]->element->update();
	}
}





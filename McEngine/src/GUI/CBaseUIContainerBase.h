/*
 * CBaseUIContainerBase.h
 *
 *  Created on: May 31, 2017
 *      Author: Psy
 */

#ifndef GUI_CBASEUICONTAINERBASE_H_
#define GUI_CBASEUICONTAINERBASE_H_

#define CONTAINER_BODY(T) ELEMENT_BODY(T)\
	\
	virtual T *addElement(CBaseUIElement *element, bool back=false) {CBaseUIContainerBase::addElement(element, back); return this;} \
	virtual T *addElement(std::shared_ptr<CBaseUIElement> element, bool back=false) {CBaseUIContainerBase::addElement(element, back); return this;} \
	virtual T *insertElement(CBaseUIElement *element, CBaseUIElement *index, bool back=false) {CBaseUIContainerBase::insertElement(element, index, back); return this;} \
	virtual T *insertElement(std::shared_ptr<CBaseUIElement> element, CBaseUIElement *index, bool back=false) {CBaseUIContainerBase::insertElement(element, index, back); return this;} \
	virtual T *insertElement(CBaseUIElement *element, std::shared_ptr<CBaseUIElement> index, bool back=false) {CBaseUIContainerBase::insertElement(element, index, back); return this;} \
	virtual T *insertElement(std::shared_ptr<CBaseUIElement> element, std::shared_ptr<CBaseUIElement> index, bool back=false) {CBaseUIContainerBase::insertElement(element, index, back); return this;} \
	virtual T *setClipping(bool clipping) {CBaseUIContainerBase::setClipping(clipping); return this;}

#include "CBaseUIElement.h"
#include "cbase.h"

class CBaseUIContainerBase : public CBaseUIElement
{
public:

	CBaseUIContainerBase(UString name="");
	virtual ~CBaseUIContainerBase();

	ELEMENT_BODY(CBaseUIContainerBase);

	virtual CBaseUIContainerBase *addElement(CBaseUIElement *element, bool back=false);
	virtual CBaseUIContainerBase *addElement(std::shared_ptr<CBaseUIElement> element, bool back=false);
	virtual CBaseUIContainerBase *insertElement(CBaseUIElement *element, CBaseUIElement *index, bool back=false);
	virtual CBaseUIContainerBase *insertElement(std::shared_ptr<CBaseUIElement> element, CBaseUIElement *index, bool back=false);
	virtual CBaseUIContainerBase *insertElement(CBaseUIElement *element, std::shared_ptr<CBaseUIElement> index, bool back=false);
	virtual CBaseUIContainerBase *insertElement(std::shared_ptr<CBaseUIElement> element, std::shared_ptr<CBaseUIElement> index, bool back=false);

	virtual void removeElement(CBaseUIElement *element);
	virtual void removeElement(std::shared_ptr<CBaseUIElement> element);

	virtual CBaseUIContainerBase *setClipping(bool clipping) {m_bClipping = clipping; return this;}

	CBaseUIElement *getElementByName(UString name, bool searchNestedContainers=false);
	std::shared_ptr<CBaseUIElement> getElementSharedByName(UString name, bool searchNestedContainers=false);
	std::vector<CBaseUIElement*> getAllElements();
	inline std::vector<std::shared_ptr<CBaseUIElement>> getAllElementsShared(){return m_vElements;}
	inline std::vector<std::shared_ptr<CBaseUIElement>> *getAllElementsReference(){return &m_vElements;}

	virtual void draw(Graphics *g);
	virtual void drawDebug(Graphics *g, Color color=COLOR(255,255,0,0)) {;}
	virtual void update();

	virtual void empty();

protected:
	// events
	virtual void updateElement(CBaseUIElement *element) {;}

	bool m_bClipping;
	std::vector<std::shared_ptr<CBaseUIElement>> m_vElements;
};

#endif /* GUI_CBASEUICONTAINERBASE_H_ */

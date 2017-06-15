/*
 * UIFrameworkTestVerticalBox.cpp
 *
 *  Created on: Jun 7, 2017
 *      Author: Psy
 */

#include "UIFrameworkTestVerticalBox.h"

#include "Engine.h"

#include "CBaseUI.h"


UIFrameworkTestVerticalBox::UIFrameworkTestVerticalBox(UIFrameworkTest *app) : UIFrameworkTestScreen(app)
{
	m_boxNormal = std::shared_ptr<UI::VerticalBox>((new UI::VerticalBox)
			->setPos(50, 350)
			->setSize(100, 600)
			->setAnchor(0, 0.5)
			->setName("NormalBox")

			->addElement((new UI::Textbox)
						 ->setText("1")
						 ->setTextJustification(1)
						 ->setName("Normal1")
			)

			->addElement((new UI::Textbox)
						 ->setText("2")
						 ->setTextJustification(1)
						 ->setName("Normal2")
			)

			->addElement((new UI::Textbox)
						 ->setText("3")
						 ->setTextJustification(1)
						 ->setName("Normal3")
			)

			->addElement((new UI::Textbox)
						 ->setText("4")
						 ->setTextJustification(1)
						 ->setName("Normal4")
			)
	);

	m_boxWithPadding = std::shared_ptr<UI::VerticalBox>((new UI::VerticalBox)
				->setPos(300, 350)
				->setSize(100, 600)
				->setAnchor(0, 0.5)
				->setName("PaddedBox")
				->setPadding(10)
				->setMargin(10, 20)

				->addElement((new UI::Textbox)
							 ->setText("1")
							 ->setTextJustification(1)
							 ->setName("Padded1")
				)

				->addElement((new UI::Textbox)
							 ->setText("2")
							 ->setTextJustification(1)
							 ->setName("Padded2")
				)

				->addElement((new UI::Textbox)
							 ->setText("3")
							 ->setTextJustification(1)
							 ->setName("Padded3")
				)

				->addElement((new UI::Textbox)
							 ->setText("4")
							 ->setTextJustification(1)
							 ->setName("Padded4")
				)
	);

	m_boxHeightOverride = std::shared_ptr<UI::VerticalBox>((new UI::VerticalBox)
				->setPos(550, 350)
				->setSize(100, 600)
				->setAnchor(0, 0.5)
				->setName("HeightOverrideBox")
				->setPadding(10)
				->setMargin(10)
				->setHeightOverride(true)

				->addElement((new UI::Textbox)
							 ->setRelSizeY(0.1)
							 ->setText("1")
							 ->setTextJustification(1)
							 ->setName("HeightOverride1")
				)

				->addElement((new UI::Textbox)
							 ->setRelSizeY(0.1)
							 ->setText("2")
							 ->setTextJustification(1)
							 ->setName("HeightOverride2")
				)

				->addElement((new UI::Textbox)
							 ->setRelSizeY(0.2)
							 ->setText("3")
							 ->setTextJustification(1)
							 ->setName("HeightOverride3")
				)

				->addElement((new UI::Textbox)
							 ->setRelSizeY(0.1)
							 ->setText("4")
							 ->setTextJustification(1)
							 ->setName("HeightOverride4")
				)
	);

	m_boxSizeByHeightOnly = std::shared_ptr<UI::VerticalBox>((new UI::VerticalBox)
				->setPos(800, 350)
				->setSize(100, 600)
				->setAnchor(0, 0.5)
				->setName("ScaleByHeightOnlyBox")
				->setPadding(10)
				->setMargin(10)
				->setScaleByHeightOnly(true)
				->setClipping(true)

				->addElement((new UI::Textbox)
							 ->setText("1")
							 ->setTextJustification(1)
							 ->setName("Height1")
							 ->setRelSizeY(0.7)
				)

				->addElement((new UI::Textbox)
							 ->setText("2")
							 ->setTextJustification(1)
							 ->setName("Height2")
							 ->setRelSizeY(0.7)
				)

				->addElement((new UI::Textbox)
							 ->setText("3")
							 ->setTextJustification(1)
							 ->setName("Height3")
							 ->setRelSizeY(0.9)
				)

				->addElement((new UI::Textbox)
							 ->setText("4")
							 ->setTextJustification(1)
							 ->setName("Height4")
							 ->setRelSizeY(0.7)
				)
	);

	m_boxNormalLabel = std::make_shared<UI::Label>(100, 25, 0, 0, "NormalLabel", "Normal");
	m_boxNormalLabel
					->setAnchor(0.5, 0)
					->setTextJustification(UI::Label::TEXT_JUSTIFICATION::TEXT_JUSTIFICATION_CENTERED)
					->setDrawFrame(false)
					->setDrawBackground(false)
					->setSizeToContent();

	m_boxWithPaddingLabel = std::make_shared<UI::Label>(350, 675, 0, 0, "PaddingLabel", "Padding: 10 - Margin: (10, 20)");
	m_boxWithPaddingLabel
						->setAnchor(0.5, 0)
						->setTextJustification(UI::Label::TEXT_JUSTIFICATION::TEXT_JUSTIFICATION_CENTERED)
						->setDrawFrame(false)
						->setDrawBackground(false)
						->setSizeToContent();

	m_boxHeightOverrideLabel = std::make_shared<UI::Label>(600, 25, 0, 0, "HeightOverrideLabel", "Padding: 10 - Margin: 10 - Height Override");
	m_boxHeightOverrideLabel
						->setAnchor(0.5, 0)
						->setTextJustification(UI::Label::TEXT_JUSTIFICATION::TEXT_JUSTIFICATION_CENTERED)
						->setDrawFrame(false)
						->setDrawBackground(false)
						->setSizeToContent();

	m_boxSizeByHeightOnlyLabel = std::make_shared<UI::Label>(850, 675, 0, 0, "ScaleByWidthOnly", "Padding: 10 - Margin: 10 - Scale By Width Only - Clipping");
	m_boxSizeByHeightOnlyLabel
						->setAnchor(0.5, 0)
						->setTextJustification(UI::Label::TEXT_JUSTIFICATION::TEXT_JUSTIFICATION_CENTERED)
						->setDrawFrame(false)
						->setDrawBackground(false)
						->setSizeToContent();

	m_resizeBoxesHeight = std::make_shared<UI::Button>(1000, 100, 200, 25, "ResizeHeightButton", "Resize Height");
	m_resizeBoxesHeight->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestVerticalBox::resizeBoxesHeight));

	m_resizeBoxesWidth = std::make_shared<UI::Button>(1000, 200, 200, 25, "ResizeWidthButton", "Resize Width");
	m_resizeBoxesWidth->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestVerticalBox::resizeBoxesWidth));

	m_addElement = std::make_shared<UI::Button>(1000, 300, 200, 25, "AddElementButton", "Add Element");
	m_addElement->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestVerticalBox::addElement));

	m_removeElement = std::make_shared<UI::Button>(1000, 400, 200, 25, "RemoveElementButton", "Remove Element");
	m_removeElement->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestVerticalBox::removeElement));

	m_bHeightResized = false;
	m_bWidthResized = false;
	m_elementCount = 4;

}

UIFrameworkTestVerticalBox::~UIFrameworkTestVerticalBox()
{
}

void UIFrameworkTestVerticalBox::draw(Graphics *g)
{
	m_boxNormal->draw(g);
	m_boxWithPadding->draw(g);
	m_boxHeightOverride->draw(g);
	m_boxSizeByHeightOnly->draw(g);

	m_boxNormal->drawDebug(g);
	m_boxWithPadding->drawDebug(g);
	m_boxHeightOverride->drawDebug(g);
	m_boxSizeByHeightOnly->drawDebug(g);

	m_boxNormalLabel->draw(g);
	m_boxWithPaddingLabel->draw(g);
	m_boxHeightOverrideLabel->draw(g);
	m_boxSizeByHeightOnlyLabel->draw(g);

	m_resizeBoxesHeight->draw(g);
	m_resizeBoxesWidth->draw(g);
	m_addElement->draw(g);
	m_removeElement->draw(g);
}

void UIFrameworkTestVerticalBox::update()
{
	m_boxNormal->update();
	m_boxWithPadding->update();
	m_boxHeightOverride->update();
	m_boxSizeByHeightOnly->update();

	m_resizeBoxesHeight->update();
	m_resizeBoxesWidth->update();
	m_addElement->update();
	m_removeElement->update();
}

void UIFrameworkTestVerticalBox::resizeBoxesHeight()
{
	if (!m_bHeightResized)
	{
		m_boxNormal->setSizeY(400);
		m_boxWithPadding->setSizeY(400);
		m_boxHeightOverride->setSizeY(400);
		m_boxSizeByHeightOnly->setSizeY(400);
		m_bHeightResized = true;
	}

	else
	{
		m_boxNormal->setSizeY(600);
		m_boxWithPadding->setSizeY(600);
		m_boxHeightOverride->setSizeY(600);
		m_boxSizeByHeightOnly->setSizeY(600);
		m_bHeightResized = false;
	}
}

void UIFrameworkTestVerticalBox::resizeBoxesWidth()
{
	if (!m_bWidthResized)
	{
		m_boxNormal->setSizeX(175);
		m_boxWithPadding->setSizeX(175);
		m_boxHeightOverride->setSizeX(175);
		m_boxSizeByHeightOnly->setSizeX(175);
		m_bWidthResized = true;
	}

	else
	{
		m_boxNormal->setSizeX(100);
		m_boxWithPadding->setSizeX(100);
		m_boxHeightOverride->setSizeX(100);
		m_boxSizeByHeightOnly->setSizeX(100);
		m_bWidthResized = false;
	}
}

void UIFrameworkTestVerticalBox::addElement()
{
	m_elementCount++;

	char text[8];
	sprintf(text, "%i", m_elementCount);

	char name[16];
	sprintf(name, "Normal%i", m_elementCount);
	m_boxNormal->addElement((new UI::Textbox)
							->setText(text)
							->setName(name)
							->setTextJustification(1));

	sprintf(name, "Padded%i", m_elementCount);
	m_boxWithPadding->addElement((new UI::Textbox)
								 ->setText(text)
								 ->setName(name)
								 ->setTextJustification(1));

	sprintf(name, "HeightOverride%i", m_elementCount);
	m_boxHeightOverride->addElement((new UI::Textbox)
								   ->setText(text)
								   ->setName(name)
								   ->setTextJustification(1)
								   ->setRelSizeY(0.1));

	sprintf(name, "Height%i", m_elementCount);
	m_boxSizeByHeightOnly->addElement((new UI::Textbox)
									  ->setText(text)
									  ->setName(name)
									  ->setTextJustification(1)
									  ->setRelSizeY(0.7));
}

void UIFrameworkTestVerticalBox::removeElement()
{
	char name[16];
	sprintf(name, "Normal%i", m_elementCount);
	m_boxNormal->removeElement(m_boxNormal->getElementByName(name));

	sprintf(name, "Padded%i", m_elementCount);
	m_boxWithPadding->removeElement(m_boxWithPadding->getElementByName(name));

	sprintf(name, "HeightOverride%i", m_elementCount);
	m_boxHeightOverride->removeElement(m_boxHeightOverride->getElementByName(name));

	sprintf(name, "Height%i", m_elementCount);
	m_boxSizeByHeightOnly->removeElement(m_boxSizeByHeightOnly->getElementByName(name));

	m_elementCount--;
}

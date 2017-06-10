/*
 * UIFrameworkTestHorizontalBox.cpp
 *
 *  Created on: Jun 7, 2017
 *      Author: Psy
 */

#include "UIFrameworkTestHorizontalBox.h"

#include "Engine.h"

#include "CBaseUI.h"


UIFrameworkTestHorizontalBox::UIFrameworkTestHorizontalBox(UIFrameworkTest *app) : UIFrameworkTestScreen(app)
{
	m_boxNormal = std::shared_ptr<UI::HorizontalBox>((new UI::HorizontalBox)
			->setPos(550, 25)
			->setSize(600, 100)
			->setAnchor(0.5, 0)
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

	m_boxWithPadding = std::shared_ptr<UI::HorizontalBox>((new UI::HorizontalBox)
				->setPos(550, 200)
				->setSize(600, 100)
				->setAnchor(0.5, 0)
				->setName("PaddedBox")
				->setPadding(10)
				->setMargin(20, 10)

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

	m_boxWidthOverride = std::shared_ptr<UI::HorizontalBox>((new UI::HorizontalBox)
				->setPos(550, 375)
				->setSize(600, 100)
				->setAnchor(0.5, 0)
				->setName("WidthOverrideBox")
				->setPadding(10)
				->setMargin(10)
				->setWidthOverride(true)

				->addElement((new UI::Textbox)
							 ->setRelSizeX(0.1)
							 ->setText("1")
							 ->setTextJustification(1)
							 ->setName("WidthOverride1")
				)

				->addElement((new UI::Textbox)
							 ->setRelSizeX(0.1)
							 ->setText("2")
							 ->setTextJustification(1)
							 ->setName("WidthOverride2")
				)

				->addElement((new UI::Textbox)
							 ->setRelSizeX(0.2)
							 ->setText("3")
							 ->setTextJustification(1)
							 ->setName("WidthOverride3")
				)

				->addElement((new UI::Textbox)
							 ->setRelSizeX(0.1)
							 ->setText("4")
							 ->setTextJustification(1)
							 ->setName("WidthOverride4")
				)
	);

	m_boxSizeByHeightOnly = std::shared_ptr<UI::HorizontalBox>((new UI::HorizontalBox)
				->setPos(550, 550)
				->setSize(600, 100)
				->setAnchor(0.5, 0)
				->setName("ScaleByHeightOnlyBox")
				->setPadding(10)
				->setMargin(10)
				->setScaleByHeightOnly(true)
				->setClipping(true)

				->addElement((new UI::Textbox)
							 ->setText("1")
							 ->setTextJustification(1)
							 ->setName("Height1")
							 ->setRelSizeX(0.3)
				)

				->addElement((new UI::Textbox)
							 ->setText("2")
							 ->setTextJustification(1)
							 ->setName("Height2")
							 ->setRelSizeX(0.3)
				)

				->addElement((new UI::Textbox)
							 ->setText("3")
							 ->setTextJustification(1)
							 ->setName("Height3")
							 ->setRelSizeX(0.5)
				)

				->addElement((new UI::Textbox)
							 ->setText("4")
							 ->setTextJustification(1)
							 ->setName("Height4")
							 ->setRelSizeX(0.3)
				)
	);

	m_boxNormalLabel = std::make_shared<UI::Label>(550, 0, 0, 0, "NormalLabel", "Normal");
	m_boxNormalLabel
					->setAnchor(0.5, 0)
					->setTextJustification(UI::Label::TEXT_JUSTIFICATION::TEXT_JUSTIFICATION_CENTERED)
					->setDrawFrame(false)
					->setDrawBackground(false)
					->setSizeToContent();

	m_boxWithPaddingLabel = std::make_shared<UI::Label>(550, 175, 0, 0, "PaddingLabel", "Padding: 10 - Margin: (20, 10)");
	m_boxWithPaddingLabel
						->setAnchor(0.5, 0)
						->setTextJustification(UI::Label::TEXT_JUSTIFICATION::TEXT_JUSTIFICATION_CENTERED)
						->setDrawFrame(false)
						->setDrawBackground(false)
						->setSizeToContent();

	m_boxWidthOverrideLabel = std::make_shared<UI::Label>(550, 350, 0, 0, "WidthOverrideLabel", "Padding: 10 - Margin: 10 - Width Override");
	m_boxWidthOverrideLabel
						->setAnchor(0.5, 0)
						->setTextJustification(UI::Label::TEXT_JUSTIFICATION::TEXT_JUSTIFICATION_CENTERED)
						->setDrawFrame(false)
						->setDrawBackground(false)
						->setSizeToContent();

	m_boxSizeByHeightOnlyLabel = std::make_shared<UI::Label>(550, 525, 0, 0, "ScaleByHeightOnly", "Padding: 10 - Margin: 10 - Scale By Height Only - Clipping");
	m_boxSizeByHeightOnlyLabel
						->setAnchor(0.5, 0)
						->setTextJustification(UI::Label::TEXT_JUSTIFICATION::TEXT_JUSTIFICATION_CENTERED)
						->setDrawFrame(false)
						->setDrawBackground(false)
						->setSizeToContent();

	m_resizeBoxesHeight = std::make_shared<UI::Button>(1000, 100, 200, 25, "ResizeHeightButton", "Resize Height");
	m_resizeBoxesHeight->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestHorizontalBox::resizeBoxesHeight));

	m_resizeBoxesWidth = std::make_shared<UI::Button>(1000, 200, 200, 25, "ResizeWidthButton", "Resize Width");
	m_resizeBoxesWidth->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestHorizontalBox::resizeBoxesWidth));

	m_addElement = std::make_shared<UI::Button>(1000, 300, 200, 25, "AddElementButton", "Add Element");
	m_addElement->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestHorizontalBox::addElement));

	m_removeElement = std::make_shared<UI::Button>(1000, 400, 200, 25, "RemoveElementButton", "Remove Element");
	m_removeElement->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestHorizontalBox::removeElement));

	m_bHeightResized = false;
	m_bWidthResized = false;
	m_elementCount = 4;

}

UIFrameworkTestHorizontalBox::~UIFrameworkTestHorizontalBox()
{
}

void UIFrameworkTestHorizontalBox::draw(Graphics *g)
{
	m_boxNormal->draw(g);
	m_boxWithPadding->draw(g);
	m_boxWidthOverride->draw(g);
	m_boxSizeByHeightOnly->draw(g);

	m_boxNormal->drawDebug(g);
	m_boxWithPadding->drawDebug(g);
	m_boxWidthOverride->drawDebug(g);
	m_boxSizeByHeightOnly->drawDebug(g);

	m_boxNormalLabel->draw(g);
	m_boxWithPaddingLabel->draw(g);
	m_boxWidthOverrideLabel->draw(g);
	m_boxSizeByHeightOnlyLabel->draw(g);

	m_resizeBoxesHeight->draw(g);
	m_resizeBoxesWidth->draw(g);
	m_addElement->draw(g);
	m_removeElement->draw(g);
}

void UIFrameworkTestHorizontalBox::update()
{
	m_boxNormal->update();
	m_boxWithPadding->update();
	m_boxWidthOverride->update();
	m_boxSizeByHeightOnly->update();

	m_resizeBoxesHeight->update();
	m_resizeBoxesWidth->update();
	m_addElement->update();
	m_removeElement->update();
}

void UIFrameworkTestHorizontalBox::resizeBoxesHeight()
{
	if (!m_bHeightResized)
	{
		m_boxNormal->setSizeY(125);
		m_boxWithPadding->setSizeY(125);
		m_boxWidthOverride->setSizeY(125);
		m_boxSizeByHeightOnly->setSizeY(125);
		m_bHeightResized = true;
	}

	else
	{
		m_boxNormal->setSizeY(100);
		m_boxWithPadding->setSizeY(100);
		m_boxWidthOverride->setSizeY(100);
		m_boxSizeByHeightOnly->setSizeY(100);
		m_bHeightResized = false;
	}
}

void UIFrameworkTestHorizontalBox::resizeBoxesWidth()
{
	if (!m_bWidthResized)
	{
		m_boxNormal->setSizeX(400);
		m_boxWithPadding->setSizeX(400);
		m_boxWidthOverride->setSizeX(400);
		m_boxSizeByHeightOnly->setSizeX(400);
		m_bWidthResized = true;
	}

	else
	{
		m_boxNormal->setSizeX(600);
		m_boxWithPadding->setSizeX(600);
		m_boxWidthOverride->setSizeX(600);
		m_boxSizeByHeightOnly->setSizeX(600);
		m_bWidthResized = false;
	}
}

void UIFrameworkTestHorizontalBox::addElement()
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

	sprintf(name, "WidthOverride%i", m_elementCount);
	m_boxWidthOverride->addElement((new UI::Textbox)
								   ->setText(text)
								   ->setName(name)
								   ->setTextJustification(1)
								   ->setRelSizeX(0.1));

	sprintf(name, "Height%i", m_elementCount);
	m_boxSizeByHeightOnly->addElement((new UI::Textbox)
									  ->setText(text)
									  ->setName(name)
									  ->setTextJustification(1)
									  ->setRelSizeX(0.3));
}

void UIFrameworkTestHorizontalBox::removeElement()
{
	char name[16];
	sprintf(name, "Normal%i", m_elementCount);
	m_boxNormal->removeElement(m_boxNormal->getElementByName(name));

	sprintf(name, "Padded%i", m_elementCount);
	m_boxWithPadding->removeElement(m_boxWithPadding->getElementByName(name));

	sprintf(name, "WidthOverride%i", m_elementCount);
	m_boxWidthOverride->removeElement(m_boxWidthOverride->getElementByName(name));

	sprintf(name, "Height%i", m_elementCount);
	m_boxSizeByHeightOnly->removeElement(m_boxSizeByHeightOnly->getElementByName(name));

	m_elementCount--;
}



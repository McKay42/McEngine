/*
 * UIFrameworktTestMultiCanvas.cpp
 *
 *  Created on: May 29, 2017
 *      Author: Psy
 */

#include "UIFrameworkTestMultiCanvas.h"

#include "Engine.h"
#include "UIFrameworkTest.h"

#include "CBaseUI.h"

UIFrameworkTestMultiCanvas::UIFrameworkTestMultiCanvas(UIFrameworkTest *app) : UIFrameworkTestScreen(app)
{
	// Main Canvas
	m_canvasMain = std::shared_ptr<UI::Canvas>((new UI::Canvas)
			->setPos(0, 0)
			->setSize(engine->getScreenSize())
			->setName("MainCanvas")

			// Top Left
			->addElement((new UI::Canvas)
						 ->setRelPos(0, 0)
						 ->setRelSize(0.5, 0.5)
						 ->setName("TopLeftCanvas")

						 // Top Left
						 ->addElement((new UI::Textbox)
									  ->setRelPos(0, 0)
									  ->setRelSize(0.15, 0.1)
									  ->setName("TopLeft-TopLeft")
									  ->setText("TopLeft")
									  ->setTextJustification(1)
						 )

						 // Top Right
						 ->addElement((new UI::Textbox)
									  ->setRelPos(1, 0)
									  ->setAnchor(1, 0)
									  ->setRelSize(0.15, 0.1)
									  ->setName("TopLeft-TopRight")
									  ->setText("TopRight")
									  ->setTextJustification(1)
						 )

						 // Bottom Left
						 ->addElement((new UI::Textbox)
									  ->setRelPos(0, 1)
									  ->setAnchor(0, 1)
									  ->setRelSize(0.15, 0.1)
									  ->setName("TopLeft-BottomLeft")
									  ->setText("BottomLeft")
									  ->setTextJustification(1)
						 )

						 // Bottom Right
						 ->addElement((new UI::Textbox)
									  ->setRelPos(1, 1)
									  ->setAnchor(1, 1)
									  ->setRelSize(0.15, 0.1)
									  ->setName("TopLeft-BottomRight")
									  ->setText("BottomRight")
									  ->setTextJustification(1)
						 )
			)

			// Top Right
			->addElement((new UI::Canvas)
						 ->setRelPos(1, 0)
						 ->setAnchor(1, 0)
						 ->setRelSize(0.5, 0.5)
						 ->setName("TopRightCanvas")

						 // Top Left
						 ->addElement((new UI::Textbox)
									  ->setRelPos(0, 0)
									  ->setRelSize(0.15, 0.1)
									  ->setName("TopRight-TopLeft")
									  ->setText("TopLeft")
									  ->setTextJustification(1)
						 )

						 // Top Right
						 ->addElement((new UI::Textbox)
									  ->setRelPos(1, 0)
									  ->setAnchor(1, 0)
									  ->setRelSize(0.15, 0.1)
									  ->setName("TopRight-TopRight")
									  ->setText("TopRight")
									  ->setTextJustification(1)
						 )

						 // Bottom Left
						 ->addElement((new UI::Textbox)
									  ->setRelPos(0, 1)
									  ->setAnchor(0, 1)
									  ->setRelSize(0.15, 0.1)
									  ->setName("TopRight-BottomLeft")
									  ->setText("BottomLeft")
									  ->setTextJustification(1)
						 )

						 // Bottom Right
						 ->addElement((new UI::Textbox)
									  ->setRelPos(1, 1)
									  ->setAnchor(1, 1)
									  ->setRelSize(0.15, 0.1)
									  ->setName("TopRight-BottomRight")
									  ->setText("BottomRight")
									  ->setTextJustification(1)
						 )
				)

				// Bottom Left
				->addElement((new UI::Canvas)
							 ->setRelPos(0, 1)
							 ->setAnchor(0, 1)
							 ->setRelSize(0.5, 0.5)
							 ->setName("BottomLeftCanvas")

							 // Top Left
							 ->addElement((new UI::Textbox)
										  ->setRelPos(0, 0)
										  ->setRelSize(0.15, 0.1)
										  ->setName("BottomLeft-TopLeft")
										  ->setText("TopLeft")
										  ->setTextJustification(1)
							 )

							 // Top Right
							 ->addElement((new UI::Textbox)
										  ->setRelPos(1, 0)
										  ->setAnchor(1, 0)
										  ->setRelSize(0.15, 0.1)
										  ->setName("BottomLeft-TopRight")
										  ->setText("TopRight")
										  ->setTextJustification(1)
							 )

							 // Bottom Left
							 ->addElement((new UI::Textbox)
										  ->setRelPos(0, 1)
										  ->setAnchor(0, 1)
										  ->setRelSize(0.15, 0.1)
										  ->setName("BottomLeft-BottomLeft")
										  ->setText("BottomLeft")
										  ->setTextJustification(1)
							 )

							 // Bottom Right
							 ->addElement((new UI::Textbox)
										  ->setRelPos(1, 1)
										  ->setAnchor(1, 1)
										  ->setRelSize(0.15, 0.1)
										  ->setName("BottomLeft-BottomRight")
										  ->setText("BottomRight")
										  ->setTextJustification(1)
							 )
					)

					// Bottom Right
					->addElement((new UI::Canvas)
								 ->setRelPos(1, 1)
								 ->setAnchor(1, 1)
								 ->setRelSize(0.5, 0.5)
								 ->setName("BottomRightCanvas")

								 // Top Left
								 ->addElement((new UI::Textbox)
											  ->setRelPos(0, 0)
											  ->setRelSize(0.15, 0.1)
											  ->setName("BottomRight-TopLeft")
											  ->setText("TopLeft")
											  ->setTextJustification(1)
								 )

								 // Top Right
								 ->addElement((new UI::Textbox)
											  ->setRelPos(1, 0)
											  ->setAnchor(1, 0)
											  ->setRelSize(0.15, 0.1)
											  ->setName("BottomRight-TopRight")
											  ->setText("TopRight")
											  ->setTextJustification(1)
								 )

								 // Bottom Left
								 ->addElement((new UI::Textbox)
											  ->setRelPos(0, 1)
											  ->setAnchor(0, 1)
											  ->setRelSize(0.15, 0.1)
											  ->setName("BottomRight-BottomLeft")
											  ->setText("BottomLeft")
											  ->setTextJustification(1)
								 )

								 // Bottom Right
								 ->addElement((new UI::Textbox)
											  ->setRelPos(1, 1)
											  ->setAnchor(1, 1)
											  ->setRelSize(0.15, 0.1)
											  ->setName("BottomRight-BottomRight")
											  ->setText("BottomRight")
											  ->setTextJustification(1)
								 )
					)
		);

	// Buttons
	m_canvasMainResize 	= std::make_shared<UI::Button>(300, 600, 300, 25, "canvasMainResize", "Resize Main Canvas");
	m_canvasMainResize->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestMultiCanvas::canvasMainResize));

	m_canvasMainMove 	= std::make_shared<UI::Button>(300, 500, 300, 25, "canvasMainMove", "Move Main Canvas");
	m_canvasMainMove->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestMultiCanvas::canvasMainMove));

	m_canvasInnerResize = std::make_shared<UI::Button>(300, 400, 300, 25, "canvasInnerResize", "Resize Inner Canvas");
	m_canvasInnerResize->setClickCallback(fastdelegate::MakeDelegate(this, &UIFrameworkTestMultiCanvas::canvasInnerResize));

	m_bCanvasMainResize=false;
	m_bCanvasMainMove=false;
	m_bCanvasInnerResize=false;
}

UIFrameworkTestMultiCanvas::~UIFrameworkTestMultiCanvas()
{
	m_canvasMain.reset();
	m_canvasMainResize.reset();
	m_canvasMainMove.reset();
	m_canvasInnerResize.reset();
}

void UIFrameworkTestMultiCanvas::draw(Graphics *g)
{
	m_canvasMain->draw(g);
	m_canvasMainResize->draw(g);
	m_canvasMainMove->draw(g);
	m_canvasInnerResize->draw(g);

	m_canvasMain->drawDebug(g);

	dynamic_cast<UI::Canvas *>(m_canvasMain->getElementByName("TopLeftCanvas"))->drawDebug(g, COLOR(255, 0, 255, 0));
	dynamic_cast<UI::Canvas *>(m_canvasMain->getElementByName("TopRightCanvas"))->drawDebug(g, COLOR(255, 0, 255, 0));
	dynamic_cast<UI::Canvas *>(m_canvasMain->getElementByName("BottomLeftCanvas"))->drawDebug(g, COLOR(255, 0, 255, 0));
	dynamic_cast<UI::Canvas *>(m_canvasMain->getElementByName("BottomRightCanvas"))->drawDebug(g, COLOR(255, 0, 255, 0));
}

void UIFrameworkTestMultiCanvas::update()
{
	m_canvasMain->update();
	m_canvasMainResize->update();
	m_canvasMainMove->update();
	m_canvasInnerResize->update();
}

void UIFrameworkTestMultiCanvas::onResolutionChanged(Vector2 newResolution)
{
	if (!m_bCanvasMainResize)
	{
		m_canvasMain->setSize(newResolution);
	}
}

void UIFrameworkTestMultiCanvas::canvasMainResize()
{
	if (m_bCanvasMainResize)
	{
		m_canvasMain->setSize(engine->getScreenSize());
		m_bCanvasMainResize = false;
	}
	else
	{
		m_canvasMain->setSize(800, 600);
		m_bCanvasMainResize = true;
	}

}

void UIFrameworkTestMultiCanvas::canvasMainMove()
{
	if (m_bCanvasMainMove)
	{
		m_canvasMain->setPos(0, 0);
		m_bCanvasMainMove = false;
	}
	else
	{
		m_canvasMain->setPos(150, 150);
		m_bCanvasMainMove = true;
	}
}

void UIFrameworkTestMultiCanvas::canvasInnerResize()
{
	if (m_bCanvasInnerResize)
	{
		m_canvasMain->getElementByName("TopLeftCanvas")->setRelSize(0.5, 0.5);
		m_canvasMain->getElementByName("TopRightCanvas")->setRelSize(0.5, 0.5);
		m_canvasMain->getElementByName("BottomLeftCanvas")->setRelSize(0.5, 0.5);
		m_canvasMain->getElementByName("BottomRightCanvas")->setRelSize(0.5, 0.5);
		m_bCanvasInnerResize=false;
	}
	else
	{
		m_canvasMain->getElementByName("TopLeftCanvas")->setRelSize(0.25, 0.25);
		m_canvasMain->getElementByName("TopRightCanvas")->setRelSize(0.75, 0.25);
		m_canvasMain->getElementByName("BottomLeftCanvas")->setRelSize(0.5, 0.75);
		m_canvasMain->getElementByName("BottomRightCanvas")->setRelSize(0.5, 0.75);
		m_bCanvasInnerResize=true;
	}
}

/*
    This file is part of Helio Workstation.

    Helio is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Helio is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Helio. If not, see <http://www.gnu.org/licenses/>.
*/

#include "Common.h"
#include "MultiTouchController.h"

#if HELIO_DESKTOP
#   define ZOOM_THRESHOLD 0.5
#   define DRAG_THRESHOLD 10
#   define ZOOM_H_SPEED 0.005f
#   define ZOOM_V_SPEED 0.01f
#elif JUCE_IOS
#   define ZOOM_THRESHOLD 0.1f
#   define DRAG_THRESHOLD 25
#   define ZOOM_H_SPEED 0.0095f
#   define ZOOM_V_SPEED 0.02f
#elif JUCE_ANDROID
#   define ZOOM_THRESHOLD 0.15f
#   define DRAG_THRESHOLD 25
#   define ZOOM_H_SPEED 0.0025f
#   define ZOOM_V_SPEED 0.005f
#endif

MultiTouchController::MultiTouchController(MultiTouchListener &parent) :
    listener(parent),
    zoomDiff(0, 0),
    dragDiff(0, 0),
    finger1Anchor(0, 0),
    finger2Anchor(0, 0),
    finger1Position(0, 0),
    finger2Position(0, 0),
    center1(0, 0),
    center2(0, 0),
    finger1On(false),
    finger2On(false),
    gesture(NoMultitouch)
{

}

void MultiTouchController::mouseDown(const MouseEvent &event)
{
    if (event.source.getIndex() > 1)
    {
        return;
    }

    this->finger1On = (event.source.getIndex() == 0) ? true : this->finger1On;
    this->finger2On = (event.source.getIndex() == 1) ? true : this->finger2On;

    if (this->finger1On && this->finger2On && this->gesture != HasMultitouch)
    {
        this->gesture = HasMultitouch;
        this->listener.multiTouchCancelPan();
        this->listener.multiTouchCancelZoom();
        //Logger::writeToLog("Enter Multitouch");
    }
    
    this->initAnchors(event);
}

void MultiTouchController::mouseDrag(const MouseEvent &event)
{
    if (this->gesture != HasMultitouch) { return; }
    
    if (event.source.getIndex() == 0)
    {
        this->finger1Position = event.source.getScreenPosition();
        this->center1 = this->listener.getMultiTouchOrigin(event.position);
    }
    else if (event.source.getIndex() == 1)
    {
        this->finger2Position = event.source.getScreenPosition();
        this->center2 = this->listener.getMultiTouchOrigin(event.position);
    }
    
    const Point<float> f1DragLength = (this->finger1Anchor - this->finger1Position);
    const Point<float> f2DragLength = (this->finger2Anchor - this->finger2Position);
    const Point<float> midDragLength = (f1DragLength + f2DragLength) / 2;
    const Point<float> dragOffset = (midDragLength - this->dragDiff).toFloat() * 3;
    
    const float vZoomLength = fabs(this->finger2Position.getX() - this->finger1Position.getX());
    const float hZoomLength = fabs(this->finger2Position.getY() - this->finger1Position.getY());
    const Point<float>zoomOffset = (Point<float>(vZoomLength, hZoomLength) - this->zoomDiff) * Point<float>(ZOOM_V_SPEED, ZOOM_H_SPEED);

//    if (userIntents == Dragging)
//    {
//        this->listener.multiTouchPanEvent(dragOffset.toInt());
//        this->dragDiff = midDragLength;
//        
//        this->zoomDiff.setXY(vZoomLength, hZoomLength);
//        return;
//    }
    
//    if (userIntents == Zooming)
    {
        // todo! rewrite
        
        //Logger::writeToLog("Zoom Vertical: " + String(hZoomOffset));
        const Point<float> diffV(0.f, zoomOffset.getY());
        this->listener.multiTouchCancelPan();
        this->listener.multiTouchZoomEvent((this->center1 + this->center2) / 2.f, diffV);

        //Logger::writeToLog("Zoom Horizontal: " + String(vZoomOffset));
        const Point<float> diffH(zoomOffset.getX(), 0.f);
        this->listener.multiTouchCancelPan();
        this->listener.multiTouchZoomEvent((this->center1 + this->center2) / 2.f, diffH);
        
        this->zoomDiff.setXY(vZoomLength, hZoomLength);
    }
}

void MultiTouchController::mouseUp(const MouseEvent &event)
{
    if (this->gesture != NoMultitouch)
    {
        this->finger1On = false;
        this->finger2On = false;
        this->gesture = NoMultitouch;
        this->zoomDiff.setXY(0, 0);
        this->dragDiff.setXY(0, 0);
        this->listener.multiTouchCancelPan();
        this->listener.multiTouchCancelZoom();
        //Logger::writeToLog("Exit Multitouch");
    }
}

void MultiTouchController::initAnchors(const MouseEvent &event)
{
    if (event.source.getIndex() == 0)
    {
        this->finger1Anchor = event.source.getScreenPosition();
        this->finger1Position = event.source.getScreenPosition();
        this->center1 = this->listener.getMultiTouchOrigin(event.position);
        //Logger::writeToLog("anchor0 " + String(anchor0.getX()) + ", " + String(anchor0.getY()));
        //Logger::writeToLog("anchor1 " + String(anchor1.getX()) + ", " + String(anchor1.getY()));
    }
    else if (event.source.getIndex() == 1)
    {
        this->finger2Anchor = event.source.getScreenPosition();
        this->finger2Position = event.source.getScreenPosition();
        this->center2 = this->listener.getMultiTouchOrigin(event.position);
        //Logger::writeToLog("anchor2 " + String(anchor2.getX()) + ", " + String(anchor2.getY()));
    }

    const int xLength = fabs(this->finger2Position.getX() - this->finger1Position.getX());
    const int yLength = fabs(this->finger2Position.getY() - this->finger1Position.getY());
    this->zoomDiff.setXY(xLength, yLength);
}

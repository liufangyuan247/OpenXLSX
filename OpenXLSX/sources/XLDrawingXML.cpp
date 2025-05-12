/*

   ____                               ____      ___ ____       ____  ____      ___
  6MMMMb                              `MM(      )M' `MM'      6MMMMb\`MM(      )M'
 8P    Y8                              `MM.     d'   MM      6M'    ` `MM.     d'
6M      Mb __ ____     ____  ___  __    `MM.   d'    MM      MM        `MM.   d'
MM      MM `M6MMMMb   6MMMMb `MM 6MMb    `MM. d'     MM      YM.        `MM. d'
MM      MM  MM'  `Mb 6M'  `Mb MMM9 `Mb    `MMd       MM       YMMMMb     `MMd
MM      MM  MM    MM MM    MM MM'   MM     dMM.      MM           `Mb     dMM.
MM      MM  MM    MM MMMMMMMM MM    MM    d'`MM.     MM            MM    d'`MM.
YM      M9  MM    MM MM       MM    MM   d'  `MM.    MM            MM   d'  `MM.
 8b    d8   MM.  ,M9 YM    d9 MM    MM  d'    `MM.   MM    / L    ,M9  d'    `MM.
  YMMMM9    MMYMMM9   YMMMM9 _MM_  _MM_M(_    _)MM_ _MMMMMMM MYMMMM9 _M(_    _)MM_
            MM
            MM
           _MM_

  Copyright (c) 2018, Kenneth Troldal Balslev

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  - Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  - Neither the name of the author nor the
    names of any contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

// ===== External Includes ===== //
#include <algorithm>
#include <pugixml.hpp>

// ===== OpenXLSX Includes ===== //
#include "XLDrawingXML.hpp"
#include "XLException.hpp"
#include "XLDocument.hpp"

using namespace OpenXLSX;

/**
 * @details Constructor taking an XLXmlData pointer. If the XML document is empty,
 * it will be initialized with the basic structure for a drawing file.
 */
XLDrawingXML::XLDrawingXML(XLXmlData* xmlData) 
    : XLXmlFile(xmlData) 
{
    // Check if the xml document is empty and initialize it if necessary
    if (xmlDocument().document_element().empty()) {
        initializeDrawingXML();
    }
}

/**
 * @details Initialize a new drawing XML file with the basic structure.
 */
void XLDrawingXML::initializeDrawingXML() 
{
    auto& doc = xmlDocument();
    XMLNode root;
    
    if (!doc.document_element()) {
        root = doc.append_child("xdr:wsDr");
    } else {
        root = doc.document_element();
    }
    
    // Ensure all required namespaces are present
    const std::pair<const char*, const char*> namespaces[] = {
        {"xmlns:xdr", "http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing"},
        {"xmlns:a", "http://schemas.openxmlformats.org/drawingml/2006/main"},
        {"xmlns:r", "http://schemas.openxmlformats.org/officeDocument/2006/relationships"},
        {"xmlns:a14", "http://schemas.microsoft.com/office/drawing/2010/main"}
    };
    
    for (const auto& ns : namespaces) {
        if (!root.attribute(ns.first)) {
            root.append_attribute(ns.first) = ns.second;
        }
    }
}

/**
 * @details Get the next available drawing ID by examining all existing drawing elements
 * and returning the highest ID + 1.
 */
uint32_t XLDrawingXML::getNextDrawingID() const 
{
    uint32_t maxID = 0;
    XMLNode docElement = xmlDocument().document_element();
    
    if (!docElement) return 1;
    
    for (auto directChild : docElement.children()) {
        XMLNode cNvPrNode;
        std::string nodeName = directChild.name();
        
        // Handle drawing container nodes
        if (nodeName == "xdr:twoCellAnchor" || nodeName == "xdr:absoluteAnchor") {
            auto picNode = directChild.child("xdr:pic");
            if (picNode) {
                auto nvPicPrNode = picNode.child("xdr:nvPicPr");
                if (nvPicPrNode) cNvPrNode = nvPicPrNode.child("xdr:cNvPr");
            } else {
                auto graphicFrameNode = directChild.child("xdr:graphicFrame");
                if (graphicFrameNode) {
                    auto nvGraphicFramePrNode = graphicFrameNode.child("xdr:nvGraphicFramePr");
                    if (nvGraphicFramePrNode) cNvPrNode = nvGraphicFramePrNode.child("xdr:cNvPr");
                }
            }
        } 
        // Handle direct graphicFrame at root level
        else if (nodeName == "xdr:graphicFrame") {
            auto nvGraphicFramePrNode = directChild.child("xdr:nvGraphicFramePr");
            if (nvGraphicFramePrNode) cNvPrNode = nvGraphicFramePrNode.child("xdr:cNvPr");
        }
        // Handle shapes
        else if (directChild.child("xdr:nvSpPr")) {
            auto nvSpPrNode = directChild.child("xdr:nvSpPr");
            if (nvSpPrNode) cNvPrNode = nvSpPrNode.child("xdr:cNvPr");
        }

        // Get ID value if cNvPr exists and has id attribute
        if (cNvPrNode && cNvPrNode.attribute("id")) {
            maxID = std::max(maxID, cNvPrNode.attribute("id").as_uint());
        }
    }
    
    return maxID + 1;
}

/**
 * @details Add an image to the drawing file, specifying its position and size.
 */
bool XLDrawingXML::addImage(const std::string& relID, 
                           uint32_t fromRow, 
                           uint16_t fromCol, 
                           uint32_t toRow, 
                           uint16_t toCol) 
{
    try {
        auto root = xmlDocument().document_element();
        
        // Create anchor that attaches to cells
        auto anchor = root.append_child("xdr:twoCellAnchor");
        
        // Set starting position (from)
        auto from = anchor.append_child("xdr:from");
        from.append_child("xdr:col").text().set(static_cast<uint32_t>(fromCol));
        from.append_child("xdr:colOff").text().set(0);
        from.append_child("xdr:row").text().set(fromRow);
        from.append_child("xdr:rowOff").text().set(0);
        
        // Set ending position (to)
        auto to = anchor.append_child("xdr:to");
        to.append_child("xdr:col").text().set(static_cast<uint32_t>(toCol));
        to.append_child("xdr:colOff").text().set(0);
        to.append_child("xdr:row").text().set(toRow);
        to.append_child("xdr:rowOff").text().set(0);
        
        // Create picture element
        auto pic = anchor.append_child("xdr:pic");
        
        // Add non-visual picture properties
        auto nvPicPr = pic.append_child("xdr:nvPicPr");
        
        // Set ID and name
        auto cNvPr = nvPicPr.append_child("xdr:cNvPr");
        uint32_t id = getNextDrawingID();
        cNvPr.append_attribute("id") = id;
        cNvPr.append_attribute("name") = ("Picture " + std::to_string(id)).c_str();
        
        // Add non-visual picture properties
        nvPicPr.append_child("xdr:cNvPicPr");
        
        // Add blip fill (image reference)
        auto blipFill = pic.append_child("xdr:blipFill");
        auto blip = blipFill.append_child("a:blip");
        blip.append_attribute("xmlns:r") = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";
        blip.append_attribute("r:embed") = relID.c_str();
        
        // Add stretch to fill
        auto stretch = blipFill.append_child("a:stretch");
        stretch.append_child("a:fillRect");
        
        // Add shape properties (rectangle)
        auto spPr = pic.append_child("xdr:spPr");
        auto prstGeom = spPr.append_child("a:prstGeom");
        prstGeom.append_attribute("prst") = "rect";
        prstGeom.append_child("a:avLst");
        
        // Add client data
        anchor.append_child("xdr:clientData");
        
        return true;
    }
    catch (const std::exception& e) {
        throw XLException("Error adding image to drawing: " + std::string(e.what()));
    }
    
    return false;
}

bool XLDrawingXML::addImageAbsolute(const std::string& relID,
                                    int64_t xOffsetEMU,
                                    int64_t yOffsetEMU,
                                    int64_t widthEMU,
                                    int64_t heightEMU)
{
    try {
        initializeDrawingXML();

        auto wsDrNode = xmlDocument().document_element();
        if (!wsDrNode || std::string(wsDrNode.name()) != "xdr:wsDr") {
            return false; 
        }

        uint32_t newDrawingID = getNextDrawingID();
        std::string drawingName = "Picture " + std::to_string(newDrawingID);

        auto anchorNode = wsDrNode.append_child("xdr:twoCellAnchor");
        anchorNode.append_attribute("editAs").set_value("oneCell");

        // Set position (from element)
        auto from = anchorNode.append_child("xdr:from");
        from.append_child("xdr:col").text().set(0);
        from.append_child("xdr:colOff").text().set(xOffsetEMU);
        from.append_child("xdr:row").text().set(0);
        from.append_child("xdr:rowOff").text().set(yOffsetEMU);
        
        // Set size (to element)
        auto to = anchorNode.append_child("xdr:to");
        to.append_child("xdr:col").text().set(0);
        to.append_child("xdr:colOff").text().set(xOffsetEMU + widthEMU);
        to.append_child("xdr:row").text().set(0);
        to.append_child("xdr:rowOff").text().set(yOffsetEMU + heightEMU);

        // Create picture element
        auto picNode = anchorNode.append_child("xdr:pic");

        // Add non-visual picture properties
        auto nvPicPrNode = picNode.append_child("xdr:nvPicPr");
        auto cNvPrNode = nvPicPrNode.append_child("xdr:cNvPr");
        cNvPrNode.append_attribute("id").set_value(newDrawingID);
        cNvPrNode.append_attribute("name").set_value(drawingName.c_str());
        cNvPrNode.append_attribute("descr").set_value("Absolute Positioned Image");
        
        nvPicPrNode.append_child("xdr:cNvPicPr");

        // Add blip fill (image reference)
        auto blipFillNode = picNode.append_child("xdr:blipFill");
        auto blipNode = blipFillNode.append_child("a:blip");
        blipNode.append_attribute("xmlns:r") = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";
        blipNode.append_attribute("r:embed").set_value(relID.c_str());

        // Add stretch properties
        auto stretchNode = blipFillNode.append_child("a:stretch");
        stretchNode.append_child("a:fillRect");

        // Add shape properties
        auto spPrNode = picNode.append_child("xdr:spPr");
        auto xfrmNode = spPrNode.append_child("a:xfrm");
        
        // Position and size in shape properties
        auto offNode = xfrmNode.append_child("a:off");
        offNode.append_attribute("x").set_value(std::to_string(xOffsetEMU).c_str());
        offNode.append_attribute("y").set_value(std::to_string(yOffsetEMU).c_str());
        
        auto extNode = xfrmNode.append_child("a:ext");
        extNode.append_attribute("cx").set_value(std::to_string(widthEMU).c_str()); 
        extNode.append_attribute("cy").set_value(std::to_string(heightEMU).c_str());

        // Shape geometry
        auto prstGeomNode = spPrNode.append_child("a:prstGeom");
        prstGeomNode.append_attribute("prst").set_value("rect");
        prstGeomNode.append_child("a:avLst");

        // No fill for line
        auto lnNode = spPrNode.append_child("a:ln");
        lnNode.append_child("a:noFill");

        // Add client data
        anchorNode.append_child("xdr:clientData");

        return true;
    }
    catch (const std::exception& e) {
        throw XLException("Error adding absolute positioned image to drawing: " + std::string(e.what()));
    }
    
    return false;
}

bool XLDrawingXML::addImagePageAnchored(const std::string& relID,
                                       int64_t xOffsetEMU,
                                       int64_t yOffsetEMU,
                                       int64_t widthEMU,
                                       int64_t heightEMU)
{
    try {
        initializeDrawingXML();

        auto wsDrNode = xmlDocument().document_element();
        if (!wsDrNode || std::string(wsDrNode.name()) != "xdr:wsDr") {
            return false; 
        }

        uint32_t newDrawingID = getNextDrawingID();
        std::string drawingName = "Picture " + std::to_string(newDrawingID);

        // Create twoCellAnchor with absolute positioning
        auto anchorNode = wsDrNode.append_child("xdr:twoCellAnchor");
        anchorNode.append_attribute("editAs").set_value("absolute");

        // Create 'from' element specifying starting position
        auto from = anchorNode.append_child("xdr:from");
        from.append_child("xdr:col").text().set(0);
        from.append_child("xdr:colOff").text().set(xOffsetEMU);
        from.append_child("xdr:row").text().set(0);
        from.append_child("xdr:rowOff").text().set(yOffsetEMU);
        
        // Create 'to' element specifying size
        auto to = anchorNode.append_child("xdr:to");
        to.append_child("xdr:col").text().set(0);
        to.append_child("xdr:colOff").text().set(xOffsetEMU + widthEMU);
        to.append_child("xdr:row").text().set(0);
        to.append_child("xdr:rowOff").text().set(yOffsetEMU + heightEMU);

        auto picNode = anchorNode.append_child("xdr:pic");

        auto nvPicPrNode = picNode.append_child("xdr:nvPicPr");
        auto cNvPrNode = nvPicPrNode.append_child("xdr:cNvPr");
        cNvPrNode.append_attribute("id").set_value(newDrawingID);
        cNvPrNode.append_attribute("name").set_value(drawingName.c_str());
        cNvPrNode.append_attribute("descr").set_value("Page Anchored Image");
        
        auto cNvPicPrChildNode = nvPicPrNode.append_child("xdr:cNvPicPr");
        // Reduce locking to allow image movement and resize
        auto picLocksNode = cNvPicPrChildNode.append_child("a:picLocks");
        picLocksNode.append_attribute("noChangeAspect").set_value(1);

        auto blipFillNode = picNode.append_child("xdr:blipFill");
        auto blipNode = blipFillNode.append_child("a:blip");
        blipNode.append_attribute("xmlns:r") = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";
        blipNode.append_attribute("r:embed").set_value(relID.c_str());

        auto stretchNode = blipFillNode.append_child("a:stretch");
        stretchNode.append_child("a:fillRect");

        auto spPrNode = picNode.append_child("xdr:spPr");
        auto xfrmNode = spPrNode.append_child("a:xfrm");
        
        auto offChildNode = xfrmNode.append_child("a:off");
        offChildNode.append_attribute("x").set_value(std::to_string(xOffsetEMU).c_str());
        offChildNode.append_attribute("y").set_value(std::to_string(yOffsetEMU).c_str());
        
        auto extChildNode = xfrmNode.append_child("a:ext");
        extChildNode.append_attribute("cx").set_value(std::to_string(widthEMU).c_str()); 
        extChildNode.append_attribute("cy").set_value(std::to_string(heightEMU).c_str());

        auto prstGeomNode = spPrNode.append_child("a:prstGeom");
        prstGeomNode.append_attribute("prst").set_value("rect");
        prstGeomNode.append_child("a:avLst");

        auto lnNode = spPrNode.append_child("a:ln");
        lnNode.append_child("a:noFill");

        // Set client data attributes
        auto clientDataNode = anchorNode.append_child("xdr:clientData");
        clientDataNode.append_attribute("fPrint").set_value(1);  // Show image when printing

        return true;
    }
    catch (const std::exception& e) {
        throw XLException("Error adding page-anchored image to drawing: " + std::string(e.what()));
    }
    
    return false;
} 
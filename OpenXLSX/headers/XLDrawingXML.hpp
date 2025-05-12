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

#ifndef OPENXLSX_XLDRAWINGXML_HPP
#define OPENXLSX_XLDRAWINGXML_HPP

#include "OpenXLSX-Exports.hpp"
#include "XLXmlFile.hpp"

#include <string>
#include <cstdint>

namespace OpenXLSX
{
    class XLWorksheet;

    /**
     * @brief A class for handling Excel drawing XML files, which are used for images and charts.
     */
    class OPENXLSX_EXPORT XLDrawingXML : public XLXmlFile
    {
    public:
        /**
         * @brief Default constructor
         */
        XLDrawingXML() : XLXmlFile(nullptr) {}

        /**
         * @brief Constructor with XML data
         * @param xmlData XML data for the drawing file
         */
        explicit XLDrawingXML(XLXmlData* xmlData);

        /**
         * @brief Copy constructor
         * @param other The XLDrawingXML object to copy
         */
        XLDrawingXML(const XLDrawingXML& other) = default;

        /**
         * @brief Move constructor
         * @param other The XLDrawingXML object to move
         */
        XLDrawingXML(XLDrawingXML&& other) noexcept = default;

        /**
         * @brief Destructor
         */
        ~XLDrawingXML() = default;

        /**
         * @brief Copy assignment operator
         * @param other The XLDrawingXML object to copy
         * @return Reference to this object
         */
        XLDrawingXML& operator=(const XLDrawingXML& other) = default;

        /**
         * @brief Move assignment operator
         * @param other The XLDrawingXML object to move
         * @return Reference to this object
         */
        XLDrawingXML& operator=(XLDrawingXML&& other) noexcept = default;

        /**
         * @brief Add an image anchored to cells
         * @param relID The relationship ID for the image
         * @param fromRow The row from which the image starts (0-based)
         * @param fromCol The column from which the image starts (0-based)
         * @param toRow The row to which the image extends (0-based)
         * @param toCol The column to which the image extends (0-based)
         * @return True if the operation was successful
         */
        bool addImage(const std::string& relID, 
                      uint32_t fromRow, 
                      uint16_t fromCol, 
                      uint32_t toRow, 
                      uint16_t toCol);

        /**
         * @brief Add an image with absolute positioning and sizing
         * @param relID The relationship ID for the image
         * @param xOffsetEMU The X offset in English Metric Units (EMUs)
         * @param yOffsetEMU The Y offset in English Metric Units (EMUs)
         * @param widthEMU The width in English Metric Units (EMUs)
         * @param heightEMU The height in English Metric Units (EMUs)
         * @return True if the operation was successful
         */
        bool addImageAbsolute(const std::string& relID,
                              int64_t xOffsetEMU,
                              int64_t yOffsetEMU,
                              int64_t widthEMU,
                              int64_t heightEMU);

        /**
         * @brief Add a page-anchored image that doesn't move with cells
         * @param relID The relationship ID for the image
         * @param xOffsetEMU The X offset in English Metric Units (EMUs)
         * @param yOffsetEMU The Y offset in English Metric Units (EMUs)
         * @param widthEMU The width in English Metric Units (EMUs)
         * @param heightEMU The height in English Metric Units (EMUs)
         * @return True if the operation was successful
         */
        bool addImagePageAnchored(const std::string& relID,
                                 int64_t xOffsetEMU,
                                 int64_t yOffsetEMU,
                                 int64_t widthEMU,
                                 int64_t heightEMU);

        /**
         * @brief Get the next available drawing ID
         * @return The next drawing ID to use
         */
        uint32_t getNextDrawingID() const;

    private:
        /**
         * @brief Initialize the drawing XML if it's empty
         */
        void initializeDrawingXML();
    };
} // namespace OpenXLSX

#endif // OPENXLSX_XLDRAWINGXML_HPP 
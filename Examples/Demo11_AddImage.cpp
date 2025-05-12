#include <iostream>
#include <OpenXLSX.hpp>

using namespace OpenXLSX;

int main() {
    std::cout << "Creating new XLDocument...\n";
    XLDocument doc;
    doc.create("Demo11_AddImage.xlsx");
    auto wks = doc.workbook().worksheet("Sheet1");
    
    // 添加一些单元格内容，以便能清楚地看到各个区域
    // 注意：每个图片应当有不同的位置和标识，这样能够确认每个添加方法是否正常工作
    XLCellValue val1("图片位置标记");
    XLCellValue val2("图片1: 单元格锚定");
    XLCellValue val3("图片2: 绝对位置锚定");
    XLCellValue val4("图片3应该在这里附近");
    
    wks.cell("A1").value() = val1;
    wks.cell("B2").value() = val2;
    wks.cell("D6").value() = val3;
    wks.cell("G10").value() = val4;
    
    std::cout << "Adding image to worksheet...\n";
    // 图片路径
    std::string imagePath = "/home/liufangyuan/Desktop/far_after.png"; 
    
    std::cout << "添加第一张图片 - 单元格锚定...\n";
    // 第一张图片 - 标准单元格锚定在B2
    if (wks.addImage(imagePath, "B2", 5, 5)) {
        std::cout << "图片1添加成功：单元格锚定(B2).\n";
    } else {
        std::cout << "Failed to add image to B2.\n";
    }

    std::cout << "添加第二张图片 - 绝对位置锚定...\n";
    // 第二张图片 - 使用绝对位置
    // 1 inch = 914400 EMUs. 1 pixel is approx 9525 EMUs at 96 DPI.
    int64_t xOffsetEMU = 1828800;  // 2 inch from the left
    int64_t yOffsetEMU = 1828800;  // 2 inches from the top
    int64_t widthEMU   = 1828800;  // 2 inches wide
    int64_t heightEMU  = 1828800;  // 2 inches high
    
    if (wks.addImageAbsolute(imagePath, xOffsetEMU, yOffsetEMU, widthEMU, heightEMU)) {
        std::cout << "图片2添加成功：绝对位置锚定.\n";
    } else {
        std::cout << "Failed to add image with absolute positioning.\n";
    }

    std::cout << "添加第三张图片 - 页面锚定...\n";
    // 第三张图片 - 使用页面锚定（修改位置，使其在G10附近，更容易在视图中看到）
    xOffsetEMU = 6096000;  // 约6.67英寸，对应G列
    yOffsetEMU = 2286000;  // 约2.5英寸，对应第10行
    widthEMU   = 1828800;  // 2 inches wide
    heightEMU  = 1828800;  // 2 inches high
    
    if (wks.addImagePageAnchored(imagePath, xOffsetEMU, yOffsetEMU, widthEMU, heightEMU)) {
        std::cout << "图片3添加成功：页面锚定.\n";
    } else {
        std::cout << "Failed to add image with page anchoring.\n";
    }

    std::cout << "Saving document...\n";
    doc.save();
    doc.close();
    std::cout << "Document saved and closed.\n";

    return 0;
} 
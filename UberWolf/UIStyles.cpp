/*
 *  File: UIStyles.cpp
 *  Copyright (c) 2025 vagmr
 *
 *  MIT License
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 */

#include "UIStyles.h"

// 设置控件的字体
void SetControlFont(HWND hWnd, int fontSize, bool isBold, const wchar_t* fontName)
{
    LOGFONT lf = { 0 };
    lf.lfHeight = -MulDiv(fontSize, GetDeviceCaps(GetDC(hWnd), LOGPIXELSY), 72);
    lf.lfWeight = isBold ? FW_BOLD : FW_NORMAL;
    lf.lfQuality = CLEARTYPE_QUALITY;
    wcscpy_s(lf.lfFaceName, fontName);
    
    HFONT hFont = CreateFontIndirect(&lf);
    SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
}

// 设置按钮的现代样式
void SetModernButtonStyle(HWND hButton)
{
    // 启用视觉样式
    SetWindowTheme(hButton, L"Explorer", NULL);
    
    // 设置按钮样式
    LONG_PTR style = GetWindowLongPtr(hButton, GWL_STYLE);
    style |= BS_FLAT;
    SetWindowLongPtr(hButton, GWL_STYLE, style);
    
    // 设置按钮字体
    SetControlFont(hButton, 9, true);
}

// 设置编辑框的现代样式
void SetModernEditStyle(HWND hEdit)
{
    // 启用视觉样式
    SetWindowTheme(hEdit, L"Explorer", NULL);
    
    // 设置编辑框字体
    SetControlFont(hEdit, 9, false);
}

// 设置标签的现代样式
void SetModernLabelStyle(HWND hLabel)
{
    // 设置标签字体
    SetControlFont(hLabel, 9, false);
}

// 自定义绘制拖放区域
void DrawDropZone(HWND hWnd, HDC hdc)
{
    RECT rect;
    GetClientRect(hWnd, &rect);
    
    // 创建圆角矩形区域
    int radius = 10;
    HBRUSH hBrush = CreateSolidBrush(UI_DROPZONE_BG_COLOR);
    HPEN hPen = CreatePen(PS_SOLID, 2, UI_DROPZONE_BORDER_COLOR);
    
    SelectObject(hdc, hBrush);
    SelectObject(hdc, hPen);
    
    // 绘制圆角矩形
    RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, radius, radius);
    
    // 绘制文本
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, UI_TEXT_COLOR);
    
    // 释放资源
    DeleteObject(hBrush);
    DeleteObject(hPen);
}

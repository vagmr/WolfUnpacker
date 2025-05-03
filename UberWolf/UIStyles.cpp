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

    // 设置按钮字体 - 使用更大的字体
    SetControlFont(hButton, 10, true);

    // 调整按钮大小，使其更高一些
    RECT rect;
    GetWindowRect(hButton, &rect);
    int width = rect.right - rect.left;
    int height = 32; // 固定高度为32像素

    // 将窗口坐标转换为客户区坐标
    POINT pt = { rect.left, rect.top };
    ScreenToClient(GetParent(hButton), &pt);

    // 设置新的位置和大小
    SetWindowPos(hButton, NULL, pt.x, pt.y, width, height, SWP_NOZORDER);
}

// 设置编辑框的现代样式
void SetModernEditStyle(HWND hEdit)
{
    // 启用视觉样式
    SetWindowTheme(hEdit, L"Explorer", NULL);

    // 设置编辑框字体
    SetControlFont(hEdit, 10, false);

    // 设置编辑框边框样式 - 使用扁平化的边框
    LONG_PTR style = GetWindowLongPtr(hEdit, GWL_EXSTYLE);
    style |= WS_EX_CLIENTEDGE;
    SetWindowLongPtr(hEdit, GWL_EXSTYLE, style);

    // 调整编辑框高度
    RECT rect;
    GetWindowRect(hEdit, &rect);
    int width = rect.right - rect.left;
    int height = 26; // 减小高度为26像素

    // 将窗口坐标转换为客户区坐标
    POINT pt = { rect.left, rect.top };
    ScreenToClient(GetParent(hEdit), &pt);

    // 设置新的位置和大小
    SetWindowPos(hEdit, NULL, pt.x, pt.y, width, height, SWP_NOZORDER);
}

// 设置标签的现代样式
void SetModernLabelStyle(HWND hLabel)
{
    // 设置标签字体
    SetControlFont(hLabel, 10, false);

    // 设置标签文本颜色
    // 注意：这需要在父窗口的WM_CTLCOLORSTATIC消息中处理
    // 这里我们只能设置字体
}

// 自定义绘制拖放区域
void DrawDropZone(HWND hWnd, HDC hdc)
{
    RECT rect;
    GetClientRect(hWnd, &rect);

    // 创建圆角矩形区域
    int radius = 12; // 增加圆角半径
    HBRUSH hBrush = CreateSolidBrush(UI_DROPZONE_BG_COLOR);
    HPEN hPen = CreatePen(PS_SOLID, 2, UI_DROPZONE_BORDER_COLOR);

    SelectObject(hdc, hBrush);
    SelectObject(hdc, hPen);

    // 绘制圆角矩形
    RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, radius, radius);

    // 添加一个轻微的阴影效果（简单模拟）
    HPEN hShadowPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
    SelectObject(hdc, hShadowPen);
    HBRUSH hShadowBrush = CreateSolidBrush(RGB(240, 240, 240));
    SelectObject(hdc, hShadowBrush);
    RoundRect(hdc, rect.left + 2, rect.bottom - 2, rect.right + 2, rect.bottom + 2, radius, radius);

    // 恢复原来的画笔和画刷
    SelectObject(hdc, hPen);
    SelectObject(hdc, hBrush);

    // 绘制文本
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, UI_TEXT_COLOR);

    // 释放资源
    DeleteObject(hBrush);
    DeleteObject(hPen);
    DeleteObject(hShadowPen);
    DeleteObject(hShadowBrush);
}

/*
 *  File: CustomControls.cpp
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

#include "CustomControls.h"

// 自定义绘制按钮
LRESULT CALLBACK CustomButtonProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            // 获取按钮状态
            LRESULT state = SendMessage(hWnd, BM_GETSTATE, 0, 0);
            bool isPressed = (state & BST_PUSHED) != 0;
            bool isHovered = (state & BST_HOT) != 0;
            
            // 获取按钮文本
            wchar_t text[256] = {0};
            GetWindowText(hWnd, text, 256);
            
            // 获取按钮区域
            RECT rect;
            GetClientRect(hWnd, &rect);
            
            // 设置按钮颜色
            COLORREF bgColor = isPressed ? UI_BUTTON_HOVER_COLOR : (isHovered ? UI_BUTTON_HOVER_COLOR : UI_ACCENT_COLOR);
            
            // 创建圆角矩形区域
            int radius = 5;
            HBRUSH hBrush = CreateSolidBrush(bgColor);
            SelectObject(hdc, hBrush);
            
            // 绘制圆角矩形
            RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, radius, radius);
            
            // 绘制文本
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(255, 255, 255));
            
            // 居中绘制文本
            DrawText(hdc, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            
            // 释放资源
            DeleteObject(hBrush);
            
            EndPaint(hWnd, &ps);
            return 0;
        }
    }
    
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

// 拖放区域处理程序
LRESULT CALLBACK DropZoneProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    const HWND hParent = (HWND)dwRefData;
    static bool isHovered = false;

    switch (uMsg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            // 绘制拖放区域
            DrawDropZone(hWnd, hdc);
            
            // 获取文本
            wchar_t text[512] = {0};
            GetWindowText(hWnd, text, 512);
            
            // 获取客户区域
            RECT rect;
            GetClientRect(hWnd, &rect);
            
            // 设置文本颜色和背景模式
            SetTextColor(hdc, UI_TEXT_COLOR);
            SetBkMode(hdc, TRANSPARENT);
            
            // 设置字体
            HFONT hFont = CreateFont(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
            
            // 绘制文本
            DrawText(hdc, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_WORDBREAK);
            
            // 恢复原来的字体
            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);
            
            EndPaint(hWnd, &ps);
            return 0;
        }
        
        case WM_DROPFILES:
            if (WindowBase::ProcessMessage(hParent, GetDlgCtrlID(hWnd), uMsg, wParam, lParam))
                return TRUE;
            break;
            
        case WM_MOUSEMOVE:
            if (!isHovered)
            {
                isHovered = true;
                
                // 创建跟踪结构
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hWnd;
                tme.dwHoverTime = HOVER_DEFAULT;
                TrackMouseEvent(&tme);
                
                // 重绘控件
                InvalidateRect(hWnd, NULL, TRUE);
            }
            break;
            
        case WM_MOUSELEAVE:
            isHovered = false;
            
            // 重绘控件
            InvalidateRect(hWnd, NULL, TRUE);
            break;
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

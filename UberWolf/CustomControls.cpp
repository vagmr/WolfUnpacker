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
            bool isDisabled = !IsWindowEnabled(hWnd);

            // 获取按钮文本
            wchar_t text[256] = {0};
            GetWindowText(hWnd, text, 256);

            // 获取按钮区域
            RECT rect;
            GetClientRect(hWnd, &rect);

            // 设置按钮颜色
            COLORREF bgColor;
            if (isDisabled) {
                bgColor = RGB(180, 180, 190); // 禁用状态为灰色
            } else if (isPressed) {
                bgColor = UI_BUTTON_PRESSED_COLOR; // 按下状态
            } else if (isHovered) {
                bgColor = UI_BUTTON_HOVER_COLOR; // 悬停状态
            } else {
                bgColor = UI_ACCENT_COLOR; // 正常状态
            }

            // 创建圆角矩形区域
            int radius = 8; // 增加圆角半径

            // 创建一个内存DC用于双缓冲绘制
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

            // 填充背景
            HBRUSH bgBrush = CreateSolidBrush(UI_BACKGROUND_COLOR);
            FillRect(memDC, &rect, bgBrush);
            DeleteObject(bgBrush);

            // 创建按钮画刷和画笔
            HBRUSH hBrush = CreateSolidBrush(bgColor);
            SelectObject(memDC, hBrush);

            // 绘制圆角矩形
            RoundRect(memDC, rect.left, rect.top, rect.right, rect.bottom, radius, radius);

            // 如果按钮被按下，添加一个轻微的内阴影效果
            if (isPressed && !isDisabled) {
                HPEN shadowPen = CreatePen(PS_SOLID, 1, RGB(50, 50, 50));
                HPEN oldPen = (HPEN)SelectObject(memDC, shadowPen);

                // 绘制内阴影
                RoundRect(memDC, rect.left + 1, rect.top + 1, rect.right - 1, rect.bottom - 1, radius, radius);

                SelectObject(memDC, oldPen);
                DeleteObject(shadowPen);
            }
            // 如果按钮未被按下，添加一个轻微的阴影效果
            else if (!isDisabled) {
                // 绘制底部阴影
                HBRUSH shadowBrush = CreateSolidBrush(RGB(50, 50, 50));
                RECT shadowRect = { rect.left + 2, rect.bottom - 2, rect.right + 2, rect.bottom + 2 };
                FillRect(memDC, &shadowRect, shadowBrush);
                DeleteObject(shadowBrush);

                // 重新绘制按钮以覆盖部分阴影，创建立体效果
                SelectObject(memDC, hBrush);
                RoundRect(memDC, rect.left, rect.top, rect.right, rect.bottom, radius, radius);
            }

            // 绘制文本
            SetBkMode(memDC, TRANSPARENT);

            // 设置文本颜色
            if (isDisabled) {
                SetTextColor(memDC, RGB(150, 150, 150)); // 禁用状态文本颜色
            } else {
                SetTextColor(memDC, UI_BUTTON_TEXT_COLOR);
            }

            // 设置字体
            HFONT hFont = CreateFont(-14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            HFONT oldFont = (HFONT)SelectObject(memDC, hFont);

            // 居中绘制文本
            DrawText(memDC, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // 将内存DC的内容复制到窗口DC
            BitBlt(hdc, 0, 0, rect.right, rect.bottom, memDC, 0, 0, SRCCOPY);

            // 清理资源
            SelectObject(memDC, oldFont);
            DeleteObject(hFont);
            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);
            DeleteObject(hBrush);

            EndPaint(hWnd, &ps);
            return 0;
        }

        // 处理鼠标悬停
        case WM_MOUSEMOVE:
        {
            // 确保我们能收到鼠标离开消息
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hWnd;
            tme.dwHoverTime = 0;
            TrackMouseEvent(&tme);

            // 设置热点状态并重绘
            SendMessage(hWnd, BM_SETSTATE, TRUE, 0);
            InvalidateRect(hWnd, NULL, TRUE);
            return 0;
        }

        // 处理鼠标离开
        case WM_MOUSELEAVE:
        {
            // 清除热点状态并重绘
            SendMessage(hWnd, BM_SETSTATE, FALSE, 0);
            InvalidateRect(hWnd, NULL, TRUE);
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

            // 获取客户区域
            RECT rect;
            GetClientRect(hWnd, &rect);

            // 创建一个内存DC用于双缓冲绘制
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

            // 填充背景
            HBRUSH bgBrush = CreateSolidBrush(UI_BACKGROUND_COLOR);
            FillRect(memDC, &rect, bgBrush);
            DeleteObject(bgBrush);

            // 创建圆角矩形区域
            int radius = 15; // 更大的圆角半径

            // 设置拖放区域颜色 - 如果悬停则使用更亮的颜色
            COLORREF borderColor = isHovered ? RGB(99, 102, 241) : UI_DROPZONE_BORDER_COLOR;
            COLORREF bgColor = isHovered ? RGB(239, 246, 255) : UI_DROPZONE_BG_COLOR;

            // 创建画刷和画笔
            HBRUSH hBrush = CreateSolidBrush(bgColor);
            HPEN hPen = CreatePen(PS_SOLID, 2, borderColor);

            SelectObject(memDC, hBrush);
            SelectObject(memDC, hPen);

            // 绘制圆角矩形
            RoundRect(memDC, rect.left, rect.top, rect.right, rect.bottom, radius, radius);

            // 添加一个轻微的阴影效果
            HPEN shadowPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
            SelectObject(memDC, shadowPen);
            HBRUSH shadowBrush = CreateSolidBrush(RGB(240, 240, 240));
            SelectObject(memDC, shadowBrush);
            RoundRect(memDC, rect.left + 2, rect.bottom - 2, rect.right + 2, rect.bottom + 2, radius, radius);

            // 恢复原来的画笔和画刷
            SelectObject(memDC, hPen);
            SelectObject(memDC, hBrush);

            // 如果悬停，添加一个图标提示拖放
            if (isHovered) {
                // 绘制一个简单的下载/拖放图标
                HPEN iconPen = CreatePen(PS_SOLID, 2, RGB(79, 70, 229));
                SelectObject(memDC, iconPen);

                // 绘制箭头
                int centerX = rect.right / 2;
                int centerY = rect.bottom / 2 - 30; // 向上偏移一点
                int arrowSize = 15;

                // 绘制箭头主体
                MoveToEx(memDC, centerX, centerY - arrowSize, NULL);
                LineTo(memDC, centerX, centerY + arrowSize);

                // 绘制箭头头部
                MoveToEx(memDC, centerX - arrowSize / 2, centerY, NULL);
                LineTo(memDC, centerX, centerY + arrowSize);
                LineTo(memDC, centerX + arrowSize / 2, centerY);

                // 绘制接收框
                RoundRect(memDC, centerX - arrowSize, centerY + arrowSize + 5,
                          centerX + arrowSize, centerY + arrowSize * 2 + 5, 5, 5);

                DeleteObject(iconPen);
            }

            // 获取文本
            wchar_t text[512] = {0};
            GetWindowText(hWnd, text, 512);

            // 设置文本颜色和背景模式
            SetTextColor(memDC, UI_TEXT_COLOR);
            SetBkMode(memDC, TRANSPARENT);

            // 设置字体
            HFONT hFont = CreateFont(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            HFONT hOldFont = (HFONT)SelectObject(memDC, hFont);

            // 如果悬停状态，不显示文本，只显示图标
            if (!isHovered) {
                // 绘制文本
                DrawText(memDC, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_WORDBREAK);
            } else {
                // 在悬停状态下，绘制提示文本
                wchar_t hoverText[] = L"拖放游戏EXE或wolf文件到这里";
                DrawText(memDC, hoverText, -1, &rect, DT_CENTER | DT_BOTTOM | DT_SINGLELINE);
            }

            // 将内存DC的内容复制到窗口DC
            BitBlt(hdc, 0, 0, rect.right, rect.bottom, memDC, 0, 0, SRCCOPY);

            // 清理资源
            SelectObject(memDC, hOldFont);
            DeleteObject(hFont);
            DeleteObject(hPen);
            DeleteObject(shadowPen);
            DeleteObject(shadowBrush);
            DeleteObject(hBrush);
            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);

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

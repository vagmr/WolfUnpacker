/*
 *  File: UIStyles.h
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

#pragma once

#include <Windows.h>
#include <Commctrl.h>
#include <Uxtheme.h>
#include <vssym32.h>

// Link to the required libraries
#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "Comctl32.lib")

// UI Style Constants
#define UI_BACKGROUND_COLOR RGB(245, 245, 250)
#define UI_ACCENT_COLOR RGB(100, 120, 220)
#define UI_TEXT_COLOR RGB(50, 50, 50)
#define UI_BUTTON_HOVER_COLOR RGB(120, 140, 240)
#define UI_DROPZONE_BORDER_COLOR RGB(180, 190, 240)
#define UI_DROPZONE_BG_COLOR RGB(235, 240, 255)

// 设置控件的字体
void SetControlFont(HWND hWnd, int fontSize = 9, bool isBold = false, const wchar_t* fontName = L"Segoe UI");

// 设置按钮的现代样式
void SetModernButtonStyle(HWND hButton);

// 设置编辑框的现代样式
void SetModernEditStyle(HWND hEdit);

// 设置标签的现代样式
void SetModernLabelStyle(HWND hLabel);

// 自定义绘制拖放区域
void DrawDropZone(HWND hWnd, HDC hdc);

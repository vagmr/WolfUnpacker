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

// UI Style Constants - 更现代的配色方案
#define UI_BACKGROUND_COLOR RGB(248, 250, 252)  // 更亮的背景色
#define UI_ACCENT_COLOR RGB(79, 70, 229)        // 靛蓝色作为主色调
#define UI_TEXT_COLOR RGB(30, 41, 59)           // 深蓝灰色文本
#define UI_BUTTON_HOVER_COLOR RGB(99, 102, 241) // 浅靛蓝色作为悬停色
#define UI_BUTTON_PRESSED_COLOR RGB(67, 56, 202) // 深靛蓝色作为按下色
#define UI_BUTTON_TEXT_COLOR RGB(255, 255, 255) // 按钮文本为白色
#define UI_DROPZONE_BORDER_COLOR RGB(129, 140, 248) // 浅靛蓝色边框
#define UI_DROPZONE_BG_COLOR RGB(224, 231, 255)  // 非常浅的靛蓝色背景
#define UI_EDIT_BORDER_COLOR RGB(203, 213, 225) // 输入框边框颜色
#define UI_EDIT_FOCUS_COLOR RGB(99, 102, 241)   // 输入框焦点颜色
#define UI_LABEL_COLOR RGB(71, 85, 105)         // 标签文本颜色

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

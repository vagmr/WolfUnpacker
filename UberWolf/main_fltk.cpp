/*
 *  File: main_fltk.cpp
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

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <UberWolfLib.h>
#include <Utils.h>
#include <memory>
#include <iostream>

#include "FltkMainWindow.h"
#pragma execution_character_set("utf-8")
int main(int argc, char* argv[])
{
    try
    {
        // 初始化FLTK
        Fl::scheme("gtk+");
        Fl::visual(FL_DOUBLE | FL_INDEX);

        // 设置FLTK的外观 - 现代化配色
        Fl::set_color(FL_BACKGROUND_COLOR, 248, 249, 250);      // 浅灰背景
        Fl::set_color(FL_BACKGROUND2_COLOR, 255, 255, 255);     // 白色
        Fl::set_color(FL_FOREGROUND_COLOR, 33, 37, 41);         // 深灰文字
        Fl::set_color(FL_SELECTION_COLOR, 0, 123, 255);         // 蓝色选择

        // 设置字体
        Fl::set_font(FL_HELVETICA, "Arial");
        Fl::set_font(FL_HELVETICA_BOLD, "Arial Bold");

        // 创建主窗口
        std::unique_ptr<FltkMainWindow> mainWindow = std::make_unique<FltkMainWindow>(1000, 750, "UberWolf v0.5.0 - Wolf RPG Complete Toolkit");

        // 初始化窗口
        if (!mainWindow->Initialize())
        {
            fl_alert("Window initialization failed!\n\nPlease check:\n• FLTK libraries are properly installed\n• Localization files are available\n• Configuration files are accessible");
            return -1;
        }

        // 设置UberWolfLib的本地化查询函数
        try
        {
            UberWolfLib::RegisterLocQueryFunc([](const std::string& s) -> const tString& {
                return LOCT(s);
            });
        }
        catch (...)
        {
            // 如果本地化注册失败，继续运行但记录警告
            std::cout << "Warning: Failed to register localization function" << std::endl;
        }

        // 显示窗口
        mainWindow->ShowWindow();

        // 设置窗口居中
        int screen_w = Fl::w();
        int screen_h = Fl::h();
        int win_w = mainWindow->w();
        int win_h = mainWindow->h();
        mainWindow->position((screen_w - win_w) / 2, (screen_h - win_h) / 2);

        // 运行FLTK事件循环
        int result = Fl::run();

        // mainWindow会自动清理（unique_ptr）
        return result;
    }
    catch (const std::exception& e)
    {
        fl_alert("Application startup failed: %s\n\nPlease check your installation and try again.", e.what());
        return -1;
    }
    catch (...)
    {
        fl_alert("Application startup failed: Unknown error\n\nPlease check your installation and try again.");
        return -1;
    }
}

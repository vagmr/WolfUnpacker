/*
 *  File: FltkMainWindow.cpp
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

#include "FltkMainWindow.h"
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_message.H>
#include <FL/Fl_Shared_Image.H>
#include <filesystem>
#include <iostream>
#include <ctime>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <locale>
#include <codecvt>
#ifdef _WIN32
#include <windows.h>
#endif
#pragma execution_character_set("utf-8")

FltkMainWindow::FltkMainWindow(int w, int h, const char* title)
    : Fl_Double_Window(w, h, title)
    , m_currentLanguageId(15000) // Default to English
    , m_isProcessing(false)
{
    // 设置窗口属性
    resizable(this);
    size_range(1000, 700);

    // 初始化FLTK图像支持
    fl_register_images();

    // 设置窗口图标和样式
    color(FL_BACKGROUND_COLOR);
}

FltkMainWindow::~FltkMainWindow()
{
    // 等待所有工作线程完成
    for (auto& thread : m_workerThreads)
    {
        if (thread.joinable())
            thread.join();
    }

    saveSettings();
}

bool FltkMainWindow::Initialize()
{
    try
    {
        // 初始化本地化系统
        initializeLocalization();

        // 初始化各个组件
        initializeMenuBar();
        initializeTabs();
        initializeCommonComponents();

        // 加载设置
        loadSettings();

        // 更新本地化
        UpdateLocalization();

        // 设置拖放支持
        Fl::dnd_text_ops(1);

        // 初始状态
        setProcessingState(false);
        updateStatus(getLocalizedText("ready"));

        return true;
    }
    catch (const std::exception& e)
    {
        fl_alert("Initialization failed: %s", e.what());
        return false;
    }
}

void FltkMainWindow::ShowWindow()
{
    show();
}

// === 本地化系统初始化 ===
void FltkMainWindow::initializeLocalization()
{
    // 初始化语言映射
    m_languageMap[15000] = "en";
    m_languageMap[15001] = "cn";
    m_languageMap[15002] = "jp";
    m_languageMap[15003] = "ko";

    // 初始化本地化系统
    try
    {
        // 获取可用语言列表
        std::vector<uint16_t> langIDs = { 135, 136, 137, 138 }; // 对应资源ID

        for (size_t i = 0; i < langIDs.size() && i < 4; ++i)
        {
            int langId = 15000 + static_cast<int>(i);
            Localizer::LocMap locMap;

            // 从资源加载本地化数据
            if (Localizer::ReadLocalizationFromResource(langIDs[i], locMap))
            {
                std::string langCode = m_languageMap[langId];
                LOC_ADD_LANG(langCode, langIDs[i]);
            }
        }

        // 从文件加载额外的语言
        const auto fileLangList = Localizer::GetLangCodesFromFolder();
        for (const auto& lang : fileLangList)
        {
            std::string langCode = lang.second;
            LOC_ADD_LANG(langCode, -1);
        }

        // 初始化本地化系统
        LOC_INIT();
    }
    catch (const std::exception& e)
    {
        // 如果本地化初始化失败，使用默认英语
        std::cout << "Localization initialization failed: " << e.what() << std::endl;
    }
}

const char* FltkMainWindow::getLocalizedText(const std::string& key)
{
    try
    {
        // 使用LOC而不是LOCW来获取const char*
        return LOC(key);
    }
    catch (...)
    {
        // 如果本地化失败，返回键名
        return key.c_str();
    }
}

void FltkMainWindow::initializeMenuBar()
{
    // 创建菜单栏
    m_menuBar = std::make_unique<Fl_Menu_Bar>(0, 0, w(), 30);
    m_menuBar->box(FL_FLAT_BOX);
    m_menuBar->color(FL_BACKGROUND_COLOR);

    // 添加文件菜单
    m_menuBar->add("File/Open Game...", FL_CTRL + 'o', selectGameCallback, this);
    m_menuBar->add("File/Open Project...", FL_CTRL + 'p', selectProjectCallback, this);
    m_menuBar->add("File/-", 0, 0, 0, FL_MENU_DIVIDER);
    m_menuBar->add("File/Exit", FL_CTRL + 'q', [](Fl_Widget*, void*) { exit(0); }, this);

    // 添加工具菜单
    m_menuBar->add("Tools/Decrypt Game", 0, decryptCallback, this);
    m_menuBar->add("Tools/Extract Translation", 0, extractCallback, this);
    m_menuBar->add("Tools/Apply Translation", 0, applyTranslationCallback, this);
    m_menuBar->add("Tools/Pack Game", 0, packCallback, this);

    // 添加语言菜单
    m_menuBar->add("Language/English", 0, languageCallback, this);
    m_menuBar->add("Language/中文", 0, languageCallback, this);
    m_menuBar->add("Language/日本語", 0, languageCallback, this);
    m_menuBar->add("Language/한국어", 0, languageCallback, this);

    // 添加帮助菜单
    m_menuBar->add("Help/About UberWolf", 0, [](Fl_Widget*, void*) {
        fl_message("UberWolf v0.5.0\n\nWolf RPG Complete Toolkit\nDecrypt → Translate → Pack\n\nDeveloped by vagmr");
    }, this);
}

void FltkMainWindow::initializeTabs()
{
    // 创建标签页容器 - 优化布局
    int tab_y = 35;  // 菜单栏下方
    int tab_h = h() - 140; // 为底部状态栏和日志区域留空间

    m_tabs = std::make_unique<Fl_Tabs>(10, tab_y, w() - 20, tab_h);
    m_tabs->box(FL_THIN_UP_BOX);
    m_tabs->selection_color(FL_BLUE);

    // 初始化各个标签页
    initializeDecryptTab();
    initializeTranslateTab();
    initializePackTab();
    initializeSettingsTab();

    m_tabs->end();
}

void FltkMainWindow::initializeDecryptTab()
{
    // 解密标签页 - 优化布局
    int tab_x = 15, tab_y = 60, tab_w = w() - 30, tab_h = h() - 145;
    m_decryptTab = std::make_unique<Fl_Group>(tab_x, tab_y, tab_w, tab_h, "🔓 Decrypt");
    m_decryptTab->begin();

    int content_x = tab_x + 10, content_y = tab_y + 30;
    int content_w = tab_w - 20;

    // 拖放区域 - 更大更明显
    m_dropArea = std::make_unique<Fl_Box>(content_x, content_y, content_w, 100,
        "🎮 Drop Game Files Here\n\nSupported: Game.exe, GamePro.exe, *.wolf files\nOr click 'Select Game' button below");
    m_dropArea->box(FL_DOWN_BOX);
    m_dropArea->color(FL_LIGHT2);
    m_dropArea->labelsize(12);
    m_dropArea->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);

    content_y += 120;

    // 游戏路径输入区域
    Fl_Box* gameLabel = new Fl_Box(content_x, content_y, 100, 25, "Game Path:");
    gameLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    gameLabel->labelfont(FL_BOLD);

    m_gamePathInput = std::make_unique<Fl_Input>(content_x + 105, content_y, content_w - 185, 25);
    m_gamePathInput->box(FL_DOWN_BOX);

    m_selectGameBtn = std::make_unique<Fl_Button>(content_x + content_w - 75, content_y, 75, 25, "Browse...");
    m_selectGameBtn->callback(selectGameCallback, this);
    m_selectGameBtn->color(FL_BACKGROUND2_COLOR);

    content_y += 40;

    // 保护密钥显示区域
    Fl_Box* keyLabel = new Fl_Box(content_x, content_y, 100, 25, "Protection Key:");
    keyLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    keyLabel->labelfont(FL_BOLD);

    m_protectionKeyOutput = std::make_unique<Fl_Output>(content_x + 105, content_y, content_w - 105, 25);
    m_protectionKeyOutput->box(FL_DOWN_BOX);
    m_protectionKeyOutput->color(FL_BACKGROUND2_COLOR);

    content_y += 40;

    // 选项复选框 - 更好的布局
    m_overwriteCheck = std::make_unique<Fl_Check_Button>(content_x, content_y, 180, 25, "Overwrite existing files");
    m_unprotectCheck = std::make_unique<Fl_Check_Button>(content_x + 200, content_y, 150, 25, "Remove protection");

    content_y += 30;
    m_decWolfXCheck = std::make_unique<Fl_Check_Button>(content_x, content_y, 180, 25, "Decrypt WolfX files");

    content_y += 50;

    // 解密按钮 - 更大更明显
    m_decryptBtn = std::make_unique<Fl_Button>(content_x, content_y, 120, 35, "🚀 Start Decrypt");
    m_decryptBtn->callback(decryptCallback, this);
    m_decryptBtn->color(FL_BLUE);
    m_decryptBtn->labelcolor(FL_WHITE);
    m_decryptBtn->labelfont(FL_BOLD);

    m_decryptTab->end();
}

void FltkMainWindow::initializeTranslateTab()
{
    // 翻译标签页 - 重点加入WolfTL功能
    int tab_x = 15, tab_y = 60, tab_w = w() - 30, tab_h = h() - 145;
    m_translateTab = std::make_unique<Fl_Group>(tab_x, tab_y, tab_w, tab_h, "🌐 Translate");
    m_translateTab->begin();

    int content_x = tab_x + 10, content_y = tab_y + 30;
    int content_w = tab_w - 20;

    // 项目路径区域
    Fl_Box* projectLabel = new Fl_Box(content_x, content_y, 100, 25, "Project Path:");
    projectLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    projectLabel->labelfont(FL_BOLD);

    m_projectPathInput = std::make_unique<Fl_Input>(content_x + 105, content_y, content_w - 185, 25);
    m_projectPathInput->box(FL_DOWN_BOX);

    m_selectProjectBtn = std::make_unique<Fl_Button>(content_x + content_w - 75, content_y, 75, 25, "Browse...");
    m_selectProjectBtn->callback(selectProjectCallback, this);
    m_selectProjectBtn->color(FL_BACKGROUND2_COLOR);

    content_y += 40;

    // WolfTL选项
    m_skipGameDatCheck = std::make_unique<Fl_Check_Button>(content_x, content_y, 200, 25, "Skip Game.dat processing");

    content_y += 35;

    // 翻译文件列表区域
    Fl_Box* filesLabel = new Fl_Box(content_x, content_y, 200, 20, "Translation Files:");
    filesLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    filesLabel->labelfont(FL_BOLD);

    content_y += 25;

    // 左侧：翻译文件列表
    int list_w = (content_w - 20) / 2;
    m_translationFilesList = std::make_unique<Fl_Browser>(content_x, content_y, list_w, 200);
    m_translationFilesList->box(FL_DOWN_BOX);
    m_translationFilesList->type(FL_MULTI_BROWSER);

    // 右侧：翻译统计信息
    int stats_x = content_x + list_w + 10;
    Fl_Box* statsLabel = new Fl_Box(stats_x, content_y - 25, 200, 20, "Translation Statistics:");
    statsLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    statsLabel->labelfont(FL_BOLD);

    m_translationStatsOutput = std::make_unique<Fl_Output>(stats_x, content_y, list_w, 200);
    m_translationStatsOutput->box(FL_DOWN_BOX);
    m_translationStatsOutput->color(FL_BACKGROUND2_COLOR);
    m_translationStatsOutput->type(FL_MULTILINE_OUTPUT);

    content_y += 220;

    // 操作按钮区域 - 更完整的WolfTL工作流
    m_extractBtn = std::make_unique<Fl_Button>(content_x, content_y, 140, 35, "📤 Extract Translation");
    m_extractBtn->callback(extractCallback, this);
    m_extractBtn->color(FL_GREEN);
    m_extractBtn->labelcolor(FL_WHITE);
    m_extractBtn->labelfont(FL_BOLD);

    m_applyTranslationBtn = std::make_unique<Fl_Button>(content_x + 150, content_y, 140, 35, "📥 Apply Translation");
    m_applyTranslationBtn->callback(applyTranslationCallback, this);
    m_applyTranslationBtn->color(FL_DARK_GREEN);
    m_applyTranslationBtn->labelcolor(FL_WHITE);
    m_applyTranslationBtn->labelfont(FL_BOLD);

    m_openTranslationBtn = std::make_unique<Fl_Button>(content_x + 300, content_y, 120, 35, "📁 Open Folder");
    m_openTranslationBtn->callback(openTranslationCallback, this);
    m_openTranslationBtn->color(FL_DARK_BLUE);
    m_openTranslationBtn->labelcolor(FL_WHITE);

    // 刷新按钮
    Fl_Button* refreshBtn = new Fl_Button(content_x + 430, content_y, 80, 35, "🔄 Refresh");
    refreshBtn->callback(refreshTranslationCallback, this);
    refreshBtn->color(FL_BACKGROUND2_COLOR);

    m_translateTab->end();
}

void FltkMainWindow::initializePackTab()
{
    // 打包标签页 - 优化布局
    int tab_x = 15, tab_y = 60, tab_w = w() - 30, tab_h = h() - 145;
    m_packTab = std::make_unique<Fl_Group>(tab_x, tab_y, tab_w, tab_h, "📦 Pack");
    m_packTab->begin();

    int content_x = tab_x + 10, content_y = tab_y + 30;
    int content_w = tab_w - 20;

    // 输出路径区域
    Fl_Box* outputLabel = new Fl_Box(content_x, content_y, 100, 25, "Output Path:");
    outputLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    outputLabel->labelfont(FL_BOLD);

    m_outputPathInput = std::make_unique<Fl_Input>(content_x + 105, content_y, content_w - 185, 25);
    m_outputPathInput->box(FL_DOWN_BOX);

    m_selectOutputBtn = std::make_unique<Fl_Button>(content_x + content_w - 75, content_y, 75, 25, "Browse...");
    m_selectOutputBtn->callback(selectOutputCallback, this);
    m_selectOutputBtn->color(FL_BACKGROUND2_COLOR);

    content_y += 50;

    // 加密方式选择
    Fl_Box* encLabel = new Fl_Box(content_x, content_y, 120, 25, "Encryption Type:");
    encLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    encLabel->labelfont(FL_BOLD);

    m_encryptionChoice = std::make_unique<Fl_Choice>(content_x + 125, content_y, 250, 25);
    m_encryptionChoice->box(FL_DOWN_BOX);
    // 从UberWolfLib获取加密方式列表
    try {
        auto encryptions = UberWolfLib::GetEncryptionsW();
        for (const auto& enc : encryptions) {
            std::string encStr(enc.begin(), enc.end());
            m_encryptionChoice->add(encStr.c_str());
        }
        if (m_encryptionChoice->size() > 0) {
            m_encryptionChoice->value(0);
        }
    } catch (...) {
        m_encryptionChoice->add("Default Encryption");
        m_encryptionChoice->value(0);
    }

    content_y += 50;

    // 打包选项
    m_createBackupCheck = std::make_unique<Fl_Check_Button>(content_x, content_y, 200, 25, "Create backup before packing");
    m_createBackupCheck->value(1); // 默认创建备份

    content_y += 50;

    // 打包按钮
    m_packBtn = std::make_unique<Fl_Button>(content_x, content_y, 120, 35, "📦 Start Pack");
    m_packBtn->callback(packCallback, this);
    m_packBtn->color(FL_MAGENTA);
    m_packBtn->labelcolor(FL_WHITE);
    m_packBtn->labelfont(FL_BOLD);

    m_packTab->end();
}

void FltkMainWindow::initializeSettingsTab()
{
    // 设置标签页 - 优化布局
    int tab_x = 15, tab_y = 60, tab_w = w() - 30, tab_h = h() - 145;
    m_settingsTab = std::make_unique<Fl_Group>(tab_x, tab_y, tab_w, tab_h, "⚙️ Settings");
    m_settingsTab->begin();

    int content_x = tab_x + 10, content_y = tab_y + 30;
    int content_w = tab_w - 20;

    // 语言设置区域
    Fl_Box* langLabel = new Fl_Box(content_x, content_y, 120, 25, "Interface Language:");
    langLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    langLabel->labelfont(FL_BOLD);

    m_languageChoice = std::make_unique<Fl_Choice>(content_x + 125, content_y, 200, 25);
    m_languageChoice->box(FL_DOWN_BOX);
    m_languageChoice->add("English");
    m_languageChoice->add("中文");
    m_languageChoice->add("日本語");
    m_languageChoice->add("한국어");
    m_languageChoice->callback(languageCallback, this);

    content_y += 60;

    // 关于信息区域
    Fl_Box* aboutLabel = new Fl_Box(content_x, content_y, 200, 25, "About UberWolf:");
    aboutLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    aboutLabel->labelfont(FL_BOLD);

    content_y += 30;

    m_aboutInfo = std::make_unique<Fl_Browser>(content_x, content_y, content_w, 300);
    m_aboutInfo->box(FL_DOWN_BOX);
    m_aboutInfo->color(FL_BACKGROUND2_COLOR);

    // 添加关于信息
    m_aboutInfo->add("🐺 UberWolf v0.5.0 - Wolf RPG Complete Toolkit");
    m_aboutInfo->add("");
    m_aboutInfo->add("📋 Features:");
    m_aboutInfo->add("  • 🔓 Decrypt Wolf RPG games (including WolfPro)");
    m_aboutInfo->add("  • 🌐 Extract and apply translations (WolfTL integration)");
    m_aboutInfo->add("  • 📦 Pack games with various encryption methods");
    m_aboutInfo->add("  • 🔧 Support for WolfX files");
    m_aboutInfo->add("");
    m_aboutInfo->add("🛠️ Technical Details:");
    m_aboutInfo->add("  • Built with FLTK for modern UI");
    m_aboutInfo->add("  • Supports multiple languages");
    m_aboutInfo->add("  • Cross-platform compatibility");
    m_aboutInfo->add("");
    m_aboutInfo->add("👨‍💻 Developed by: vagmr");
    m_aboutInfo->add("📄 License: MIT License");
    m_aboutInfo->add("🌟 GitHub: https://github.com/vagmr/UberWolf");

    m_settingsTab->end();
}

void FltkMainWindow::initializeCommonComponents()
{
    // 底部区域布局
    int bottom_y = h() - 105;  // 底部区域起始位置

    // 进度条
    m_progressBar = std::make_unique<Fl_Progress>(10, bottom_y, w() - 20, 25);
    m_progressBar->minimum(0);
    m_progressBar->maximum(100);
    m_progressBar->color(FL_BACKGROUND2_COLOR);
    m_progressBar->selection_color(FL_BLUE);
    m_progressBar->box(FL_DOWN_BOX);

    // 状态栏
    m_statusBar = std::make_unique<Fl_Box>(10, bottom_y + 30, w() - 20, 20, "Ready");
    m_statusBar->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    m_statusBar->box(FL_FLAT_BOX);
    m_statusBar->color(FL_BACKGROUND_COLOR);
    m_statusBar->labelfont(FL_HELVETICA);
    m_statusBar->labelsize(12);

    // 日志显示区域
    m_logBuffer = std::make_unique<Fl_Text_Buffer>();
    m_logDisplay = std::make_unique<Fl_Text_Display>(10, bottom_y + 55, w() - 20, 40);
    m_logDisplay->buffer(m_logBuffer.get());
    m_logDisplay->wrap_mode(1, 0);
    m_logDisplay->scrollbar_width(15);
    m_logDisplay->box(FL_DOWN_BOX);
    m_logDisplay->color(FL_BACKGROUND2_COLOR);
    m_logDisplay->textfont(FL_COURIER);
    m_logDisplay->textsize(11);

    // 添加初始日志信息
    addLogEntry("UberWolf v0.5.0 initialized successfully");
    addLogEntry("Ready for Wolf RPG operations: Decrypt → Translate → Pack");
}

void FltkMainWindow::UpdateLocalization()
{
    try
    {
        // 更新标签页标题
        if (m_decryptTab) m_decryptTab->label((std::string("🔓 ") + getLocalizedText("decrypt_tab")).c_str());
        if (m_translateTab) m_translateTab->label((std::string("🌐 ") + getLocalizedText("translate_tab")).c_str());
        if (m_packTab) m_packTab->label((std::string("📦 ") + getLocalizedText("pack_tab")).c_str());
        if (m_settingsTab) m_settingsTab->label((std::string("⚙️ ") + getLocalizedText("settings_tab")).c_str());

        // 更新菜单栏
        if (m_menuBar)
        {
            // 重新构建菜单以应用本地化
            m_menuBar->clear();

            // 文件菜单
            m_menuBar->add((std::string(getLocalizedText("file")) + "/" + getLocalizedText("open_game") + "...").c_str(),
                          FL_CTRL + 'o', selectGameCallback, this);
            m_menuBar->add((std::string(getLocalizedText("file")) + "/" + getLocalizedText("open_project") + "...").c_str(),
                          FL_CTRL + 'p', selectProjectCallback, this);
            m_menuBar->add((std::string(getLocalizedText("file")) + "/-").c_str(), 0, 0, 0, FL_MENU_DIVIDER);
            m_menuBar->add((std::string(getLocalizedText("file")) + "/" + getLocalizedText("exit")).c_str(),
                          FL_CTRL + 'q', [](Fl_Widget*, void*) { exit(0); }, this);

            // 工具菜单
            m_menuBar->add((std::string(getLocalizedText("tools")) + "/" + getLocalizedText("decrypt_game")).c_str(),
                          0, decryptCallback, this);
            m_menuBar->add((std::string(getLocalizedText("tools")) + "/" + getLocalizedText("extract_translation")).c_str(),
                          0, extractCallback, this);
            m_menuBar->add((std::string(getLocalizedText("tools")) + "/" + getLocalizedText("apply_translation")).c_str(),
                          0, applyTranslationCallback, this);
            m_menuBar->add((std::string(getLocalizedText("tools")) + "/" + getLocalizedText("pack_game")).c_str(),
                          0, packCallback, this);

            // 语言菜单
            m_menuBar->add((std::string(getLocalizedText("language")) + "/English").c_str(), 0, languageCallback, this);
            m_menuBar->add((std::string(getLocalizedText("language")) + "/中文").c_str(), 0, languageCallback, this);
            m_menuBar->add((std::string(getLocalizedText("language")) + "/日本語").c_str(), 0, languageCallback, this);
            m_menuBar->add((std::string(getLocalizedText("language")) + "/한국어").c_str(), 0, languageCallback, this);

            // 帮助菜单
            m_menuBar->add((std::string(getLocalizedText("help")) + "/" + getLocalizedText("about")).c_str(), 0,
                          [](Fl_Widget*, void*) {
                              fl_message("UberWolf v0.5.0\n\nWolf RPG Complete Toolkit\nDecrypt → Translate → Pack\n\nDeveloped by vagmr");
                          }, this);
        }

        // 更新状态栏
        updateStatus(getLocalizedText("ready"));

        // 刷新界面
        redraw();
    }
    catch (const std::exception& e)
    {
        // 如果本地化更新失败，使用默认文本
        std::cout << "Localization update failed: " << e.what() << std::endl;
    }
}

// 静态回调函数在FltkMainWindow_Events.cpp中实现

// 事件处理方法在FltkMainWindow_Events.cpp中实现

// onDecrypt方法在FltkMainWindow_Operations.cpp中实现

// 这些方法已经在单独的文件中实现了
// FltkMainWindow_WolfTL.cpp - WolfTL相关操作
// FltkMainWindow_Operations.cpp - 解密和打包操作

// 工具方法实现已移动到单独的文件中

// loadSettings和saveSettings在FltkMainWindow_Operations.cpp中实现

// getSystemLanguageCode在FltkMainWindow_Operations.cpp中实现

int FltkMainWindow::getLanguageIdFromCode(const std::string& code)
{
    if (code == "cn") return 15001;
    if (code == "jp") return 15002;
    if (code == "ko") return 15003;
    return 15000; // 默认英语
}

// === 拖放事件处理 ===
int FltkMainWindow::handle(int event)
{
    switch (event)
    {
    case FL_DND_ENTER:
    case FL_DND_DRAG:
    case FL_DND_RELEASE:
        return 1;

    case FL_PASTE:
        {
            std::string droppedText = Fl::event_text();
            if (!droppedText.empty())
            {
                onDropFile(droppedText);
            }
            return 1;
        }
    }

    return Fl_Double_Window::handle(event);
}

// === 工具方法实现 ===
void FltkMainWindow::addLogEntry(const std::string& message)
{
    std::lock_guard<std::mutex> lock(m_logMutex);

    if (m_logBuffer)
    {
        // 获取当前时间
        auto now = std::time(nullptr);
        std::tm tm;
#ifdef _WIN32
        localtime_s(&tm, &now);
#else
        tm = *std::localtime(&now);
#endif

        std::ostringstream timestamp;
        timestamp << "[" << std::put_time(&tm, "%H:%M:%S") << "] ";

        std::string fullMessage = timestamp.str() + message + "\n";
        m_logBuffer->append(fullMessage.c_str());

        // 滚动到底部
        if (m_logDisplay)
        {
            m_logDisplay->scroll(m_logBuffer->length(), 0);
            m_logDisplay->redraw();
        }
    }
}

void FltkMainWindow::updateProgress(int percentage, const std::string& message)
{
    if (m_progressBar)
    {
        m_progressBar->value(percentage);
        m_progressBar->redraw();
    }

    if (!message.empty())
    {
        addLogEntry(message);
        updateStatus(message);
    }

    Fl::check(); // 更新界面
}

void FltkMainWindow::updateStatus(const std::string& status)
{
    if (m_statusBar)
    {
        m_statusBar->label(status.c_str());
        m_statusBar->redraw();
    }
}

void FltkMainWindow::setProcessingState(bool processing)
{
    m_isProcessing = processing;
    setButtonsEnabled(!processing);

    if (processing)
    {
        updateStatus(getLocalizedText("processing"));
    }
    else
    {
        updateStatus(getLocalizedText("ready"));
        updateProgress(0);
    }
}

void FltkMainWindow::setButtonsEnabled(bool enabled)
{
    if (m_decryptBtn) enabled ? m_decryptBtn->activate() : m_decryptBtn->deactivate();
    if (m_extractBtn) enabled ? m_extractBtn->activate() : m_extractBtn->deactivate();
    if (m_applyTranslationBtn) enabled ? m_applyTranslationBtn->activate() : m_applyTranslationBtn->deactivate();
    if (m_packBtn) enabled ? m_packBtn->activate() : m_packBtn->deactivate();
    if (m_selectGameBtn) enabled ? m_selectGameBtn->activate() : m_selectGameBtn->deactivate();
    if (m_selectProjectBtn) enabled ? m_selectProjectBtn->activate() : m_selectProjectBtn->deactivate();
    if (m_selectOutputBtn) enabled ? m_selectOutputBtn->activate() : m_selectOutputBtn->deactivate();
}
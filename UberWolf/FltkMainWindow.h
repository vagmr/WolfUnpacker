/*
 *  File: FltkMainWindow.h
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

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Output.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>

#include <memory>
#include <map>
#include <string>
#include <functional>
#include <vector>
#include <thread>
#include <mutex>

#include "ConfigManager.h"
#include "Localizer.h"
#include <UberWolfLib.h>

/**
 * @brief FLTK版本的主窗口类 - UberWolf完整工具套件
 *
 * 替换原有的Win32 API实现，提供现代化的界面设计
 * 包含完整的Wolf RPG工作流：解密→翻译→打包
 */
class FltkMainWindow : public Fl_Double_Window
{
public:
    /**
     * @brief 构造函数
     * @param w 窗口宽度
     * @param h 窗口高度
     * @param title 窗口标题
     */
    FltkMainWindow(int w, int h, const char* title);

    /**
     * @brief 析构函数
     */
    virtual ~FltkMainWindow();

    /**
     * @brief 初始化窗口
     * @return 成功返回true
     */
    bool Initialize();

    /**
     * @brief 显示窗口
     */
    void ShowWindow();

    /**
     * @brief 更新本地化文本
     */
    void UpdateLocalization();

    /**
     * @brief 处理拖放事件
     */
    int handle(int event) override;

private:
    // === 主要窗口组件 ===
    std::unique_ptr<Fl_Menu_Bar> m_menuBar;
    std::unique_ptr<Fl_Tabs> m_tabs;

    // === 标签页 ===
    std::unique_ptr<Fl_Group> m_decryptTab;      // 解密标签页
    std::unique_ptr<Fl_Group> m_translateTab;    // 翻译标签页
    std::unique_ptr<Fl_Group> m_packTab;         // 打包标签页
    std::unique_ptr<Fl_Group> m_settingsTab;     // 设置标签页

    // === 解密标签页组件 ===
    std::unique_ptr<Fl_Box> m_dropArea;          // 拖放区域
    std::unique_ptr<Fl_Input> m_gamePathInput;   // 游戏路径输入
    std::unique_ptr<Fl_Button> m_selectGameBtn;  // 选择游戏按钮
    std::unique_ptr<Fl_Output> m_protectionKeyOutput; // 保护密钥显示
    std::unique_ptr<Fl_Input> m_protectionKeyInput;   // 保护密钥输入（兼容性）
    std::unique_ptr<Fl_Check_Button> m_overwriteCheck;  // 覆盖文件选项
    std::unique_ptr<Fl_Check_Button> m_unprotectCheck;  // 解除保护选项
    std::unique_ptr<Fl_Check_Button> m_decWolfXCheck;   // 解密WolfX选项
    std::unique_ptr<Fl_Button> m_decryptBtn;     // 解密按钮

    // === 翻译标签页组件 ===
    std::unique_ptr<Fl_Input> m_projectPathInput;       // 项目路径输入
    std::unique_ptr<Fl_Button> m_selectProjectBtn;      // 选择项目按钮
    std::unique_ptr<Fl_Browser> m_translationFilesList; // 翻译文件列表
    std::unique_ptr<Fl_Button> m_extractBtn;            // 提取翻译按钮
    std::unique_ptr<Fl_Button> m_applyTranslationBtn;   // 应用翻译按钮
    std::unique_ptr<Fl_Button> m_openTranslationBtn;    // 打开翻译文件夹按钮
    std::unique_ptr<Fl_Check_Button> m_skipGameDatCheck; // 跳过GameDat选项
    std::unique_ptr<Fl_Output> m_translationStatsOutput; // 翻译统计信息

    // === 打包标签页组件 ===
    std::unique_ptr<Fl_Choice> m_encryptionChoice;      // 加密方式选择
    std::unique_ptr<Fl_Input> m_outputPathInput;        // 输出路径输入
    std::unique_ptr<Fl_Button> m_selectOutputBtn;       // 选择输出路径按钮
    std::unique_ptr<Fl_Button> m_packBtn;               // 打包按钮
    std::unique_ptr<Fl_Check_Button> m_createBackupCheck; // 创建备份选项

    // === 设置标签页组件 ===
    std::unique_ptr<Fl_Choice> m_languageChoice;        // 语言选择
    std::unique_ptr<Fl_Browser> m_aboutInfo;            // 关于信息

    // === 通用组件 ===
    std::unique_ptr<Fl_Progress> m_progressBar;         // 进度条
    std::unique_ptr<Fl_Text_Display> m_logDisplay;      // 日志显示
    std::unique_ptr<Fl_Text_Buffer> m_logBuffer;        // 日志缓冲区
    std::unique_ptr<Fl_Box> m_statusBar;                // 状态栏

    // === 数据成员 ===
    std::map<int, std::string> m_languageMap;           // 语言映射
    int m_currentLanguageId;                            // 当前语言ID
    std::string m_currentGamePath;                      // 当前游戏路径
    std::string m_currentProjectPath;                   // 当前项目路径
    std::mutex m_logMutex;                              // 日志互斥锁
    std::vector<std::thread> m_workerThreads;           // 工作线程
    bool m_isProcessing;                                // 是否正在处理

    // === 初始化方法 ===
    void initializeMenuBar();
    void initializeTabs();
    void initializeDecryptTab();
    void initializeTranslateTab();
    void initializePackTab();
    void initializeSettingsTab();
    void initializeCommonComponents();
    void initializeLocalization();

    // === 事件处理方法 ===
    void onLanguageChanged(int langId);
    void onSelectGame();
    void onSelectProject();
    void onSelectOutput();
    void onDecrypt();
    void onExtractTranslation();
    void onApplyTranslation();
    void onOpenTranslationFolder();
    void onPack();
    void onDropFile(const std::string& filePath);
    void onRefreshTranslationFiles();

    // === WolfTL相关方法 ===
    void updateTranslationFilesList();
    void updateTranslationStats();
    bool validateTranslationProject();
    void showTranslationProgress(int progress, const std::string& message);

    // === 工具方法 ===
    void addLogEntry(const std::string& message);
    void updateProgress(int percentage, const std::string& message = "");
    void updateStatus(const std::string& status);
    void setButtonsEnabled(bool enabled);
    void setProcessingState(bool processing);
    std::string selectFile(const std::string& title, const std::string& filter);
    std::string selectDirectory(const std::string& title);
    void openFolder(const std::string& path);

    // === 静态回调函数 ===
    static void languageCallback(Fl_Widget* w, void* data);
    static void selectGameCallback(Fl_Widget* w, void* data);
    static void selectProjectCallback(Fl_Widget* w, void* data);
    static void selectOutputCallback(Fl_Widget* w, void* data);
    static void decryptCallback(Fl_Widget* w, void* data);
    static void extractCallback(Fl_Widget* w, void* data);
    static void applyTranslationCallback(Fl_Widget* w, void* data);
    static void openTranslationCallback(Fl_Widget* w, void* data);
    static void packCallback(Fl_Widget* w, void* data);
    static void refreshTranslationCallback(Fl_Widget* w, void* data);

    // === 配置和本地化 ===
    void loadSettings();
    void saveSettings();
    void initializeLanguages();
    std::string getSystemLanguageCode();
    int getLanguageIdFromCode(const std::string& code);
    const char* getLocalizedText(const std::string& key);
};

/*
 *  File: FltkMainWindow_Events.cpp
 *  Copyright (c) 2025 vagmr
 *
 *  事件处理和回调函数实现
 */

#include "FltkMainWindow.h"
#include <FL/fl_message.H>
#include <filesystem>
#include <thread>

// === 静态回调函数实现 ===
void FltkMainWindow::languageCallback(Fl_Widget* w, void* data)
{
    FltkMainWindow* window = static_cast<FltkMainWindow*>(data);
    if (window && window->m_languageChoice)
    {
        int selected = window->m_languageChoice->value();
        int langId = 15000 + selected;
        window->onLanguageChanged(langId);
    }
}

void FltkMainWindow::selectGameCallback(Fl_Widget* w, void* data)
{
    FltkMainWindow* window = static_cast<FltkMainWindow*>(data);
    if (window) window->onSelectGame();
}

void FltkMainWindow::selectProjectCallback(Fl_Widget* w, void* data)
{
    FltkMainWindow* window = static_cast<FltkMainWindow*>(data);
    if (window) window->onSelectProject();
}

void FltkMainWindow::selectOutputCallback(Fl_Widget* w, void* data)
{
    FltkMainWindow* window = static_cast<FltkMainWindow*>(data);
    if (window) window->onSelectOutput();
}

void FltkMainWindow::decryptCallback(Fl_Widget* w, void* data)
{
    FltkMainWindow* window = static_cast<FltkMainWindow*>(data);
    if (window) window->onDecrypt();
}

void FltkMainWindow::extractCallback(Fl_Widget* w, void* data)
{
    FltkMainWindow* window = static_cast<FltkMainWindow*>(data);
    if (window) window->onExtractTranslation();
}

void FltkMainWindow::applyTranslationCallback(Fl_Widget* w, void* data)
{
    FltkMainWindow* window = static_cast<FltkMainWindow*>(data);
    if (window) window->onApplyTranslation();
}

void FltkMainWindow::openTranslationCallback(Fl_Widget* w, void* data)
{
    FltkMainWindow* window = static_cast<FltkMainWindow*>(data);
    if (window) window->onOpenTranslationFolder();
}

void FltkMainWindow::packCallback(Fl_Widget* w, void* data)
{
    FltkMainWindow* window = static_cast<FltkMainWindow*>(data);
    if (window) window->onPack();
}

void FltkMainWindow::refreshTranslationCallback(Fl_Widget* w, void* data)
{
    FltkMainWindow* window = static_cast<FltkMainWindow*>(data);
    if (window) window->onRefreshTranslationFiles();
}

// === 事件处理方法实现 ===
void FltkMainWindow::onLanguageChanged(int langId)
{
    m_currentLanguageId = langId;

    if (m_languageMap.find(langId) != m_languageMap.end())
    {
        try
        {
            std::string langCode = m_languageMap[langId];
            LOC_LOAD(langCode);
            UpdateLocalization();

            ConfigManager::GetInstance().SetValue(0, "language", langId);
            addLogEntry("Language changed to: " + langCode);
        }
        catch (const std::exception& e)
        {
            addLogEntry("Failed to change language: " + std::string(e.what()));
        }
    }
}

void FltkMainWindow::onSelectGame()
{
    std::string filePath = selectFile("Select Game File", "Executable Files (*.exe)\tWolf Files (*.wolf)");
    if (!filePath.empty() && m_gamePathInput)
    {
        m_gamePathInput->value(filePath.c_str());
        m_currentGamePath = filePath;
        addLogEntry("Game file selected: " + filePath);

        // 自动检测保护密钥
        if (m_protectionKeyOutput)
        {
            m_protectionKeyOutput->value("Detecting...");
            m_protectionKeyOutput->redraw();

            // 在后台线程中检测保护密钥
            std::thread([this, filePath]() {
                try
                {
                    UberWolfLib uwl;
                    uwl.InitGame(std::filesystem::path(filePath));

                    std::wstring protKey;
                    UWLExitCode result = uwl.FindProtectionKey(protKey);

                    std::string keyStr;
                    if (result == UWLExitCode::SUCCESS)
                    {
                        keyStr = std::string(protKey.begin(), protKey.end());
                    }
                    else if (result == UWLExitCode::NOT_WOLF_PRO)
                    {
                        keyStr = "Not Protected";
                    }
                    else
                    {
                        keyStr = "Detection Failed";
                    }

                    // 在主线程中更新UI
                    Fl::awake([](void* data) {
                        auto* info = static_cast<std::pair<FltkMainWindow*, std::string>*>(data);
                        if (info->first && info->first->m_protectionKeyOutput)
                        {
                            info->first->m_protectionKeyOutput->value(info->second.c_str());
                            info->first->m_protectionKeyOutput->redraw();
                        }
                        delete info;
                    }, new std::pair<FltkMainWindow*, std::string>(this, keyStr));
                }
                catch (const std::exception& e)
                {
                    Fl::awake([](void* data) {
                        auto* window = static_cast<FltkMainWindow*>(data);
                        if (window && window->m_protectionKeyOutput)
                        {
                            window->m_protectionKeyOutput->value("Error");
                            window->m_protectionKeyOutput->redraw();
                        }
                    }, this);
                }
            }).detach();
        }
    }
}

void FltkMainWindow::onSelectProject()
{
    std::string dirPath = selectDirectory("Select Project Directory");
    if (!dirPath.empty() && m_projectPathInput)
    {
        m_projectPathInput->value(dirPath.c_str());
        m_currentProjectPath = dirPath;
        addLogEntry("Project directory selected: " + dirPath);

        // 刷新翻译文件列表
        updateTranslationFilesList();
        updateTranslationStats();
    }
}

void FltkMainWindow::onSelectOutput()
{
    std::string dirPath = selectDirectory("Select Output Directory");
    if (!dirPath.empty() && m_outputPathInput)
    {
        m_outputPathInput->value(dirPath.c_str());
        addLogEntry("Output directory selected: " + dirPath);
    }
}

void FltkMainWindow::onDropFile(const std::string& filePath)
{
    std::filesystem::path path(filePath);

    if (path.extension() == ".exe" || path.extension() == ".wolf")
    {
        // 游戏文件
        if (m_gamePathInput)
        {
            m_gamePathInput->value(filePath.c_str());
            m_currentGamePath = filePath;
            addLogEntry("Game file dropped: " + filePath);

            // 切换到解密标签页
            if (m_tabs) m_tabs->value(m_decryptTab.get());
        }
    }
    else if (std::filesystem::is_directory(path))
    {
        // 目录
        if (m_projectPathInput)
        {
            m_projectPathInput->value(filePath.c_str());
            m_currentProjectPath = filePath;
            addLogEntry("Project directory dropped: " + filePath);

            // 切换到翻译标签页
            if (m_tabs) m_tabs->value(m_translateTab.get());

            updateTranslationFilesList();
            updateTranslationStats();
        }
    }
}

void FltkMainWindow::onRefreshTranslationFiles()
{
    updateTranslationFilesList();
    updateTranslationStats();
    addLogEntry("Translation files list refreshed");
}

// === 文件选择工具方法 ===
std::string FltkMainWindow::selectFile(const std::string& title, const std::string& filter)
{
    Fl_Native_File_Chooser chooser;
    chooser.title(title.c_str());
    chooser.type(Fl_Native_File_Chooser::BROWSE_FILE);
    chooser.filter(filter.c_str());

    if (chooser.show() == 0)
    {
        return std::string(chooser.filename());
    }

    return "";
}

std::string FltkMainWindow::selectDirectory(const std::string& title)
{
    Fl_Native_File_Chooser chooser;
    chooser.title(title.c_str());
    chooser.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);

    if (chooser.show() == 0)
    {
        return std::string(chooser.filename());
    }

    return "";
}

void FltkMainWindow::openFolder(const std::string& path)
{
    if (std::filesystem::exists(path))
    {
#ifdef _WIN32
        std::string command = "explorer \"" + path + "\"";
        system(command.c_str());
#else
        std::string command = "xdg-open \"" + path + "\"";
        system(command.c_str());
#endif
        addLogEntry("Opened folder: " + path);
    }
    else
    {
        fl_alert("Folder does not exist: %s", path.c_str());
    }
}

/*
 *  File: FltkMainWindow_WolfTL.cpp
 *  Copyright (c) 2025 vagmr
 *
 *  WolfTL相关操作实现
 */

#include "FltkMainWindow.h"
#include <FL/fl_message.H>
#include <filesystem>
#include <thread>
#include <fstream>
#include <sstream>
#include <chrono>
#include <locale>
#include <codecvt>
// 暂时不使用WolfTLWrapper，直接使用WolfTL
#include "WolfTL.h"
#pragma execution_character_set("utf-8")
// === WolfTL相关方法实现 ===
void FltkMainWindow::onExtractTranslation()
{
    if (m_currentProjectPath.empty())
    {
        fl_alert("Please select a project directory first!");
        return;
    }

    if (m_isProcessing)
    {
        fl_alert("Another operation is in progress!");
        return;
    }

    setProcessingState(true);
    addLogEntry("Starting translation extraction...");

    // 在后台线程中执行提取操作
    std::thread([this]() {
        try
        {
            std::filesystem::path projectPath = m_currentProjectPath;
            std::filesystem::path outputPath = projectPath / "translation_output";

            // 创建输出目录
            std::filesystem::create_directories(outputPath);

            // 使用真正的WolfTL进行翻译提取
            bool skipGameDat = false;
            if (m_skipGameDatCheck) {
                skipGameDat = m_skipGameDatCheck->value();
            }

            WolfTL wolfTL(projectPath.wstring(), outputPath.wstring(), skipGameDat);

            if (!wolfTL.IsValid())
            {
                Fl::awake([](void* data) {
                    auto* window = static_cast<FltkMainWindow*>(data);
                    window->setProcessingState(false);
                    window->addLogEntry("Failed to initialize WolfTL! Check if the project contains valid Wolf RPG data.");
                    fl_alert("Failed to initialize translation tool!\n\nPlease ensure the selected directory contains:\n• Map files (*.mps)\n• Database files (*.dat)\n• CommonEvents.dat\n• Game.dat (if not skipped)");
                }, this);
                return;
            }

            // 设置进度回调
            wolfTL.SetProgressCallback([this](int progress, const tString& message) {
                std::string msg;
                #ifdef UNICODE
                    // 转换wstring到string
                    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                    msg = converter.to_bytes(message);
                #else
                    msg = message;
                #endif

                Fl::awake([](void* data) {
                    auto* info = static_cast<std::pair<FltkMainWindow*, std::pair<int, std::string>>*>(data);
                    info->first->updateProgress(info->second.first, info->second.second);
                    delete info;
                }, new std::pair<FltkMainWindow*, std::pair<int, std::string>>(this, {progress, msg}));
            });

            // 执行提取操作
            bool result = wolfTL.ExtractToJson();

            if (!result)
            {
                tString error = wolfTL.GetLastError();
                std::string errorMsg;
                #ifdef UNICODE
                    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                    errorMsg = converter.to_bytes(error);
                #else
                    errorMsg = error;
                #endif

                Fl::awake([](void* data) {
                    auto* info = static_cast<std::pair<FltkMainWindow*, std::string>*>(data);
                    info->first->addLogEntry("Extraction failed: " + info->second);
                    info->first->setProcessingState(false);
                    fl_alert("Extraction failed: %s", info->second.c_str());
                    delete info;
                }, new std::pair<FltkMainWindow*, std::string>(this, errorMsg));
                return;
            }

            // 完成
            Fl::awake([](void* data) {
                auto* window = static_cast<FltkMainWindow*>(data);
                window->updateProgress(100, "Translation extraction completed!");
                window->addLogEntry("Translation files extracted to: " + window->m_currentProjectPath + "/translation_output");
                window->setProcessingState(false);
                window->updateTranslationFilesList();
                window->updateTranslationStats();
            }, this);
        }
        catch (const std::exception& e)
        {
            Fl::awake([](void* data) {
                auto* info = static_cast<std::pair<FltkMainWindow*, std::string>*>(data);
                info->first->addLogEntry("Extraction failed: " + info->second);
                info->first->setProcessingState(false);
                fl_alert("Extraction failed: %s", info->second.c_str());
                delete info;
            }, new std::pair<FltkMainWindow*, std::string>(this, e.what()));
        }
    }).detach();
}

void FltkMainWindow::onApplyTranslation()
{
    if (m_currentProjectPath.empty())
    {
        fl_alert("Please select a project directory first!");
        return;
    }

    std::filesystem::path translationPath = std::filesystem::path(m_currentProjectPath) / "translation_output";
    if (!std::filesystem::exists(translationPath))
    {
        fl_alert("Translation files not found! Please extract translation files first.");
        return;
    }

    if (m_isProcessing)
    {
        fl_alert("Another operation is in progress!");
        return;
    }

    setProcessingState(true);
    addLogEntry("Starting translation application...");

    // 在后台线程中执行应用操作
    std::thread([this, translationPath]() {
        try
        {
            std::filesystem::path projectPath = m_currentProjectPath;

            // 使用真正的WolfTL应用翻译
            bool skipGameDat = false;
            if (m_skipGameDatCheck) {
                skipGameDat = m_skipGameDatCheck->value();
            }

            WolfTL wolfTL(projectPath.wstring(), translationPath.wstring(), skipGameDat);

            if (!wolfTL.IsValid())
            {
                Fl::awake([](void* data) {
                    auto* window = static_cast<FltkMainWindow*>(data);
                    window->setProcessingState(false);
                    window->addLogEntry("Failed to initialize WolfTL! Check if the project contains valid Wolf RPG data.");
                    fl_alert("Failed to initialize translation tool!\n\nPlease ensure the selected directory contains valid Wolf RPG data files.");
                }, this);
                return;
            }

            // 设置进度回调
            wolfTL.SetProgressCallback([this](int progress, const tString& message) {
                std::string msg;
                #ifdef UNICODE
                    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                    msg = converter.to_bytes(message);
                #else
                    msg = message;
                #endif

                Fl::awake([](void* data) {
                    auto* info = static_cast<std::pair<FltkMainWindow*, std::pair<int, std::string>>*>(data);
                    info->first->updateProgress(info->second.first, info->second.second);
                    delete info;
                }, new std::pair<FltkMainWindow*, std::pair<int, std::string>>(this, {progress, msg}));
            });

            // 执行应用翻译操作
            // inPlace = false 表示不覆盖原文件，而是创建新的翻译版本
            bool result = wolfTL.ApplyTranslations(false);

            if (!result)
            {
                tString error = wolfTL.GetLastError();
                std::string errorMsg;
                #ifdef UNICODE
                    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                    errorMsg = converter.to_bytes(error);
                #else
                    errorMsg = error;
                #endif

                Fl::awake([](void* data) {
                    auto* info = static_cast<std::pair<FltkMainWindow*, std::string>*>(data);
                    info->first->addLogEntry("Application failed: " + info->second);
                    info->first->setProcessingState(false);
                    fl_alert("Translation application failed: %s", info->second.c_str());
                    delete info;
                }, new std::pair<FltkMainWindow*, std::string>(this, errorMsg));
                return;
            }

            // 完成
            Fl::awake([](void* data) {
                auto* window = static_cast<FltkMainWindow*>(data);
                window->updateProgress(100, "Translation application completed!");
                window->addLogEntry("Translations successfully applied to game files.");
                window->setProcessingState(false);
            }, this);
        }
        catch (const std::exception& e)
        {
            Fl::awake([](void* data) {
                auto* info = static_cast<std::pair<FltkMainWindow*, std::string>*>(data);
                info->first->addLogEntry("Application failed: " + info->second);
                info->first->setProcessingState(false);
                fl_alert("Application failed: %s", info->second.c_str());
                delete info;
            }, new std::pair<FltkMainWindow*, std::string>(this, e.what()));
        }
    }).detach();
}

void FltkMainWindow::onOpenTranslationFolder()
{
    if (m_currentProjectPath.empty())
    {
        fl_alert("Please select a project directory first!");
        return;
    }

    std::filesystem::path translationPath = std::filesystem::path(m_currentProjectPath) / "translation_output";

    if (!std::filesystem::exists(translationPath))
    {
        // 创建翻译输出目录
        std::filesystem::create_directories(translationPath);
        addLogEntry("Created translation output directory: " + translationPath.string());
    }

    openFolder(translationPath.string());
}

void FltkMainWindow::updateTranslationFilesList()
{
    if (!m_translationFilesList || m_currentProjectPath.empty())
        return;

    m_translationFilesList->clear();

    std::filesystem::path translationPath = std::filesystem::path(m_currentProjectPath) / "translation_output";

    if (!std::filesystem::exists(translationPath))
    {
        m_translationFilesList->add("No translation files found");
        m_translationFilesList->add("Click 'Extract Translation' to create them");
        return;
    }

    try
    {
        int fileCount = 0;
        for (const auto& entry : std::filesystem::directory_iterator(translationPath))
        {
            if (entry.is_regular_file())
            {
                std::string filename = entry.path().filename().string();
                std::string extension = entry.path().extension().string();

                if (extension == ".json" || extension == ".txt" || extension == ".csv")
                {
                    // 添加文件大小信息
                    auto fileSize = std::filesystem::file_size(entry.path());
                    std::ostringstream oss;
                    oss << filename << " (" << fileSize << " bytes)";

                    m_translationFilesList->add(oss.str().c_str());
                    fileCount++;
                }
            }
        }

        if (fileCount == 0)
        {
            m_translationFilesList->add("No translation files found");
        }
        else
        {
            std::string summary = "Total: " + std::to_string(fileCount) + " translation files";
            m_translationFilesList->add(summary.c_str());
        }
    }
    catch (const std::exception& e)
    {
        m_translationFilesList->add("Error reading directory");
        addLogEntry("Error reading translation directory: " + std::string(e.what()));
    }

    m_translationFilesList->redraw();
}

void FltkMainWindow::updateTranslationStats()
{
    if (!m_translationStatsOutput || m_currentProjectPath.empty())
        return;

    std::filesystem::path projectPath = m_currentProjectPath;
    std::filesystem::path translationPath = projectPath / "translation_output";

    std::ostringstream stats;
    stats << "Translation Statistics:\n\n";

    // 首先检查项目是否有效
    try
    {
        bool skipGameDat = m_skipGameDatCheck ? m_skipGameDatCheck->value() : false;
        WolfTL wolfTL(projectPath.wstring(), translationPath.wstring(), skipGameDat);

        if (wolfTL.IsValid())
        {
            // 获取WolfTL统计信息
            auto translationStats = wolfTL.GetTranslationStats();

            stats << "Project Status: Valid Wolf RPG Project\n";
            stats << "Skip Game.dat: " << (skipGameDat ? "Yes" : "No") << "\n\n";

            stats << "Game Data Components:\n";
            for (const auto& [component, count] : translationStats)
            {
                std::string componentName;
                #ifdef UNICODE
                    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                    componentName = converter.to_bytes(component);
                #else
                    componentName = component;
                #endif
                stats << "• " << componentName << ": " << count << "\n";
            }
            stats << "\n";
        }
        else
        {
            stats << "Project Status: Invalid or No Wolf RPG Data\n";
            stats << "Please select a directory containing:\n";
            stats << "• Map files (*.mps)\n";
            stats << "• Database files (*.dat)\n";
            stats << "• CommonEvents.dat\n";
            if (!skipGameDat) stats << "• Game.dat\n";
            stats << "\n";
        }
    }
    catch (const std::exception& e)
    {
        stats << "Project Status: Error checking project\n";
        stats << "Error: " << e.what() << "\n\n";
    }

    // 检查翻译文件状态
    if (!std::filesystem::exists(translationPath))
    {
        stats << "Translation Files: Not extracted\n";
        stats << "Action: Click 'Extract Translation' to create them\n";
    }
    else
    {
        try
        {
            int totalFiles = 0;
            size_t totalSize = 0;
            int jsonFiles = 0;

            // 统计不同类型的翻译文件
            std::map<std::string, int> fileTypes;

            for (const auto& entry : std::filesystem::directory_iterator(translationPath))
            {
                if (entry.is_regular_file())
                {
                    std::string extension = entry.path().extension().string();
                    if (extension == ".json" || extension == ".txt" || extension == ".csv")
                    {
                        totalFiles++;
                        totalSize += std::filesystem::file_size(entry.path());
                        fileTypes[extension]++;

                        if (extension == ".json") jsonFiles++;
                    }
                }
            }

            stats << "Translation Files: " << totalFiles << " files\n";
            stats << "Total Size: " << (totalSize / 1024.0) << " KB\n";

            if (!fileTypes.empty())
            {
                stats << "File Types:\n";
                for (const auto& [ext, count] : fileTypes)
                {
                    stats << "  " << ext << ": " << count << " files\n";
                }
            }

            stats << "\nDirectory: " << translationPath.filename().string() << "\n\n";

            if (totalFiles > 0)
            {
                stats << "Available Actions:\n";
                stats << "✓ Apply translations to game\n";
                stats << "✓ Open translation folder\n";
                stats << "✓ Refresh file list\n";

                if (jsonFiles > 0)
                {
                    stats << "\nReady for translation!\n";
                    stats << "Edit the JSON files and apply changes.";
                }
            }
            else
            {
                stats << "No translation files found.\n";
                stats << "Click 'Extract Translation' first.";
            }
        }
        catch (const std::exception& e)
        {
            stats << "Error reading translation files: " << e.what() << "\n";
        }
    }

    m_translationStatsOutput->value(stats.str().c_str());
    m_translationStatsOutput->redraw();
}

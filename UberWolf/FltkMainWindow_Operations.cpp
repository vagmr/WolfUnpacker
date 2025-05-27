/*
 *  File: FltkMainWindow_Operations.cpp
 *  Copyright (c) 2025 vagmr
 *
 *  解密和打包操作实现
 */

#include "FltkMainWindow.h"
#include <FL/fl_message.H>
#include <filesystem>
#include <thread>
#include <iomanip>
#ifdef _WIN32
#include <windows.h>
#endif
#pragma execution_character_set("utf-8")
// === 解密操作实现 ===
void FltkMainWindow::onDecrypt()
{
    if (m_currentGamePath.empty())
    {
        fl_alert("Please select a game file first!");
        return;
    }

    if (m_isProcessing)
    {
        fl_alert("Another operation is in progress!");
        return;
    }

    setProcessingState(true);
    addLogEntry("Starting decryption process...");

    // 在后台线程中执行解密操作
    std::thread([this]() {
        try
        {
            std::filesystem::path exePath = m_currentGamePath;

            if (!std::filesystem::exists(exePath))
            {
                Fl::awake([](void* data) {
                    auto* window = static_cast<FltkMainWindow*>(data);
                    window->setProcessingState(false);
                    window->addLogEntry("Game file does not exist!");
                    fl_alert("Game file does not exist!");
                }, this);
                return;
            }

            // 使用UberWolfLib进行解密
            UberWolfLib uwl;
            bool overwrite = m_overwriteCheck ? m_overwriteCheck->value() : false;
            bool unprotect = m_unprotectCheck ? m_unprotectCheck->value() : false;
            bool decWolfX = m_decWolfXCheck ? m_decWolfXCheck->value() : false;

            uwl.Configure(overwrite, unprotect, decWolfX);
            uwl.InitGame(exePath);

            Fl::awake([](void* data) {
                auto* window = static_cast<FltkMainWindow*>(data);
                window->updateProgress(25, "Initializing decryption...");
            }, this);

            UWLExitCode result = uwl.UnpackData();
            if (result != UWLExitCode::SUCCESS)
            {
                std::string errorMsg = "Decryption failed with code: " + std::to_string(static_cast<int>(result));
                Fl::awake([](void* data) {
                    auto* info = static_cast<std::pair<FltkMainWindow*, std::string>*>(data);
                    info->first->setProcessingState(false);
                    info->first->addLogEntry(info->second);
                    fl_alert("Decryption failed!");
                    delete info;
                }, new std::pair<FltkMainWindow*, std::string>(this, errorMsg));
                return;
            }

            Fl::awake([](void* data) {
                auto* window = static_cast<FltkMainWindow*>(data);
                window->updateProgress(75, "Finding protection key...");
            }, this);

            // 查找保护密钥
            std::wstring protKey;
            result = uwl.FindProtectionKey(protKey);

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
                keyStr = "Key Detection Failed";
            }

            // 完成
            Fl::awake([](void* data) {
                auto* info = static_cast<std::pair<FltkMainWindow*, std::string>*>(data);
                info->first->updateProgress(100, "Decryption completed!");
                info->first->addLogEntry("Decryption process completed successfully.");
                info->first->addLogEntry("Protection key: " + info->second);

                if (info->first->m_protectionKeyOutput)
                {
                    info->first->m_protectionKeyOutput->value(info->second.c_str());
                    info->first->m_protectionKeyOutput->redraw();
                }

                info->first->setProcessingState(false);
                delete info;
            }, new std::pair<FltkMainWindow*, std::string>(this, keyStr));
        }
        catch (const std::exception& e)
        {
            Fl::awake([](void* data) {
                auto* info = static_cast<std::pair<FltkMainWindow*, std::string>*>(data);
                info->first->addLogEntry("Decryption error: " + info->second);
                info->first->setProcessingState(false);
                fl_alert("Decryption error: %s", info->second.c_str());
                delete info;
            }, new std::pair<FltkMainWindow*, std::string>(this, e.what()));
        }
    }).detach();
}

// === 打包操作实现 ===
void FltkMainWindow::onPack()
{
    if (m_currentGamePath.empty())
    {
        fl_alert("Please select a game file first!");
        return;
    }

    if (m_isProcessing)
    {
        fl_alert("Another operation is in progress!");
        return;
    }

    // 检查输出路径
    std::string outputPath;
    if (m_outputPathInput && m_outputPathInput->value() && strlen(m_outputPathInput->value()) > 0)
    {
        outputPath = m_outputPathInput->value();
    }
    else
    {
        // 使用默认输出路径
        std::filesystem::path gamePath = m_currentGamePath;
        outputPath = gamePath.parent_path().string() + "/packed";
    }

    setProcessingState(true);
    addLogEntry("Starting packing process...");

    // 在后台线程中执行打包操作
    std::thread([this, outputPath]() {
        try
        {
            std::filesystem::path exePath = m_currentGamePath;

            // 创建备份（如果选择）
            if (m_createBackupCheck && m_createBackupCheck->value())
            {
                Fl::awake([](void* data) {
                    auto* window = static_cast<FltkMainWindow*>(data);
                    window->updateProgress(10, "Creating backup...");
                }, this);

                std::filesystem::path backupPath = exePath.parent_path() / "backup";
                std::filesystem::create_directories(backupPath);

                // 复制重要文件到备份目录
                try
                {
                    if (std::filesystem::exists(exePath))
                        std::filesystem::copy_file(exePath, backupPath / exePath.filename(),
                                                 std::filesystem::copy_options::overwrite_existing);

                    // 复制数据文件
                    for (const auto& entry : std::filesystem::directory_iterator(exePath.parent_path()))
                    {
                        if (entry.is_regular_file())
                        {
                            std::string ext = entry.path().extension().string();
                            if (ext == ".wolf" || ext == ".dat")
                            {
                                std::filesystem::copy_file(entry.path(),
                                                          backupPath / entry.path().filename(),
                                                          std::filesystem::copy_options::overwrite_existing);
                            }
                        }
                    }
                }
                catch (const std::exception& e)
                {
                    Fl::awake([](void* data) {
                        auto* info = static_cast<std::pair<FltkMainWindow*, std::string>*>(data);
                        info->first->addLogEntry("Backup warning: " + info->second);
                        delete info;
                    }, new std::pair<FltkMainWindow*, std::string>(this, e.what()));
                }
            }

            Fl::awake([](void* data) {
                auto* window = static_cast<FltkMainWindow*>(data);
                window->updateProgress(30, "Initializing packing...");
            }, this);

            // 使用UberWolfLib进行打包
            UberWolfLib uwl;
            uwl.InitGame(exePath);

            int encryptionIndex = m_encryptionChoice ? m_encryptionChoice->value() : 0;

            Fl::awake([](void* data) {
                auto* window = static_cast<FltkMainWindow*>(data);
                window->updateProgress(60, "Packing game files...");
            }, this);

            // 执行打包
            UWLExitCode result = uwl.PackData(encryptionIndex);

            if (result != UWLExitCode::SUCCESS)
            {
                std::string errorMsg = "Packing failed with code: " + std::to_string(static_cast<int>(result));
                Fl::awake([](void* data) {
                    auto* info = static_cast<std::pair<FltkMainWindow*, std::string>*>(data);
                    info->first->setProcessingState(false);
                    info->first->addLogEntry(info->second);
                    fl_alert("Packing failed!");
                    delete info;
                }, new std::pair<FltkMainWindow*, std::string>(this, errorMsg));
                return;
            }

            // 移动打包结果到输出目录（如果指定）
            if (!outputPath.empty() && outputPath != exePath.parent_path().string())
            {
                Fl::awake([](void* data) {
                    auto* window = static_cast<FltkMainWindow*>(data);
                    window->updateProgress(90, "Moving files to output directory...");
                }, this);

                std::filesystem::create_directories(outputPath);

                // 移动打包后的文件
                try
                {
                    for (const auto& entry : std::filesystem::directory_iterator(exePath.parent_path()))
                    {
                        if (entry.is_regular_file())
                        {
                            std::string filename = entry.path().filename().string();
                            if (filename.find("_packed") != std::string::npos ||
                                filename.find("_encrypted") != std::string::npos)
                            {
                                std::filesystem::copy_file(entry.path(),
                                                          std::filesystem::path(outputPath) / filename,
                                                          std::filesystem::copy_options::overwrite_existing);
                            }
                        }
                    }
                }
                catch (const std::exception& e)
                {
                    Fl::awake([](void* data) {
                        auto* info = static_cast<std::pair<FltkMainWindow*, std::string>*>(data);
                        info->first->addLogEntry("File move warning: " + info->second);
                        delete info;
                    }, new std::pair<FltkMainWindow*, std::string>(this, e.what()));
                }
            }

            // 完成
            Fl::awake([](void* data) {
                auto* info = static_cast<std::pair<FltkMainWindow*, std::string>*>(data);
                info->first->updateProgress(100, "Packing completed!");
                info->first->addLogEntry("Game files packed successfully.");
                if (!info->second.empty())
                {
                    info->first->addLogEntry("Output directory: " + info->second);
                }
                info->first->setProcessingState(false);
                delete info;
            }, new std::pair<FltkMainWindow*, std::string>(this, outputPath));
        }
        catch (const std::exception& e)
        {
            Fl::awake([](void* data) {
                auto* info = static_cast<std::pair<FltkMainWindow*, std::string>*>(data);
                info->first->addLogEntry("Packing error: " + info->second);
                info->first->setProcessingState(false);
                fl_alert("Packing error: %s", info->second.c_str());
                delete info;
            }, new std::pair<FltkMainWindow*, std::string>(this, e.what()));
        }
    }).detach();
}

// === 配置管理实现 ===
void FltkMainWindow::loadSettings()
{
    // 加载语言设置
    int savedLangId = ConfigManager::GetInstance().GetValue(0, "language", -1);
    std::cout << "Loading settings: savedLangId=" << savedLangId << std::endl;

    if (savedLangId == -1)
    {
        // 首次启动 - 检测系统语言
        std::string systemLangCode = getSystemLanguageCode();
        std::cout << "First startup, detected system language: " << systemLangCode << std::endl;
        savedLangId = getLanguageIdFromCode(systemLangCode);
        std::cout << "Mapped to language ID: " << savedLangId << std::endl;
    }
    else if (savedLangId < 15000)
    {
        // 无效的保存值 - 使用默认值
        std::cout << "Invalid saved language ID, using default English" << std::endl;
        savedLangId = 15000; // 默认英语
    }

    std::cout << "Final language ID to load: " << savedLangId << std::endl;
    onLanguageChanged(savedLangId);

    // 更新语言选择控件
    if (m_languageChoice)
    {
        int choiceIndex = savedLangId - 15000;
        if (choiceIndex >= 0 && choiceIndex < 4)
        {
            m_languageChoice->value(choiceIndex);
        }
    }

    // 加载其他设置
    bool overwrite = ConfigManager::GetInstance().GetValue(0, "overwrite_files", false);
    if (m_overwriteCheck) m_overwriteCheck->value(overwrite);

    bool unprotect = ConfigManager::GetInstance().GetValue(0, "remove_protection", false);
    if (m_unprotectCheck) m_unprotectCheck->value(unprotect);

    bool decWolfX = ConfigManager::GetInstance().GetValue(0, "decrypt_wolfx", false);
    if (m_decWolfXCheck) m_decWolfXCheck->value(decWolfX);

    bool skipGameDat = ConfigManager::GetInstance().GetValue(0, "skip_gamedat", false);
    if (m_skipGameDatCheck) m_skipGameDatCheck->value(skipGameDat);

    bool createBackup = ConfigManager::GetInstance().GetValue(0, "create_backup", true);
    if (m_createBackupCheck) m_createBackupCheck->value(createBackup);
}

void FltkMainWindow::saveSettings()
{
    ConfigManager::GetInstance().SetValue(0, "language", m_currentLanguageId);

    // 修复：将FLTK控件的整数值转换为布尔值
    if (m_overwriteCheck)
        ConfigManager::GetInstance().SetValue(0, "overwrite_files", static_cast<bool>(m_overwriteCheck->value()));

    if (m_unprotectCheck)
        ConfigManager::GetInstance().SetValue(0, "remove_protection", static_cast<bool>(m_unprotectCheck->value()));

    if (m_decWolfXCheck)
        ConfigManager::GetInstance().SetValue(0, "decrypt_wolfx", static_cast<bool>(m_decWolfXCheck->value()));

    if (m_skipGameDatCheck)
        ConfigManager::GetInstance().SetValue(0, "skip_gamedat", static_cast<bool>(m_skipGameDatCheck->value()));

    if (m_createBackupCheck)
        ConfigManager::GetInstance().SetValue(0, "create_backup", static_cast<bool>(m_createBackupCheck->value()));
}

std::string FltkMainWindow::getSystemLanguageCode()
{
    // 完整的系统语言检测实现
#ifdef _WIN32
    // Windows系统语言检测
    LANGID langId = GetUserDefaultUILanguage();
    WORD primaryLang = PRIMARYLANGID(langId);
    WORD subLang = SUBLANGID(langId);

    // 添加调试信息
    std::cout << "System Language Detection: LANGID=" << langId
              << ", Primary=" << primaryLang << ", Sub=" << subLang << std::endl;

    switch (primaryLang)
    {
    case LANG_CHINESE:
        // 区分简体中文和繁体中文
        std::cout << "Detected Chinese language, sublang=" << subLang << std::endl;
        switch (subLang)
        {
        case SUBLANG_CHINESE_SIMPLIFIED:
        case SUBLANG_CHINESE_SINGAPORE:
            std::cout << "Returning Chinese Simplified (cn)" << std::endl;
            return "cn";
        case SUBLANG_CHINESE_TRADITIONAL:
        case SUBLANG_CHINESE_HONGKONG:
        case SUBLANG_CHINESE_MACAU:
            std::cout << "Returning Chinese Traditional (tw)" << std::endl;
            return "tw"; // 繁体中文，如果支持的话
        default:
            std::cout << "Returning Chinese Simplified (cn) as default" << std::endl;
            return "cn"; // 默认简体中文
        }
    case LANG_JAPANESE:
        std::cout << "Returning Japanese (jp)" << std::endl;
        return "jp";
    case LANG_KOREAN:
        std::cout << "Returning Korean (ko)" << std::endl;
        return "ko";
    case LANG_ENGLISH:
        std::cout << "Returning English (en)" << std::endl;
        return "en";
    default:
        // 尝试获取更详细的语言信息
        wchar_t localeName[LOCALE_NAME_MAX_LENGTH];
        if (GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH) > 0)
        {
            std::wstring localeWStr(localeName);
            std::string localeStr(localeWStr.begin(), localeWStr.end());

            // 转换为小写进行比较
            std::transform(localeStr.begin(), localeStr.end(), localeStr.begin(), ::tolower);

            if (localeStr.find("zh-cn") != std::string::npos ||
                localeStr.find("zh-sg") != std::string::npos) return "cn";
            if (localeStr.find("zh-tw") != std::string::npos ||
                localeStr.find("zh-hk") != std::string::npos) return "tw";
            if (localeStr.find("ja") != std::string::npos) return "jp";
            if (localeStr.find("ko") != std::string::npos) return "ko";
        }
        return "en"; // 默认英语
    }
#else
    // Linux/Unix系统语言检测
    const char* lang = getenv("LANG");
    if (!lang) lang = getenv("LC_ALL");
    if (!lang) lang = getenv("LC_MESSAGES");

    if (lang)
    {
        std::string langStr(lang);
        std::transform(langStr.begin(), langStr.end(), langStr.begin(), ::tolower);

        // 检测中文
        if (langStr.find("zh_cn") != std::string::npos ||
            langStr.find("zh-cn") != std::string::npos ||
            langStr.find("zh.utf8") != std::string::npos) return "cn";
        if (langStr.find("zh_tw") != std::string::npos ||
            langStr.find("zh-tw") != std::string::npos) return "tw";

        // 检测日语
        if (langStr.find("ja") != std::string::npos) return "jp";

        // 检测韩语
        if (langStr.find("ko") != std::string::npos) return "ko";

        // 检测英语
        if (langStr.find("en") != std::string::npos) return "en";
    }

    return "en"; // 默认英语
#endif
}

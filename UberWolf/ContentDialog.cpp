/*
 *  File: ContentDialog.cpp
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

#include "ContentDialog.h"
#include "UIStyles.h"
#include "CustomControls.h"
#include "Utils.h"
#include "Localizer.h"
#include "resource.h"
#include "WolfUtils.h"
#include <UberWolfLib.h>
#include <Shlwapi.h>
#include <Commctrl.h>
#include <shellapi.h>



ContentDialog::ContentDialog(const HINSTANCE hInstance, const HWND hWndParent) :
    WindowBase(hInstance, hWndParent),
    m_optionsDialog(hInstance, nullptr),
    m_packConfig(hInstance, nullptr),
    m_mutex()
{
    // 初始化通用控件
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    setHandle(CreateDialogParamW(m_hInstance, MAKEINTRESOURCE(IDD_CONTENT), m_hWndParent, wndProc, 0));
    registerLocalizedWindow();
    ShowWindow(hWnd(), SW_SHOW);

    // 设置对话框背景色
    SetClassLongPtr(hWnd(), GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(UI_BACKGROUND_COLOR));

    // Resize the main window to match the size of the embedded dialog
    RECT rectDialog;
    GetWindowRect(hWnd(), &rectDialog);
    int32_t dialogWidth  = rectDialog.right - rectDialog.left + 16; // I just gave up and added static values until it looked right
    int32_t dialogHeight = rectDialog.bottom - rectDialog.top + 59; // ... See above

    SetWindowPos(m_hWndParent, NULL, 0, 0, dialogWidth, dialogHeight, SWP_NOMOVE | SWP_NOZORDER);

    // 设置拖放区域的子类化过程
    SetWindowSubclass(GetDlgItem(hWnd(), IDC_LABEL_DROP_FILE), DropZoneProc, 0, (DWORD_PTR)hWnd());

    // 美化各个控件
    applyModernStyles();

    registerSlot(IDC_OPTIONS, BN_CLICKED, [this]() { onOptionsClicked(); });
    registerSlot(IDC_SELECT_GAME, BN_CLICKED, [this]() { onSelectGameClicked(); });
    registerSlot(IDC_UNPACK, BN_CLICKED, [this]() { onProcessClicked(); });
    registerSlot(IDC_PACK, BN_CLICKED, [this]() { onPackClicked(); });
    registerSlot(IDC_LABEL_DROP_FILE, WM_DROPFILES, [this](void* p) { onDropFile(p); });

    m_optionsDialog.SetParent(hWnd());
    m_packConfig.SetParent(hWnd());
    m_packConfig.Populate(UberWolfLib::GetEncryptionsW());

    // Register the Localizer GetValueW method as the query function for UberWolfLib
    UberWolfLib::RegisterLocQueryFunc([](const std::string& s) -> const tString& { return LOCT(s); });

    // Trigger a localization update to make sure that the window is properly localized
    updateLocalization();
}

ContentDialog::~ContentDialog()
{
    if (m_logIndex != -1)
        UberWolfLib::UnregisterLogCallback(m_logIndex);
}

void ContentDialog::SetupLog()
{
    m_logIndex = UberWolfLib::RegisterLogCallback([this](const std::wstring& entry, const bool& addNewline) { addLogEntry(entry, addNewline); });
}

void ContentDialog::applyModernStyles()
{
    // 设置按钮样式
    HWND hSelectGame = GetDlgItem(hWnd(), IDC_SELECT_GAME);
    HWND hProcess = GetDlgItem(hWnd(), IDC_UNPACK);
    HWND hPack = GetDlgItem(hWnd(), IDC_PACK);
    HWND hOptions = GetDlgItem(hWnd(), IDC_OPTIONS);

    // 应用自定义按钮样式
    SetWindowSubclass(hSelectGame, CustomButtonProc, 1, 0);
    SetWindowSubclass(hProcess, CustomButtonProc, 2, 0);
    SetWindowSubclass(hPack, CustomButtonProc, 3, 0);
    SetWindowSubclass(hOptions, CustomButtonProc, 4, 0);

    // 设置编辑框样式
    HWND hGameLocation = GetDlgItem(hWnd(), IDC_GAME_LOCATION);
    HWND hProtectionKey = GetDlgItem(hWnd(), IDC_PROTECTION_KEY);
    HWND hLog = GetDlgItem(hWnd(), IDC_LOG);

    SetModernEditStyle(hGameLocation);
    SetModernEditStyle(hProtectionKey);
    SetModernEditStyle(hLog);

    // 设置标签样式
    HWND hLabelGame = GetDlgItem(hWnd(), IDC_LABEL_GAME_LOCATION);
    HWND hLabelKey = GetDlgItem(hWnd(), IDC_LABEL_PROTECTION_KEY);
    HWND hLabelLog = GetDlgItem(hWnd(), IDC_LABEL_LOG);

    SetModernLabelStyle(hLabelGame);
    SetModernLabelStyle(hLabelKey);
    SetModernLabelStyle(hLabelLog);

    // 设置拖放区域样式
    HWND hDropZone = GetDlgItem(hWnd(), IDC_LABEL_DROP_FILE);
    // 初始化拖放功能
    DragAcceptFiles(hDropZone, TRUE);
    // 可以在这里添加其他样式设置
    LONG_PTR style = GetWindowLongPtr(hDropZone, GWL_STYLE);
    style |= SS_NOTIFY; // 确保标签能够接收鼠标消息
    SetWindowLongPtr(hDropZone, GWL_STYLE, style);

    // 调整控件间距和布局

    // 获取对话框客户区域
    RECT dlgRect;
    GetClientRect(hWnd(), &dlgRect);
    int dlgWidth = dlgRect.right - dlgRect.left;
    int dlgHeight = dlgRect.bottom - dlgRect.top;

    // 调整拖放区域大小和位置
    RECT dropRect;
    GetWindowRect(hDropZone, &dropRect);
    POINT dropPt = { dropRect.left, dropRect.top };
    ScreenToClient(hWnd(), &dropPt);
    int dropWidth = dlgWidth - 40; // 左右各留20像素的边距
    int dropHeight = 80; // 减小高度为80像素
    SetWindowPos(hDropZone, NULL, 20, 15, dropWidth, dropHeight, SWP_NOZORDER);

    // 调整标签和编辑框的位置
    RECT labelRect;
    GetWindowRect(hLabelGame, &labelRect);
    POINT labelPt = { labelRect.left, labelRect.top };
    ScreenToClient(hWnd(), &labelPt);

    // 调整游戏位置标签和编辑框
    int labelY = 15 + dropHeight + 15; // 拖放区域下方15像素
    SetWindowPos(hLabelGame, NULL, 20, labelY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    RECT editRect;
    GetWindowRect(hGameLocation, &editRect);
    POINT editPt = { editRect.left, editRect.top };
    ScreenToClient(hWnd(), &editPt);
    int editWidth = dlgWidth - 20 - 20 - 100 - 10; // 对话框宽度减去左右边距、按钮宽度和间距
    SetWindowPos(hGameLocation, NULL, 20, labelY + 20, editWidth, 26, SWP_NOZORDER); // 减小高度为26像素

    // 调整选择游戏按钮
    RECT btnRect;
    GetWindowRect(hSelectGame, &btnRect);
    POINT btnPt = { btnRect.left, btnRect.top };
    ScreenToClient(hWnd(), &btnPt);
    SetWindowPos(hSelectGame, NULL, 20 + editWidth + 10, labelY + 20, 100, 26, SWP_NOZORDER); // 按钮高度与编辑框一致

    // 调整保护密钥标签和编辑框
    int keyLabelY = labelY + 20 + 26 + 10; // 游戏位置编辑框下方10像素
    SetWindowPos(hLabelKey, NULL, 20, keyLabelY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    SetWindowPos(hProtectionKey, NULL, 20, keyLabelY + 20, dlgWidth - 40, 26, SWP_NOZORDER); // 减小高度为26像素

    // 调整日志标签
    int logLabelY = keyLabelY + 20 + 26 + 10; // 保护密钥编辑框下方10像素
    SetWindowPos(hLabelLog, NULL, 20, logLabelY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    // 调整日志编辑框
    int logY = logLabelY + 20;
    int logHeight = dlgHeight - logY - 15 - 32 - 10; // 对话框底部留出15像素，按钮高度32像素，按钮上方10像素
    SetWindowPos(hLog, NULL, 20, logY, dlgWidth - 40, logHeight, SWP_NOZORDER);

    // 调整按钮位置
    int btnY = dlgHeight - 15 - 32; // 对话框底部留出15像素，按钮高度32像素
    int btnSpacing = 10; // 按钮之间的间距
    int btnWidth = 100; // 按钮宽度

    // 从右到左排列按钮
    int rightX = dlgWidth - 20; // 最右边按钮的右边缘

    // 选项按钮
    SetWindowPos(hOptions, NULL, rightX - btnWidth, btnY, btnWidth, 32, SWP_NOZORDER);
    rightX -= (btnWidth + btnSpacing);

    // 打包按钮
    SetWindowPos(hPack, NULL, rightX - btnWidth, btnY, btnWidth, 32, SWP_NOZORDER);
    rightX -= (btnWidth + btnSpacing);

    // 处理/解包按钮
    SetWindowPos(hProcess, NULL, rightX - btnWidth, btnY, btnWidth, 32, SWP_NOZORDER);
}

void ContentDialog::updateLocalization()
{
    // Drop Label
    SetDlgItemText(hWnd(), IDC_LABEL_DROP_FILE, LOCW("drop_label"));

    // Labels
    SetDlgItemText(hWnd(), IDC_LABEL_GAME_LOCATION, LOCW("game_location"));
    SetDlgItemText(hWnd(), IDC_LABEL_PROTECTION_KEY, LOCW("protection_key"));

    // Buttons
    SetDlgItemText(hWnd(), IDC_SELECT_GAME, LOCW("select_game"));
    SetDlgItemText(hWnd(), IDC_UNPACK, LOCW("process"));
    SetDlgItemText(hWnd(), IDC_PACK, LOCW("pack"));
    SetDlgItemText(hWnd(), IDC_OPTIONS, LOCW("options"));

    adjustLabelEditComb(IDC_LABEL_GAME_LOCATION, IDC_GAME_LOCATION);
    adjustLabelEditComb(IDC_LABEL_PROTECTION_KEY, IDC_PROTECTION_KEY);
}

void ContentDialog::adjustButton(const int32_t& buttonID)
{
    HWND hButton = GetDlgItem(hWnd(), buttonID);
    RECT rect;
    GetWindowRect(hButton, &rect);

    int32_t newWidth = GetCaptionTextWidth(hButton) + 20;
    SetWindowPos(hButton, nullptr, 0, 0, newWidth, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
}

void ContentDialog::setButtonStates(const BOOL& en)
{
    // Set the "Process" button state
    EnableWindow(GetDlgItem(hWnd(), IDC_UNPACK), en);
    // Set the "Pack" button state
    EnableWindow(GetDlgItem(hWnd(), IDC_PACK), en);
}

void ContentDialog::adjustLabelEditComb(const int32_t& labelID, const int32_t& editID)
{
    HWND hLabel = GetDlgItem(hWnd(), labelID);
    // Get the old width
    RECT rect;
    RECT rectPos;
    GetWindowRect(hLabel, &rect);
    int32_t oldWidth = rect.right - rect.left;

    int32_t newWidth = GetCaptionTextWidth(hLabel);
    SetWindowPos(hLabel, nullptr, 0, 0, newWidth, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);

    HWND hEdit = GetDlgItem(hWnd(), editID);
    GetWindowRect(hEdit, &rect);
    GetWindowRect(hEdit, &rectPos);
    ScreenToClient(hWnd(), (LPPOINT)&rectPos);

    int32_t widthDiff = newWidth - oldWidth;
    int32_t editWidth = (rect.right - rect.left) - widthDiff;
    int32_t newX      = rectPos.left + widthDiff;

    SetWindowPos(hEdit, nullptr, newX, rectPos.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    SetWindowPos(hEdit, nullptr, 0, 0, editWidth, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
}

void ContentDialog::onOptionsClicked()
{
    m_optionsDialog.Show();
}

void ContentDialog::onPackClicked()
{
    const fs::path exePath = getExePath();

    if (exePath.empty())
        return;

    setButtonStates(FALSE);

    if (m_packConfig.Show())
    {
        // Pack the data
        UberWolfLib uwl;
        uwl.InitGame(exePath);
        uwl.Configure(m_optionsDialog.Overwrite());
        uwl.PackData(m_packConfig.GetSelectedIndex());
    }

    setButtonStates(TRUE);
}

void ContentDialog::onSelectGameClicked()
{
    wchar_t szFile[MAX_PATH] = { 0 };
    const wchar_t* filter = L"All Files (*.*)\0*.*\0";

    // 使用Utils中的OpenFile函数选择文件
    if (Utils::OpenFile(hWnd(), szFile, filter, LOCW("select_game_title")))
    {
        // 设置文件路径到编辑框
        SetWindowText(GetDlgItem(hWnd(), IDC_GAME_LOCATION), szFile);
    }
}

void ContentDialog::onProcessClicked()
{
    const fs::path exePath = getExePath();

    if (exePath.empty())
        return;

    const fs::path basePath = exePath.parent_path();
    const tString dataPath  = FS_PATH_TO_TSTRING(basePath) + TEXT("/") + GetWolfDataFolder();

    // Check if the data folder or data.wolf file exist
    if (!fs::exists(dataPath) && !ExistsWolfDataFile(basePath))
    {
        MessageBox(hWnd(), LOCW("error_msg_2"), LOCW("error"), MB_OK | MB_ICONERROR);
        return;
    }

    setButtonStates(FALSE);

    UberWolfLib uwl;
    uwl.Configure(m_optionsDialog.Overwrite(), m_optionsDialog.Unprotect());
    uwl.InitGame(exePath);
    std::wstring protKey;
    UWLExitCode result = uwl.UnpackData();

    if (result != UWLExitCode::SUCCESS)
    {
        MessageBox(hWnd(), LOCW("error_msg_3"), LOCW("error"), MB_OK | MB_ICONERROR);
        setButtonStates(TRUE);
        return;
    }

    result = uwl.FindProtectionKey(protKey);
    if (result != UWLExitCode::SUCCESS)
    {
        if (result == UWLExitCode::NOT_WOLF_PRO)
            protKey = LOCW("not_protected");
        else
        {
            MessageBox(hWnd(), LOCW("error_msg_4"), LOCW("error"), MB_OK | MB_ICONERROR);
            setButtonStates(TRUE);
            return;
        }
    }

    // Set the text of the protection key edit control to be the key
    SetDlgItemText(hWnd(), IDC_PROTECTION_KEY, protKey.c_str());

    setButtonStates(TRUE);
}

void ContentDialog::onDropFile(void* p)
{
    HDROP hDrop = static_cast<HDROP>(p);

    wchar_t szFile[MAX_PATH] = { 0 };

    // 获取拖放的文件数量
    const UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

    if (fileCount > 0)
    {
        // 只处理第一个文件
        DragQueryFile(hDrop, 0, szFile, MAX_PATH);

        // 设置文件路径到编辑框
        SetWindowText(GetDlgItem(hWnd(), IDC_GAME_LOCATION), szFile);
    }

    // 结束拖放操作
    DragFinish(hDrop);
}

void ContentDialog::addLogEntry(const std::wstring& entry, const bool& addNewline)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::wstring processedEntry = entry;

    // 替换特殊字符
    processedEntry = Utils::ReplaceAll(processedEntry, L"\r\n", L"\n");
    processedEntry = Utils::ReplaceAll(processedEntry, L"\r", L"\n");

    // 如果需要，添加换行符
    if (addNewline && !processedEntry.empty() && processedEntry.back() != L'\n')
        processedEntry += L"\n";

    // 获取日志控件
    HWND hLog = GetDlgItem(hWnd(), IDC_LOG);

    // 获取当前文本长度
    const DWORD len = GetWindowTextLength(hLog);

    // 选择文本末尾
    SendMessage(hLog, EM_SETSEL, len, len);

    // 添加新文本
    SendMessage(hLog, EM_REPLACESEL, FALSE, (LPARAM)processedEntry.c_str());

    // 滚动到末尾
    SendMessage(hLog, EM_SCROLLCARET, 0, 0);
}

fs::path ContentDialog::getExePath() const
{
    // Make sure that the user has selected a file
    WCHAR szFile[MAX_PATH];
    GetDlgItemText(hWnd(), IDC_GAME_LOCATION, szFile, MAX_PATH);

    if (szFile[0] == '\0')
    {
        MessageBox(hWnd(), LOCW("select_file"), LOCW("error"), MB_OK | MB_ICONERROR);
        return fs::path();
    }

    fs::path exePath = szFile;

    // Make sure that the file exists
    if (!fs::exists(exePath))
    {
        MessageBox(hWnd(), LOCW("error_msg_1"), LOCW("error"), MB_OK | MB_ICONERROR);
        return fs::path();
    }

    return exePath;
}

void ContentDialog::updateLoc(HWND hWnd)
{
    SetDlgItemText(hWnd, IDC_LABEL_DROP_FILE, LOCW("drop_label"));
    SetDlgItemText(hWnd, IDC_SELECT_GAME, LOCW("select_game"));
    SetDlgItemText(hWnd, IDC_UNPACK, LOCW("process"));
    SetDlgItemText(hWnd, IDC_PACK, LOCW("pack"));
    SetDlgItemText(hWnd, IDC_OPTIONS, LOCW("options"));
    SetDlgItemText(hWnd, IDC_LABEL_PROTECTION_KEY, LOCW("protection_key"));
    SetDlgItemText(hWnd, IDC_LABEL_GAME_LOCATION, LOCW("game_location"));
}

INT_PTR CALLBACK ContentDialog::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Check which message was passed in
    switch (uMsg)
    {
        case WM_COMMAND:
            if (WindowBase::ProcessCommand(hWnd, wParam))
                return TRUE;
            break;

        case WM_INITDIALOG:
            updateLoc(hWnd);
            return TRUE;

        case WM_CTLCOLORSTATIC:
        {
            HDC hdcStatic = (HDC)wParam;
            HWND hwndStatic = (HWND)lParam;

            // 获取控件ID
            int ctrlID = GetDlgCtrlID(hwndStatic);

            // 检查是否是标签控件
            if (ctrlID == IDC_LABEL_GAME_LOCATION ||
                ctrlID == IDC_LABEL_PROTECTION_KEY ||
                ctrlID == IDC_LABEL_LOG)
            {
                // 设置标签文本颜色
                SetTextColor(hdcStatic, UI_LABEL_COLOR);
                SetBkColor(hdcStatic, UI_BACKGROUND_COLOR);

                // 创建背景画刷
                static HBRUSH hBrush = CreateSolidBrush(UI_BACKGROUND_COLOR);
                return (INT_PTR)hBrush;
            }
            break;
        }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

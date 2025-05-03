/*
 *  File: Utils.cpp
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

#include "Utils.h"
#include <commdlg.h> // 添加GetOpenFileName所需的头文件

namespace Utils
{
    // 替换字符串中的所有指定子串
    std::wstring ReplaceAll(const std::wstring& str, const std::wstring& from, const std::wstring& to)
    {
        std::wstring result = str;
        size_t startPos = 0;
        while ((startPos = result.find(from, startPos)) != std::wstring::npos)
        {
            result.replace(startPos, from.length(), to);
            startPos += to.length();
        }

        return result;
    }
    
    // 打开文件选择对话框
    bool OpenFile(HWND hwndParent, LPTSTR lpstrFile, LPCTSTR lpstrFilter, LPCTSTR lpstrTitle)
    {
        OPENFILENAME ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwndParent;
        ofn.lpstrFile = lpstrFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = lpstrFilter;
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.lpstrTitle = lpstrTitle;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

        return GetOpenFileName(&ofn) == TRUE;
    }
}

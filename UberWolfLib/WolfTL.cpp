/*
 *  File: WolfTL.cpp
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

#include "WolfTL.h"
#include "WolfRPG/WolfRPGUtils.h"

#include <iostream>
#include <format>

WolfTL::WolfTL(const tString& dataPath, const tString& outputPath, const bool& skipGameDat)
    : m_dataPath(dataPath)
    , m_outputPath(outputPath)
    , m_wolf(dataPath, skipGameDat)
    , m_skipGameDat(skipGameDat)
    , m_progressCallback(nullptr)
{
}

bool WolfTL::ExtractToJson()
{
    if (!m_wolf.Valid())
    {
        setError(TEXT("WolfRPG initialization failed"));
        return false;
    }

    try
    {
        updateProgress(0, TEXT("Starting JSON extraction..."));

        // Extract each component with progress updates
        if (!extractMapsToJson()) return false;
        updateProgress(25, TEXT("Maps extracted"));

        if (!extractDatabasesToJson()) return false;
        updateProgress(50, TEXT("Databases extracted"));

        if (!extractCommonEventsToJson()) return false;
        updateProgress(75, TEXT("Common events extracted"));

        if (!extractGameDatToJson()) return false;
        updateProgress(100, TEXT("JSON extraction completed"));

        return true;
    }
    catch (const std::exception& e)
    {
        std::string errorMsg = e.what();
        setError(TEXT("Exception during JSON extraction: ") + tString(errorMsg.begin(), errorMsg.end()));
        return false;
    }
}

bool WolfTL::ApplyTranslations(bool inPlace)
{
    // Skip backup if not patching in-place
    wolfRPGUtils::g_skipBackup = !inPlace;

    if (!m_wolf.Valid())
    {
        setError(TEXT("WolfRPG initialization failed"));
        return false;
    }

    // Check if the patch folder exists
    if (!fs::exists(m_outputPath))
    {
        setError(TEXT("Patch folder does not exist: ") + m_outputPath);
        return false;
    }

    try
    {
        updateProgress(0, TEXT("Starting translation application..."));

        // Apply translations for each component
        if (!applyMapTranslations(m_outputPath)) return false;
        updateProgress(25, TEXT("Map translations applied"));

        if (!applyDatabaseTranslations(m_outputPath)) return false;
        updateProgress(50, TEXT("Database translations applied"));

        if (!applyCommonEventTranslations(m_outputPath)) return false;
        updateProgress(75, TEXT("Common event translations applied"));

        if (!applyGameDatTranslations(m_outputPath)) return false;
        updateProgress(90, TEXT("Game data translations applied"));

        // Save the patched data
        tString outputPath = inPlace ? m_dataPath : (m_outputPath + PATCHED_DATA);
        m_wolf.Save2File(outputPath);
        updateProgress(100, TEXT("Translation application completed"));

        return true;
    }
    catch (const std::exception& e)
    {
        std::string errorMsg = e.what();
        setError(TEXT("Exception during translation application: ") + tString(errorMsg.begin(), errorMsg.end()));
        return false;
    }
}

std::map<tString, size_t> WolfTL::GetTranslationStats() const
{
    std::map<tString, size_t> stats;

    if (!m_wolf.Valid())
        return stats;

    try
    {
        stats[TEXT("Maps")] = m_wolf.GetMaps().size();
        stats[TEXT("Databases")] = m_wolf.GetDatabases().size();
        stats[TEXT("CommonEvents")] = 1; // Always 1 CommonEvents file
        stats[TEXT("GameDat")] = m_skipGameDat ? 0 : 1;
    }
    catch (...)
    {
        // Return empty stats on error
    }

    return stats;
}

bool WolfTL::extractMapsToJson()
{
    try
    {
        const tString mapOutput = std::format(TEXT("{}/{}"), m_outputPath, MAP_OUTPUT);

        // Make sure the output folder exists
        fs::create_directories(mapOutput);

        for (const Map& map : m_wolf.GetMaps())
            map.ToJson(mapOutput);

        return true;
    }
    catch (const std::exception& e)
    {
        std::string errorMsg = e.what();
        setError(TEXT("Failed to extract maps to JSON: ") + tString(errorMsg.begin(), errorMsg.end()));
        return false;
    }
}

bool WolfTL::extractDatabasesToJson()
{
    try
    {
        const tString dbOutput = std::format(TEXT("{}/{}"), m_outputPath, DB_OUTPUT);

        // Make sure the output folder exists
        fs::create_directories(dbOutput);

        for (const Database& db : m_wolf.GetDatabases())
            db.ToJson(dbOutput);

        return true;
    }
    catch (const std::exception& e)
    {
        std::string errorMsg = e.what();
        setError(TEXT("Failed to extract databases to JSON: ") + tString(errorMsg.begin(), errorMsg.end()));
        return false;
    }
}

bool WolfTL::extractCommonEventsToJson()
{
    try
    {
        const tString comOutput = std::format(TEXT("{}/{}"), m_outputPath, COM_OUTPUT);

        // Make sure the output folder exists
        fs::create_directories(comOutput);

        m_wolf.GetCommonEvents().ToJson(comOutput);

        return true;
    }
    catch (const std::exception& e)
    {
        std::string errorMsg = e.what();
        setError(TEXT("Failed to extract common events to JSON: ") + tString(errorMsg.begin(), errorMsg.end()));
        return false;
    }
}

bool WolfTL::extractGameDatToJson()
{
    if (m_skipGameDat) return true;

    try
    {
        const tString gameDatOutput = std::format(TEXT("{}/{}"), m_outputPath, OUTPUT_DIR);

        m_wolf.GetGameDat().ToJson(gameDatOutput);

        return true;
    }
    catch (const std::exception& e)
    {
        std::string errorMsg = e.what();
        setError(TEXT("Failed to extract game data to JSON: ") + tString(errorMsg.begin(), errorMsg.end()));
        return false;
    }
}

bool WolfTL::applyMapTranslations(const tString& patchFolder)
{
    try
    {
        const tString mapPatch = std::format(TEXT("{}/{}"), patchFolder, MAP_OUTPUT);

        // Check if the patch folder exists
        if (!fs::exists(mapPatch))
        {
            // Not an error if no map patches exist
            return true;
        }

        for (Map& map : m_wolf.GetMaps())
            map.Patch(mapPatch);

        return true;
    }
    catch (const std::exception& e)
    {
        std::string errorMsg = e.what();
        setError(TEXT("Failed to apply map translations: ") + tString(errorMsg.begin(), errorMsg.end()));
        return false;
    }
}

bool WolfTL::applyDatabaseTranslations(const tString& patchFolder)
{
    try
    {
        const tString dbPatch = std::format(TEXT("{}/{}"), patchFolder, DB_OUTPUT);

        // Check if the patch folder exists
        if (!fs::exists(dbPatch))
        {
            // Not an error if no database patches exist
            return true;
        }

        for (Database& db : m_wolf.GetDatabases())
            db.Patch(dbPatch);

        return true;
    }
    catch (const std::exception& e)
    {
        std::string errorMsg = e.what();
        setError(TEXT("Failed to apply database translations: ") + tString(errorMsg.begin(), errorMsg.end()));
        return false;
    }
}

bool WolfTL::applyCommonEventTranslations(const tString& patchFolder)
{
    try
    {
        const tString comPatch = std::format(TEXT("{}/{}"), patchFolder, COM_OUTPUT);

        // Check if the patch folder exists
        if (!fs::exists(comPatch))
        {
            // Not an error if no common event patches exist
            return true;
        }

        m_wolf.GetCommonEvents().Patch(comPatch);

        return true;
    }
    catch (const std::exception& e)
    {
        std::string errorMsg = e.what();
        setError(TEXT("Failed to apply common event translations: ") + tString(errorMsg.begin(), errorMsg.end()));
        return false;
    }
}

bool WolfTL::applyGameDatTranslations(const tString& patchFolder)
{
    if (m_skipGameDat) return true;

    try
    {
        const tString gameDatPatch = std::format(TEXT("{}/{}"), patchFolder, OUTPUT_DIR);

        m_wolf.GetGameDat().Patch(gameDatPatch);

        return true;
    }
    catch (const std::exception& e)
    {
        std::string errorMsg = e.what();
        setError(TEXT("Failed to apply game data translations: ") + tString(errorMsg.begin(), errorMsg.end()));
        return false;
    }
}

void WolfTL::updateProgress(int progress, const tString& message)
{
    if (m_progressCallback)
        m_progressCallback(progress, message);
}

void WolfTL::setError(const tString& error)
{
    m_lastError = error;
}

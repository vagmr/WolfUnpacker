#pragma once

#include "WolfRPG/WolfRPG.h"
#include "Types.h"

#include <filesystem>
#include <functional>

namespace fs = std::filesystem;

/**
 * @brief WolfTL - Wolf RPG Translation Tool integrated into UberWolf
 * 
 * This class provides translation functionality for Wolf RPG games:
 * - Extract translatable text to JSON files
 * - Apply translations from JSON files back to game data
 * - Support for Maps, Databases, CommonEvents, and GameDat
 */
class WolfTL
{
public:
    /**
     * @brief Translation operation modes
     */
    enum class Mode
    {
        CREATE,     // Extract translatable data to JSON
        PATCH,      // Apply translations from JSON to new location
        PATCH_IP    // Apply translations in-place (overwrite original)
    };

    /**
     * @brief Progress callback function type
     * @param current Current progress (0-100)
     * @param message Status message
     */
    using ProgressCallback = std::function<void(int current, const tString& message)>;

    /**
     * @brief Constructor
     * @param dataPath Path to the Wolf RPG game data folder
     * @param outputPath Path for output (JSON files or patched data)
     * @param skipGameDat Whether to skip Game.dat processing
     */
    explicit WolfTL(const tString& dataPath, const tString& outputPath, const bool& skipGameDat = false);

    /**
     * @brief Destructor
     */
    ~WolfTL() = default;

    /**
     * @brief Check if WolfTL is valid and ready to use
     * @return true if valid, false otherwise
     */
    bool IsValid() const { return m_wolf.Valid(); }

    /**
     * @brief Set progress callback for operation feedback
     * @param callback Progress callback function
     */
    void SetProgressCallback(ProgressCallback callback) { m_progressCallback = callback; }

    /**
     * @brief Extract translatable data to JSON files
     * @return true if successful, false otherwise
     */
    bool ExtractToJson();

    /**
     * @brief Apply translations from JSON files
     * @param inPlace Whether to apply in-place (overwrite original data)
     * @return true if successful, false otherwise
     */
    bool ApplyTranslations(bool inPlace = false);

    /**
     * @brief Get the last error message
     * @return Error message string
     */
    const tString& GetLastError() const { return m_lastError; }

    /**
     * @brief Get translation statistics
     * @return Map of component names to translation counts
     */
    std::map<tString, size_t> GetTranslationStats() const;

private:
    // Output directory constants
    inline static const tString OUTPUT_DIR   = TEXT("dump/");
    inline static const tString MAP_OUTPUT   = OUTPUT_DIR + TEXT("mps/");
    inline static const tString DB_OUTPUT    = OUTPUT_DIR + TEXT("db/");
    inline static const tString COM_OUTPUT   = OUTPUT_DIR + TEXT("common/");
    inline static const tString PATCHED_DATA = TEXT("/patched/data/");

    /**
     * @brief Extract maps to JSON
     */
    bool extractMapsToJson();

    /**
     * @brief Extract databases to JSON
     */
    bool extractDatabasesToJson();

    /**
     * @brief Extract common events to JSON
     */
    bool extractCommonEventsToJson();

    /**
     * @brief Extract game data to JSON
     */
    bool extractGameDatToJson();

    /**
     * @brief Apply map translations
     * @param patchFolder Folder containing translation patches
     */
    bool applyMapTranslations(const tString& patchFolder);

    /**
     * @brief Apply database translations
     * @param patchFolder Folder containing translation patches
     */
    bool applyDatabaseTranslations(const tString& patchFolder);

    /**
     * @brief Apply common event translations
     * @param patchFolder Folder containing translation patches
     */
    bool applyCommonEventTranslations(const tString& patchFolder);

    /**
     * @brief Apply game data translations
     * @param patchFolder Folder containing translation patches
     */
    bool applyGameDatTranslations(const tString& patchFolder);

    /**
     * @brief Update progress and call callback if set
     * @param progress Progress percentage (0-100)
     * @param message Status message
     */
    void updateProgress(int progress, const tString& message);

    /**
     * @brief Set last error message
     * @param error Error message
     */
    void setError(const tString& error);

private:
    tString m_dataPath;         ///< Path to game data
    tString m_outputPath;       ///< Path for output
    WolfRPG m_wolf;             ///< Wolf RPG data handler
    bool m_skipGameDat;         ///< Whether to skip Game.dat
    
    ProgressCallback m_progressCallback;  ///< Progress callback
    tString m_lastError;                   ///< Last error message
};

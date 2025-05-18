#include <sqlite3.h>
#include <QApplication>
#include "mainwindow.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <cstdlib>      // Для system()
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

// Функция получения имени файла без расширений
std::string getBaseFilename(const std::string& path) {
    // Найти последний слэш или обратный слэш
    size_t slash = path.find_last_of("/\\");
    std::string filename = (slash == std::string::npos) ? path : path.substr(slash + 1);

    // Убрать расширение
    size_t dot = filename.find_last_of('.');
    return (dot == std::string::npos) ? filename : filename.substr(0, dot);
}

// Функция поиска треков по жанру
void findTracksByGenres(const std::string& dbPath, const std::vector<std::string>& genres) {
    if (genres.empty()) {
        std::cerr << "No genres provided.\n";
        return;
    }

    // Открытие базы данных
    sqlite3* db;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Cannot open DB\n";
        return;
    }

    // SQL-запрос: выбирает треки, у которых совпадают все указанные жанры
    std::string sql = R"(
        SELECT t.path
        FROM tracks t
        JOIN track_genres tg ON t.id = tg.track_id
        JOIN genres g ON tg.genre_id = g.id
        WHERE g.name IN ()GROUP_PLACEHOLDER()
        GROUP BY t.id
        HAVING COUNT(DISTINCT g.name) = ?
    )";

    // Построим плейсхолдеры (?, ?, ...) для IN
    std::string placeholders = "(" + std::string(genres.size() * 2 - 1, '?');
    for (size_t i = 1; i < genres.size(); ++i) {
        placeholders[i * 2 - 1] = ',';
        placeholders[i * 2] = '?';
    }
    placeholders += ")";

    // Вставим плейсхолдеры в SQL
    size_t pos = sql.find("()GROUP_PLACEHOLDER()");
    sql.replace(pos, std::strlen("()GROUP_PLACEHOLDER()"), placeholders);

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << "\n";
        sqlite3_close(db);
        return;
    }

    // Привяжем жанры
    int index = 1;
    for (const auto& genre : genres) {
        sqlite3_bind_text(stmt, index++, genre.c_str(), -1, SQLITE_STATIC);
    }

    // Привяжем количество жанров для HAVING
    sqlite3_bind_int(stmt, index, static_cast<int>(genres.size()));

    // Получим пути
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::cout << path << "\n";
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}


// Функция для инициализации БД
void createDatabase(const std::string& dbPath) {
    sqlite3* db;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Ошибка открытия базы данных: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    // SQL-запросы на создание таблиц
    const char* sqlTracks = R"(
        CREATE TABLE IF NOT EXISTS tracks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            path TEXT NOT NULL UNIQUE
        );
    )";

    const char* sqlGenres = R"(
        CREATE TABLE IF NOT EXISTS genres (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL COLLATE NOCASE UNIQUE
        );
    )";

    const char* sqlTrackGenres = R"(
        CREATE TABLE IF NOT EXISTS track_genres (
            track_id INTEGER,
            genre_id INTEGER,
            PRIMARY KEY (track_id, genre_id),
            FOREIGN KEY (track_id) REFERENCES tracks(id),
            FOREIGN KEY (genre_id) REFERENCES genres(id)
        );
    )";

    char* errMsg = nullptr;

    // Создание таблицы tracks
    if (sqlite3_exec(db, sqlTracks, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Ошибка создания таблицы tracks: " << errMsg << "\n";
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return;
    }

    // Создание таблицы genres
    if (sqlite3_exec(db, sqlGenres, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Ошибка создания таблицы genres: " << errMsg << "\n";
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return;
    }

    // Создание таблицы track_genres
    if (sqlite3_exec(db, sqlTrackGenres, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Ошибка создания таблицы track_genres: " << errMsg << "\n";
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return;
    }

    std::cout << "База данных успешно создана или уже существует.\n";
    sqlite3_close(db);
}

// Добавление трека и жанров
bool addTrackWithGenres(const std::string& dbPath, const std::string& filePath, const std::vector<std::string>& genres) {
    sqlite3* db;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Cannot open DB: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    // Проверка дубликата
    sqlite3_stmt* checkStmt;
    const char* sqlCheck = "SELECT id FROM tracks WHERE path = ?;";
    if (sqlite3_prepare_v2(db, sqlCheck, -1, &checkStmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare check statement: " << sqlite3_errmsg(db) << "\n";
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(checkStmt, 1, filePath.c_str(), -1, SQLITE_STATIC);
    bool exists = (sqlite3_step(checkStmt) == SQLITE_ROW);
    sqlite3_finalize(checkStmt);

    if (exists) {
        std::cout << "Track already exists in the database.\n";
        sqlite3_close(db);
        return false;
    }

    // Начинаем транзакцию
    if (sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to begin transaction: " << sqlite3_errmsg(db) << "\n";
        sqlite3_close(db);
        return false;
    }

    // Вставка трека
    sqlite3_stmt* stmtTrack = nullptr;
    const char* sqlInsertTrack = "INSERT INTO tracks (path) VALUES (?);";
    if (sqlite3_prepare_v2(db, sqlInsertTrack, -1, &stmtTrack, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare insert track failed: " << sqlite3_errmsg(db) << "\n";
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(stmtTrack, 1, filePath.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmtTrack) != SQLITE_DONE) {
        std::cerr << "Failed to insert track: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmtTrack);
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(db);
        return false;
    }
    sqlite3_finalize(stmtTrack);
    int trackId = static_cast<int>(sqlite3_last_insert_rowid(db));

    // Добавление жанров и связей
    for (const auto& genre : genres) {
        int genreId = -1;

        // Поиск жанра
        sqlite3_stmt* stmtFindGenre = nullptr;
        const char* sqlFindGenre = "SELECT id FROM genres WHERE name = ?;";
        if (sqlite3_prepare_v2(db, sqlFindGenre, -1, &stmtFindGenre, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmtFindGenre, 1, genre.c_str(), -1, SQLITE_STATIC);
            if (sqlite3_step(stmtFindGenre) == SQLITE_ROW) {
                genreId = sqlite3_column_int(stmtFindGenre, 0);
            }
        }
        sqlite3_finalize(stmtFindGenre);

        // Вставка жанра
        if (genreId == -1) {
            sqlite3_stmt* stmtInsertGenre = nullptr;
            const char* sqlInsertGenre = "INSERT OR IGNORE INTO genres (name) VALUES (?);";
            if (sqlite3_prepare_v2(db, sqlInsertGenre, -1, &stmtInsertGenre, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmtInsertGenre, 1, genre.c_str(), -1, SQLITE_STATIC);
                sqlite3_step(stmtInsertGenre);
                sqlite3_finalize(stmtInsertGenre);
            }

            if (sqlite3_prepare_v2(db, sqlFindGenre, -1, &stmtFindGenre, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmtFindGenre, 1, genre.c_str(), -1, SQLITE_STATIC);
                if (sqlite3_step(stmtFindGenre) == SQLITE_ROW) {
                    genreId = sqlite3_column_int(stmtFindGenre, 0);
                }
                sqlite3_finalize(stmtFindGenre);
            }
        }

        // Вставка в track_genres
        if (genreId != -1) {
            sqlite3_stmt* stmtLink;
            const char* sqlLink = "INSERT OR IGNORE INTO track_genres (track_id, genre_id) VALUES (?, ?);";
            if (sqlite3_prepare_v2(db, sqlLink, -1, &stmtLink, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(stmtLink, 1, trackId);
                sqlite3_bind_int(stmtLink, 2, genreId);
                sqlite3_step(stmtLink);
                sqlite3_finalize(stmtLink);
            }
        }
    }

    if (sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to commit transaction: " << sqlite3_errmsg(db) << "\n";
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(db);
        return false;
    }

    sqlite3_close(db);
    return true;
}

// Главная функция
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // ///////////////////////////////////////////////////////////////////////////////////////
    // -------------------- !!! УКАЗАТЬ ПУТЬ К ИКОНКЕ ПРИЛОЖЕНИЯ !!! ---------------------- //
    // ///////////////////////////////////////////////////////////////////////////////////////
    app.setWindowIcon(QIcon("/Users/ilya/Documents/PSTU/Cpp/QT/AppMusic/images/AppIcon.png"));

    // ///////////////////////////////////////////////////////////////
    // -------- !!! УКАЗАТЬ ПУТЬ К КАТАЛОГУ С МУЗЫКОЙ !!! --------- //
    // ///////////////////////////////////////////////////////////////
    std::string songsDir = "/Users/ilya/Documents/PSTU/Cpp/QT/Songs/";

    // /////////////////////////////////////////////////////////////////////////
    // --------------- !!! УКАЗАТЬ ПУТЬ К КАТАЛОГУ С БД !!! ----------------- //
    // /////////////////////////////////////////////////////////////////////////
    std::string dbPath = "/Users/ilya/Documents/PSTU/Cpp/QT/AppMusic/tracks.db";

    // Создание БД и предварительная проверка треков ДО запуска GUI
    createDatabase(dbPath);

    for (const auto& entry : fs::directory_iterator(songsDir)) {
        if (entry.path().extension() != ".mp3") continue;

        std::string mp3Filename = entry.path().string();

        // Проверка: если трек уже в БД и у него есть жанры — пропустить
        {
            sqlite3* db;
            if (sqlite3_open(dbPath.c_str(), &db) == SQLITE_OK) {
                sqlite3_stmt* stmt;
                const char* sql = R"(
                    SELECT tg.genre_id
                    FROM tracks t
                    LEFT JOIN track_genres tg ON t.id = tg.track_id
                    WHERE t.path = ?
                    LIMIT 1;
                )";

                if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                    sqlite3_bind_text(stmt, 1, mp3Filename.c_str(), -1, SQLITE_STATIC);
                    int rc = sqlite3_step(stmt);

                    if (rc == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
                        std::cout << "Track with genres already in database, skipping: " << mp3Filename << "\n";
                        sqlite3_finalize(stmt);
                        sqlite3_close(db);
                        continue;
                    }

                    sqlite3_finalize(stmt);
                }
                sqlite3_close(db);
            }
        }

        std::filesystem::path path = entry.path();
        path.replace_extension(".wav");
        std::string wavFilename = path.string();

        std::cout << "\nProcessing: " << mp3Filename << std::endl;

        // Конвертация в WAV (если не существует)
        if (!fs::exists(wavFilename)) {
            std::string ffmpegCmd = "/opt/homebrew/bin/ffmpeg -y -loglevel quiet -i \"" + mp3Filename + "\" \"" + wavFilename + "\"";
            int ret = std::system(ffmpegCmd.c_str());
            if (ret != 0) {
                std::cerr << "FFmpeg failed for " << mp3Filename << "\n";
                continue;
            }
        }

        // Запуск Python-скрипта
        std::string command = "cd /Users/ilya/Documents/PSTU/Cpp/QT/AppMusic/audioset_tagging_cnn && "
                              "/Library/Frameworks/Python.framework/Versions/3.13/bin/python3 pytorch/inference.py "
                              "audio_tagging --model_type=Cnn14 "
                              "--checkpoint_path=Cnn14_mAP=0.431.pth "
                              "--audio_path=\"" + wavFilename + "\"";

        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            std::cerr << "Failed to run Python script\n";
            continue;
        }

        std::vector<std::string> tagList;
        std::vector<std::string> excludedTags = {"Music", "Zing", "Speech", "embedding"};
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            std::string line(buffer);
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string tag = line.substr(0, colon);
                if (std::find(excludedTags.begin(), excludedTags.end(), tag) == excludedTags.end()) {
                    tagList.push_back(tag);
                }
            }
        }
        pclose(pipe);

        std::cout << "Detected tags for " << mp3Filename << ":\n";
        for (const auto& tag : tagList) {
            std::cout << "  " << tag << std::endl;
        }

        // Добавление в БД
        addTrackWithGenres(dbPath, mp3Filename, tagList);
    }

    // После всех проверок — запуск окна
    MainWindow window;
    window.setFixedSize(1200, 800);
    window.setStyleSheet("background-color: #2c2c2c;");
    window.show();

    return app.exec();
}

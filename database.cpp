#include "database.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QSqlError>

Database& Database::instance()
{
    static Database instance;
    return instance;
}

Database::Database() : currentUserId(-1)
{
}

Database::~Database()
{
    if (db.isOpen()) {
        db.close();
    }
}

bool Database::initialize()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    QString dbPath = dataPath + "/tasks.db";
    
    db.setDatabaseName(dbPath);
    
    if (!db.open()) {
        qDebug() << "Ошибка открытия базы данных:" << db.lastError().text();
        return false;
    }
    
    return createTables();
}

bool Database::createTables()
{
    QSqlQuery query;
    
    query.exec("CREATE TABLE IF NOT EXISTS users ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "username TEXT UNIQUE NOT NULL,"
               "password_hash TEXT NOT NULL,"
               "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
               ")");
    
    if (query.lastError().isValid()) {
        qDebug() << "Ошибка создания таблицы user:" << query.lastError().text();
        return false;
    }
    
    query.exec("CREATE TABLE IF NOT EXISTS tasks ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "user_id INTEGER NOT NULL,"
               "title TEXT NOT NULL,"
               "task_date DATE NOT NULL,"
               "task_time TIME,"
               "is_time_bound INTEGER NOT NULL DEFAULT 1,"
               "recurrence TEXT,"
               "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
               "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE"
               ")");
    
    if (query.lastError().isValid()) {
        qDebug() << "Ошибка создания таблицы tasks:" << query.lastError().text();
        return false;
    }
    
    query.exec("CREATE INDEX IF NOT EXISTS idx_tasks_user_date ON tasks(user_id, task_date)");
    
    query.exec("CREATE TABLE IF NOT EXISTS habits ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "user_id INTEGER NOT NULL,"
               "name TEXT NOT NULL,"
               "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
               "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE"
               ")");
    
    if (query.lastError().isValid()) {
        qDebug() << "Ошибка создания таблицы habits:" << query.lastError().text();
        return false;
    }
    
    query.exec("CREATE TABLE IF NOT EXISTS habit_completions ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "habit_id INTEGER NOT NULL,"
               "completion_date DATE NOT NULL,"
               "FOREIGN KEY(habit_id) REFERENCES habits(id) ON DELETE CASCADE,"
               "UNIQUE(habit_id, completion_date)"
               ")");
    
    if (query.lastError().isValid()) {
        qDebug() << "Ошибка создания таблицы habit_completions:" << query.lastError().text();
        return false;
    }
    
    query.exec("CREATE TABLE IF NOT EXISTS notes ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "user_id INTEGER NOT NULL,"
               "name TEXT NOT NULL,"
               "content TEXT,"
               "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
               "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
               "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE"
               ")");
    
    if (query.lastError().isValid()) {
        qDebug() << "Ошибка создания таблицы notes:" << query.lastError().text();
        return false;
    }
    
    return true;
}

QString Database::hashPassword(const QString& password)
{
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(password.toUtf8());
    return hash.result().toHex();
}

bool Database::createUser(const QString& username, const QString& password)
{

    if (username.isEmpty() || password.isEmpty()) {
        return false;
    }
    
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password_hash) VALUES (?, ?)");
    query.addBindValue(username);
    query.addBindValue(hashPassword(password));
    
    if (!query.exec()) {
        qDebug() << "Ошибка создания пользователя:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool Database::authenticateUser(const QString& username, const QString& password)
{

    QSqlQuery query;
    query.prepare("SELECT id, password_hash FROM users WHERE username = ?");
    query.addBindValue(username);
    
    if (!query.exec() || !query.next()) {
        return false;
    }
    
    int userId = query.value(0).toInt();
    QString storedHash = query.value(1).toString();
    
    QString inputHash = hashPassword(password);
    
    if (storedHash == inputHash) {
        currentUserId = userId;
        return true;
    }
    
    return false;
}

bool Database::createTask(int userId, const QString& title, const QDateTime& dateTime,
                          bool isTimeBound, const QString& recurrence)
{
    QSqlQuery query;
    query.prepare("INSERT INTO tasks (user_id, title, task_date, task_time, is_time_bound, recurrence) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(userId);
    query.addBindValue(title);
    query.addBindValue(dateTime.date());
    query.addBindValue(isTimeBound ? dateTime.time() : QVariant());
    query.addBindValue(isTimeBound ? 1 : 0);
    query.addBindValue(recurrence.isEmpty() ? QVariant() : recurrence);
    
    if (!query.exec()) {
        qDebug() << "Ошибка создания задачи:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool Database::updateTask(int taskId, const QString& title, const QDateTime& dateTime,
                          bool isTimeBound, const QString& recurrence)
{
    QSqlQuery query;
    query.prepare("UPDATE tasks SET title = ?, task_date = ?, task_time = ?, "
                  "is_time_bound = ?, recurrence = ? WHERE id = ?");
    query.addBindValue(title);
    query.addBindValue(dateTime.date());
    query.addBindValue(isTimeBound ? dateTime.time() : QVariant());
    query.addBindValue(isTimeBound ? 1 : 0);
    query.addBindValue(recurrence.isEmpty() ? QVariant() : recurrence);
    query.addBindValue(taskId);
    
    if (!query.exec()) {
        qDebug() << "Ошибка обновления задачи:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool Database::deleteTask(int taskId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM tasks WHERE id = ?");
    query.addBindValue(taskId);
    
    if (!query.exec()) {
        qDebug() << "Ошибка удаления задачи:" << query.lastError().text();
        return false;
    }
    
    return true;
}

QList<QHash<QString, QVariant>> Database::getTasksForDay(int userId, const QDate& date)
{
    QList<QHash<QString, QVariant>> tasks;
    
    QSqlQuery query;
    query.prepare("SELECT id, title, task_date, task_time, is_time_bound, recurrence "
                  "FROM tasks WHERE user_id = ? AND task_date = ? "
                  "ORDER BY is_time_bound DESC, task_time ASC");
    query.addBindValue(userId);
    query.addBindValue(date);
    
    if (!query.exec()) {
        qDebug() << "Ошибка получения задач:" << query.lastError().text();
        return tasks;
    }
    
    while (query.next()) {
        QHash<QString, QVariant> task;
        task["id"] = query.value(0);
        task["title"] = query.value(1);
        task["task_date"] = query.value(2);
        task["task_time"] = query.value(3);
        task["is_time_bound"] = query.value(4).toBool();
        task["recurrence"] = query.value(5);
        tasks.append(task);
    }
    
    return tasks;
}

QList<QHash<QString, QVariant>> Database::getTasksForWeek(int userId, const QDate& weekStart)
{
    QList<QHash<QString, QVariant>> tasks;
    QDate weekEnd = weekStart.addDays(6);
    
    QSqlQuery query;
    query.prepare("SELECT id, title, task_date, task_time, is_time_bound, recurrence "
                  "FROM tasks WHERE user_id = ? AND task_date >= ? AND task_date <= ? "
                  "ORDER BY task_date ASC, is_time_bound DESC, task_time ASC");
    query.addBindValue(userId);
    query.addBindValue(weekStart);
    query.addBindValue(weekEnd);
    
    if (!query.exec()) {
        qDebug() << "Ошибка выборки задач на неделю:" << query.lastError().text();
        return tasks;
    }
    
    while (query.next()) {
        QHash<QString, QVariant> task;
        task["id"] = query.value(0);
        task["title"] = query.value(1);
        task["task_date"] = query.value(2);
        task["task_time"] = query.value(3);
        task["is_time_bound"] = query.value(4).toBool();
        task["recurrence"] = query.value(5);
        tasks.append(task);
    }
    
    return tasks;
}

void Database::cleanupOldData(int userId, const QDate& currentWeekStart)
{

    QDate threeWeeksAgo = currentWeekStart.addDays(-21);
    
    QSqlQuery query;
    query.prepare("DELETE FROM tasks WHERE user_id = ? AND task_date < ?");
    query.addBindValue(userId);
    query.addBindValue(threeWeeksAgo);
    
    if (!query.exec()) {
        qDebug() << "Ошибка очистки старых данных:" << query.lastError().text();
    }
}

int Database::createNote(int userId, const QString& name, const QString& content)
{
    QSqlQuery query;
    query.prepare("INSERT INTO notes (user_id, name, content) VALUES (?, ?, ?)");
    query.addBindValue(userId);
    query.addBindValue(name);
    query.addBindValue(content);
    
    if (!query.exec()) {
        qDebug() << "Ошибка создания заметки:" << query.lastError().text();
        return -1;
    }
    
    return query.lastInsertId().toInt();
}

bool Database::updateNote(int noteId, const QString& name, const QString& content)
{
    QSqlQuery query;
    query.prepare("UPDATE notes SET name = ?, content = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(name);
    query.addBindValue(content);
    query.addBindValue(noteId);
    
    if (!query.exec()) {
        qDebug() << "Ошибка обновления заметки:" << query.lastError().text();
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

bool Database::deleteNote(int noteId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM notes WHERE id = ?");
    query.addBindValue(noteId);
    
    if (!query.exec()) {
        qDebug() << "Ошибка удаления заметки:" << query.lastError().text();
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

QList<QHash<QString, QVariant>> Database::getNotes(int userId)
{
    QList<QHash<QString, QVariant>> notes;
    
    QSqlQuery query;
    query.prepare("SELECT id, name, content, created_at, updated_at FROM notes WHERE user_id = ? ORDER BY updated_at DESC");
    query.addBindValue(userId);
    
    if (!query.exec()) {
        qDebug() << "Ошибка получения заметок:" << query.lastError().text();
        return notes;
    }
    
    while (query.next()) {
        QHash<QString, QVariant> note;
        note["id"] = query.value(0);
        note["name"] = query.value(1);
        note["content"] = query.value(2);
        note["created_at"] = query.value(3);
        note["updated_at"] = query.value(4);
        notes.append(note);
    }
    
    return notes;
}

QHash<QString, QVariant> Database::getNote(int noteId)
{
    QHash<QString, QVariant> note;
    
    QSqlQuery query;
    query.prepare("SELECT id, user_id, name, content, created_at, updated_at FROM notes WHERE id = ?");
    query.addBindValue(noteId);
    
    if (!query.exec()) {
        qDebug() << "Ошибка получения заметки:" << query.lastError().text();
        return note;
    }
    
    if (query.next()) {
        note["id"] = query.value(0);
        note["user_id"] = query.value(1);
        note["name"] = query.value(2);
        note["content"] = query.value(3);
        note["created_at"] = query.value(4);
        note["updated_at"] = query.value(5);
    }
    
    return note;
}

int Database::createHabit(int userId, const QString& name)
{
    QSqlQuery query;
    query.prepare("INSERT INTO habits (user_id, name) VALUES (?, ?)");
    query.addBindValue(userId);
    query.addBindValue(name);
    
    if (!query.exec()) {
        qDebug() << "Ошибка создания привычки:" << query.lastError().text();
        return -1;
    }
    
    return query.lastInsertId().toInt();
}

bool Database::deleteHabit(int habitId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM habits WHERE id = ?");
    query.addBindValue(habitId);
    
    if (!query.exec()) {
        qDebug() << "Ошибка удаления:" << query.lastError().text();
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

QList<QHash<QString, QVariant>> Database::getHabits(int userId)
{
    QList<QHash<QString, QVariant>> habits;
    
    QSqlQuery query;
    query.prepare("SELECT id, name, created_at FROM habits WHERE user_id = ? ORDER BY created_at DESC");
    query.addBindValue(userId);
    
    if (!query.exec()) {
        qDebug() << "Ошибка получения привычки:" << query.lastError().text();
        return habits;
    }
    
    while (query.next()) {
        QHash<QString, QVariant> habit;
        habit["id"] = query.value(0);
        habit["name"] = query.value(1);
        habit["created_at"] = query.value(2);
        habits.append(habit);
    }
    
    return habits;
}

bool Database::markHabitCompleted(int habitId, const QDate& date)
{
    QSqlQuery query;
    query.prepare("INSERT OR IGNORE INTO habit_completions (habit_id, completion_date) VALUES (?, ?)");
    query.addBindValue(habitId);
    query.addBindValue(date);
    
    if (!query.exec()) {
        qDebug() << "Ошибка при попытке пометить задачу как выполненную:" << query.lastError().text();
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

bool Database::unmarkHabitCompleted(int habitId, const QDate& date)
{
    QSqlQuery query;
    query.prepare("DELETE FROM habit_completions WHERE habit_id = ? AND completion_date = ?");
    query.addBindValue(habitId);
    query.addBindValue(date);
    
    if (!query.exec()) {
        qDebug() << "Стираем отметку о выполнении:" << query.lastError().text();
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

bool Database::isHabitCompleted(int habitId, const QDate& date)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM habit_completions WHERE habit_id = ? AND completion_date = ?");
    query.addBindValue(habitId);
    query.addBindValue(date);
    
    if (!query.exec() || !query.next()) {
        return false;
    }
    
    return query.value(0).toInt() > 0;
}

QList<QDate> Database::getHabitCompletionsForMonth(int habitId, int year, int month)
{
    QList<QDate> dates;
    
    QSqlQuery query;
    query.prepare("SELECT completion_date FROM habit_completions WHERE habit_id = ? AND strftime('%Y', completion_date) = ? AND strftime('%m', completion_date) = ?");
    query.addBindValue(habitId);
    query.addBindValue(QString::number(year));
    query.addBindValue(QString::number(month).rightJustified(2, '0'));
    
    if (!query.exec()) {
        qDebug() << "Ошибка подсчёта выполнений за месяц:" << query.lastError().text();
        return dates;
    }
    
    while (query.next()) {
        dates.append(query.value(0).toDate());
    }
    
    return dates;
}

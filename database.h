#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QDateTime>

class Database
{
public:

    static Database& instance();

    bool initialize();

    bool createTables();
    
    bool createUser(const QString& username, const QString& password);
    
    bool authenticateUser(const QString& username, const QString& password);
    
    int getCurrentUserId() const { return currentUserId; }
    
    void setCurrentUserId(int userId) { currentUserId = userId; }
    
    void logout() { currentUserId = -1; }
    
    bool isLoggedIn() const { return currentUserId > 0; }
    
    bool createTask(int userId, const QString& title, const QDateTime& dateTime, 
                    bool isTimeBound, const QString& recurrence = QString());
    
    bool updateTask(int taskId, const QString& title, const QDateTime& dateTime,
                    bool isTimeBound, const QString& recurrence = QString());
    
    bool deleteTask(int taskId);
    
    QList<QHash<QString, QVariant>> getTasksForDay(int userId, const QDate& date);
    
    QList<QHash<QString, QVariant>> getTasksForWeek(int userId, const QDate& weekStart);
    
    void cleanupOldData(int userId, const QDate& currentWeekStart);
    
    int createNote(int userId, const QString& name, const QString& content = QString());
    
    bool updateNote(int noteId, const QString& name, const QString& content);
    
    bool deleteNote(int noteId);
    
    QList<QHash<QString, QVariant>> getNotes(int userId);
    
    QHash<QString, QVariant> getNote(int noteId);
    
    int createHabit(int userId, const QString& name);
    
    bool deleteHabit(int habitId);
    
    QList<QHash<QString, QVariant>> getHabits(int userId);
    
    bool markHabitCompleted(int habitId, const QDate& date);
    
    bool unmarkHabitCompleted(int habitId, const QDate& date);
    
    bool isHabitCompleted(int habitId, const QDate& date);

    QList<QDate> getHabitCompletionsForMonth(int habitId, int year, int month);
    
private:
    Database();
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    QSqlDatabase db;
    int currentUserId;
    QString hashPassword(const QString& password);
};

#endif

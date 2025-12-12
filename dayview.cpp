#include "dayview.h"
#include "taskdialog.h"
#include "database.h"
#include <QMessageBox>
#include <QLocale>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QTime>

static QString getMonthNameNominative(int month)
{
    static const QStringList months = {
        "Январь", "Февраль", "Март", "Апрель", "Май", "Июнь",
        "Июль", "Август", "Сентябрь", "Октябрь", "Ноябрь", "Декабрь"
    };
    if (month >= 1 && month <= 12) {
        return months[month - 1];
    }
    return QString();
}

DayView::DayView(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    QHBoxLayout *headerLayout = new QHBoxLayout;
    backButton = new QPushButton("<- Назад");
    dateLabel = new QLabel;
    dateLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    headerLayout->addWidget(backButton);
    headerLayout->addWidget(dateLabel);
    headerLayout->addStretch();
    
    layout->addLayout(headerLayout);
    
    taskList = new QListWidget;
    layout->addWidget(taskList);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    addButton = new QPushButton("+ Добавить задачу");
    QPushButton *deleteButton = new QPushButton("Удалить задачу");
    QPushButton *moveButton = new QPushButton("-> Перенести задачу");
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(moveButton);
    buttonLayout->addStretch();
    
    layout->addLayout(buttonLayout);
    
    connect(backButton, &QPushButton::clicked, this, &DayView::backRequested);
    connect(addButton, &QPushButton::clicked, this, &DayView::onAddTaskClicked);
    connect(deleteButton, &QPushButton::clicked, this, &DayView::onDeleteTaskClicked);
    connect(moveButton, &QPushButton::clicked, this, &DayView::onMoveTaskClicked);

    connect(taskList, &QListWidget::itemDoubleClicked, this, &DayView::onTaskDoubleClicked);
}

void DayView::setDate(const QDate& date)
{
    currentDate = date;
    
    QLocale locale(QLocale::Russian);
    QString dayName = locale.dayName(date.dayOfWeek(), QLocale::LongFormat);

    QString monthName = getMonthNameNominative(date.month());
    QString dateStr = dayName + ", " + QString::number(date.day()) + " " + monthName;
    
    if (date.year() != QDate::currentDate().year()) {
        dateStr += " " + QString::number(date.year());
    }
    
    dateLabel->setText(dateStr);
    refreshTasks();
}

void DayView::refreshTasks()
{
    taskList->clear();
    
    Database& db = Database::instance();
    if (!db.isLoggedIn()) {
        return;
    }
    
    auto tasks = db.getTasksForDay(db.getCurrentUserId(), currentDate);
    
    for (const auto& task : tasks) {
        QString title = task["title"].toString();
        bool isTimeBound = task["is_time_bound"].toBool();
        QString timeStr;
        
        if (isTimeBound && !task["task_time"].isNull()) {
            QTime time = task["task_time"].toTime();
            timeStr = time.toString("HH:mm");
        }
        
        QString recurrence = task["recurrence"].toString();
        QString recurrenceStr = formatRecurrence(recurrence);
        
        QString displayText = title;
        
        if (!timeStr.isEmpty()) {
            displayText = timeStr + " - " + displayText;
        }
        
        if (!recurrenceStr.isEmpty()) {
            displayText += " (" + recurrenceStr + ")";
        }
        
        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, task["id"]);
        taskList->addItem(item);
    }
}

void DayView::onAddTaskClicked()
{
    if (currentDate < QDate::currentDate()) {
        QMessageBox::warning(this, "Ошибка", "Нельзя добавить задачу на прошедшую дату.");
        return;
    }
    
    TaskDialog dialog(currentDate, this);
    if (dialog.exec() == QDialog::Accepted) {
        Database& db = Database::instance();
        if (db.createTask(db.getCurrentUserId(), dialog.getTitle(), 
                         dialog.getDateTime(), dialog.isTimeBound(), 
                         dialog.getRecurrence())) {
            refreshTasks();
            emit taskAdded(currentDate);
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось создать задачу");
        }
    }
}

void DayView::onTaskDoubleClicked(QListWidgetItem* item)
{
    int taskId = item->data(Qt::UserRole).toInt();
    
    Database& db = Database::instance();
    auto tasks = db.getTasksForDay(db.getCurrentUserId(), currentDate);
    
    for (const auto& task : tasks) {
        if (task["id"].toInt() == taskId) {
            QDateTime dateTime;
            if (task["is_time_bound"].toBool()) {
                dateTime = QDateTime(task["task_date"].toDate(), 
                                    task["task_time"].toTime());
            } else {
                dateTime = QDateTime(task["task_date"].toDate(), QTime());
            }
            
            TaskDialog dialog(taskId, task["title"].toString(), dateTime,
                            task["is_time_bound"].toBool(), 
                            task["recurrence"].toString(), this);
            
            if (dialog.exec() == QDialog::Accepted) {
                if (db.updateTask(taskId, dialog.getTitle(), dialog.getDateTime(),
                                dialog.isTimeBound(), dialog.getRecurrence())) {
                    refreshTasks();
                    emit taskAdded(currentDate);
                } else {
                    QMessageBox::warning(this, "Ошибка", "Не удалось обновить задачу");
                }
            }
            break;
        }
    }
}

QString DayView::formatRecurrence(const QString& recurrence) const
{
    if (recurrence.isEmpty()) {
        return QString();
    }
    return recurrence;
}

void DayView::onDeleteTaskClicked()
{
    QListWidgetItem *selectedItem = taskList->currentItem();
    if (!selectedItem) {
        QMessageBox::information(this, "Информация", "Выберите задачу для удаления");
        return;
    }
    
    int ret = QMessageBox::question(this, "Подтверждение", 
                                     "Вы уверены, что хотите удалить эту задачу?",
                                     QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) {
        return;
    }
    
    int taskId = selectedItem->data(Qt::UserRole).toInt();
    
    Database& db = Database::instance();
    
    if (db.deleteTask(taskId)) {
        refreshTasks();
        emit taskAdded(currentDate);
        QMessageBox::information(this, "Успех", "Задача удалена");
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось удалить задачу");
    }
}

void DayView::onMoveTaskClicked()
{
    QListWidgetItem *selectedItem = taskList->currentItem();
    if (!selectedItem) {
        QMessageBox::information(this, "Информация", "Выберите задачу для переноса");
        return;
    }
    
    int taskId = selectedItem->data(Qt::UserRole).toInt();
    
    Database& db = Database::instance();
    auto tasks = db.getTasksForDay(db.getCurrentUserId(), currentDate);
    
    QHash<QString, QVariant> task;
    bool found = false;
    for (const auto& t : tasks) {
        if (t["id"].toInt() == taskId) {
            task = t;
            found = true;
            break;
        }
    }
    
    if (!found) {
        QMessageBox::warning(this, "Ошибка", "Задача не найдена");
        return;
    }
    
    QString title = task["title"].toString();
    bool isTimeBound = task["is_time_bound"].toBool();
    QString recurrence = task["recurrence"].toString();
    
    QDateTime dateTime;
    if (isTimeBound && !task["task_time"].isNull()) {
        dateTime = QDateTime(task["task_date"].toDate(), task["task_time"].toTime());
    } else {
        dateTime = QDateTime(task["task_date"].toDate(), QTime(9, 0));
    }
    
    TaskDialog dialog(taskId, title, dateTime, isTimeBound, recurrence, this);
    
    if (dialog.exec() == QDialog::Accepted) {

        QString newTitle = dialog.getTitle();
        QDateTime newDateTime = dialog.getDateTime();
        bool newIsTimeBound = dialog.isTimeBound();
        QString newRecurrence = dialog.getRecurrence();
        
        int userId = db.getCurrentUserId();
        if (db.createTask(userId, newTitle, newDateTime, newIsTimeBound, newRecurrence)) {

            db.deleteTask(taskId);
            
            refreshTasks();
            emit taskAdded(currentDate);
            QMessageBox::information(this, "Успех", "Задача перенесена");
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось перенести задачу");
        }
    }
}

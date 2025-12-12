#include "trackersview.h"
#include "database.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QListWidget>
#include <QFrame>
#include <QSettings>
#include <QSet>

TrackersView::TrackersView(QWidget *parent)
    : QWidget(parent)
{

    QVBoxLayout *layout = new QVBoxLayout(this);
    
    QHBoxLayout *headerLayout = new QHBoxLayout;
    backButton = new QPushButton("← Назад");
    QLabel *titleLabel = new QLabel("Трекеры привычек");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    headerLayout->addWidget(backButton);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    
    layout->addLayout(headerLayout);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    addButton = new QPushButton("+ Добавить привычку");
    deleteButton = new QPushButton("Удалить привычку");
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addStretch();
    
    layout->addLayout(buttonLayout);
    
    scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    contentWidget = new QWidget;
    contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setAlignment(Qt::AlignTop);
    contentLayout->setSpacing(5);
    contentLayout->setContentsMargins(10, 10, 10, 10);
    
    scrollArea->setWidget(contentWidget);
    layout->addWidget(scrollArea);
    
    connect(backButton, &QPushButton::clicked, this, &TrackersView::backRequested);
    connect(addButton, &QPushButton::clicked, this, &TrackersView::onAddHabitClicked);
    connect(deleteButton, &QPushButton::clicked, this, &TrackersView::onDeleteHabitClicked);
}

void TrackersView::refreshTrackers()
{

    QLayoutItem* item;
    while ((item = contentLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    habitRows.clear();
    
    Database& db = Database::instance();
    if (!db.isLoggedIn()) {
        QLabel *noAuthLabel = new QLabel("Необходимо войти в систему");
        noAuthLabel->setAlignment(Qt::AlignCenter);
        contentLayout->addWidget(noAuthLabel);
        return;
    }
    
    int userId = db.getCurrentUserId();
    auto habits = db.getHabits(userId);
    
    if (habits.isEmpty()) {
        QLabel *noHabitsLabel = new QLabel("Привычек пока нет");
        noHabitsLabel->setAlignment(Qt::AlignCenter);
        noHabitsLabel->setStyleSheet("font-size: 14px; color: #666;");
        contentLayout->addWidget(noHabitsLabel);
        return;
    }
    
    for (const auto& habit : habits) {
        int habitId = habit["id"].toInt();
        QString habitName = habit["name"].toString();
        createHabitRow(habitId, habitName);
    }
    
    contentLayout->addStretch();
}

void TrackersView::createHabitRow(int habitId, const QString& habitName)
{
    HabitRow hr;
    hr.habitId = habitId;
    
    QFrame *rowFrame = new QFrame;
    rowFrame->setFrameStyle(QFrame::Box);
    rowFrame->setStyleSheet("QFrame { background-color: #f9f9f9; border: 1px solid #ddd; border-radius: 3px; padding: 5px; }");
    hr.rowFrame = rowFrame;
    
    QHBoxLayout *rowLayout = new QHBoxLayout(rowFrame);
    rowLayout->setSpacing(5);
    rowLayout->setContentsMargins(5, 5, 5, 5);
    hr.rowLayout = rowLayout;
    
    QLabel *nameLabel = new QLabel(habitName);
    nameLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333; min-width: 150px;");
    nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    rowLayout->addWidget(nameLabel);
    hr.nameLabel = nameLabel;
    
    QDate today = QDate::currentDate();
    int year = today.year();
    int month = today.month();
    QDate firstDay = QDate(year, month, 1);
    int daysInMonth = firstDay.daysInMonth();
    
    QList<QDate> completedDates = Database::instance().getHabitCompletionsForMonth(habitId, year, month);
    QSet<QDate> completedSet(completedDates.begin(), completedDates.end());
    
    for (int day = 1; day <= daysInMonth; ++day) {
        QDate date(year, month, day);
        bool isCompleted = completedSet.contains(date);
        bool isFuture = date > today;
        
        QPushButton *dayButton = new QPushButton(QString::number(day));
        dayButton->setMinimumSize(25, 25);
        dayButton->setMaximumSize(25, 25);
        
        if (isFuture) {

            dayButton->setEnabled(false);
            dayButton->setStyleSheet("QPushButton { background-color: #f0f0f0; color: #999; font-size: 10px; border: 1px solid #ccc; }");
        } else if (isCompleted) {

            dayButton->setStyleSheet("QPushButton { background-color: #90EE90; color: #000; font-size: 10px; border: 1px solid #ccc; font-weight: bold; }");
        } else {

            dayButton->setStyleSheet("QPushButton { background-color: white; color: #000; font-size: 10px; border: 1px solid #ccc; }");
        }
        
        dayButton->setProperty("habitId", habitId);
        dayButton->setProperty("date", date);
        
        if (!isFuture) {
            connect(dayButton, &QPushButton::clicked, [this, habitId, date]() {
                onDayCellClicked(habitId, date);
            });
        }
        
        rowLayout->addWidget(dayButton);
        hr.dayButtons.append(dayButton);
    }
    
    int streak = calculateStreak(habitId);
    
    QLabel *streakLabel = new QLabel(QString::number(streak));
    streakLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333; min-width: 50px;");
    streakLabel->setAlignment(Qt::AlignCenter);
    rowLayout->addWidget(streakLabel);
    hr.streakLabel = streakLabel;
    
    contentLayout->addWidget(rowFrame);
    habitRows.append(hr);
}

int TrackersView::calculateStreak(int habitId) const
{
    Database& db = Database::instance();
    QDate today = QDate::currentDate();
    int streak = 0;
    
    for (int i = 0; i < 365; ++i) {
        QDate checkDate = today.addDays(-i);
        if (db.isHabitCompleted(habitId, checkDate)) {
            streak++;
        } else {
            break;
        }
    }
    
    return streak;
}

void TrackersView::onAddHabitClicked()
{
    Database& db = Database::instance();
    if (!db.isLoggedIn()) {
        QMessageBox::warning(this, "Ошибка", "Необходимо войти в систему");
        return;
    }
    
    bool ok;
    QString name = QInputDialog::getText(this, "Новая привычка", 
                                         "Введите имя привычки:",
                                         QLineEdit::Normal, "", &ok);
    
    if (!ok || name.isEmpty()) {
        return;
    }
    
    int userId = db.getCurrentUserId();
    int habitId = db.createHabit(userId, name);
    
    if (habitId > 0) {
        refreshTrackers();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось создать привычку");
    }
}

void TrackersView::onDeleteHabitClicked()
{

    Database& db = Database::instance();
    if (!db.isLoggedIn()) {
        return;
    }
    
    int userId = db.getCurrentUserId();
    auto habits = db.getHabits(userId);
    
    if (habits.isEmpty()) {
        QMessageBox::information(this, "Информация", "Нет привычек для удаления");
        return;
    }
    
    QStringList habitNames;
    for (const auto& habit : habits) {
        habitNames << habit["name"].toString();
    }
    
    bool ok;
    QString selectedName = QInputDialog::getItem(this, "Удалить привычку",
                                                "Выберите привычку для удаления:",
                                                habitNames, 0, false, &ok);
    
    if (!ok) {
        return;
    }
    
    int habitId = -1;
    for (const auto& habit : habits) {
        if (habit["name"].toString() == selectedName) {
            habitId = habit["id"].toInt();
            break;
        }
    }
    
    if (habitId < 0) {
        return;
    }
    
    int ret = QMessageBox::question(this, "Подтверждение", 
                                     "Вы уверены, что хотите удалить эту привычку?",
                                     QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) {
        return;
    }
    
    if (db.deleteHabit(habitId)) {
        refreshTrackers();
        QMessageBox::information(this, "Успех", "Привычка удалена");
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось удалить привычку");
    }
}

void TrackersView::onDayCellClicked(int habitId, const QDate& date)
{
    Database& db = Database::instance();
    
    bool isCompleted = db.isHabitCompleted(habitId, date);
    
    if (isCompleted) {

        if (db.unmarkHabitCompleted(habitId, date)) {
            refreshTrackers();
        }
    } else {

        if (db.markHabitCompleted(habitId, date)) {
            refreshTrackers();
        }
    }
}

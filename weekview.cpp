#include "weekview.h"
#include "database.h"
#include <QLocale>
#include <QScrollArea>
#include <QFrame>
#include <QMessageBox>
#include <QMouseEvent>
#include <QEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTime>
#include <QSettings>

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

WeekView::WeekView(QWidget *parent)
    : QWidget(parent)
{

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QHBoxLayout *labelLayout = new QHBoxLayout;
    weekLabel = new QLabel;
    weekLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    weekLabel->setAlignment(Qt::AlignCenter);
    labelLayout->addStretch();
    labelLayout->addWidget(weekLabel);
    labelLayout->addStretch();
    mainLayout->addLayout(labelLayout);
    
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    contentWidget = new QWidget;
    QHBoxLayout *gridContainerLayout = new QHBoxLayout(contentWidget);
    gridContainerLayout->setContentsMargins(0, 0, 0, 0);
    gridContainerLayout->setSpacing(0);
    
    prevButton = new QPushButton("←");
    prevButton->setMinimumWidth(50);
    prevButton->setMaximumWidth(50);
    gridContainerLayout->addWidget(prevButton);
    
    gridLayout = new QGridLayout;
    gridLayout->setSpacing(10);
    gridLayout->setContentsMargins(10, 10, 10, 10);
    
    QWidget *gridWidget = new QWidget;
    gridWidget->setLayout(gridLayout);
    gridContainerLayout->addWidget(gridWidget, 1);
    
    nextButton = new QPushButton("→");
    nextButton->setMinimumWidth(50);
    nextButton->setMaximumWidth(50);
    gridContainerLayout->addWidget(nextButton);
    
    dayWidgets.resize(7);
    
    QLocale locale(QLocale::Russian);
    QSettings settings;
    int firstDayOfWeek = settings.value("firstDayOfWeek", 1).toInt();
    
    QStringList dayNames;
    if (firstDayOfWeek == 7) {

        dayNames = {"Воскресенье", "Понедельник", "Вторник", "Среда",
                   "Четверг", "Пятница", "Суббота"};
    } else {

        dayNames = {"Понедельник", "Вторник", "Среда", "Четверг",
                   "Пятница", "Суббота", "Воскресенье"};
    }
    
    for (int i = 0; i < 7; ++i) {
        QLabel *dayNameLabel = new QLabel(dayNames[i]);
        dayNameLabel->setAlignment(Qt::AlignCenter);
        dayNameLabel->setStyleSheet("font-weight: bold; font-size: 10px; padding: 2px;");
        gridLayout->addWidget(dayNameLabel, 0, i);
    }
    
    for (int i = 0; i < 7; ++i) {
        DayWidget& dw = dayWidgets[i];
        dw.frame = new QFrame;
        dw.frame->setFrameStyle(QFrame::Box);
        dw.frame->setStyleSheet("QFrame { background-color: #f5f5f5; }");
        
        QVBoxLayout *dayLayout = new QVBoxLayout(dw.frame);
        
        QHBoxLayout *headerLayout = new QHBoxLayout;
        dw.header = new QLabel;
        dw.header->setStyleSheet("font-weight: bold; font-size: 14px;");
        dw.header->setCursor(Qt::PointingHandCursor);
        headerLayout->addWidget(dw.header);
        headerLayout->addStretch();
        
        dw.addButton = new QPushButton("+");
        dw.addButton->setMaximumSize(30, 30);
        dw.addButton->setStyleSheet("QPushButton { font-size: 18px; }");
        headerLayout->addWidget(dw.addButton);
        
        dayLayout->addLayout(headerLayout);
        
        dw.tasksLayout = new QVBoxLayout;
        dw.tasksLayout->setAlignment(Qt::AlignTop);
        dayLayout->addLayout(dw.tasksLayout);
        
        dw.moreLabel = new QLabel("<a href='#'>Подробнее</a>");
        dw.moreLabel->setAlignment(Qt::AlignCenter);
        dw.moreLabel->setOpenExternalLinks(false);
        dw.moreLabel->hide();
        dayLayout->addWidget(dw.moreLabel);
        
        connect(dw.moreLabel, &QLabel::linkActivated, this, [this, i]() {
            emit dayClicked(dayWidgets[i].date);
        });
        
        dayLayout->addStretch();
        
        gridLayout->addWidget(dw.frame, 1, i);
        
        dw.header->installEventFilter(this);
        dw.header->setProperty("dayIndex", i);
        connect(dw.addButton, &QPushButton::clicked, this, [this, i]() {
            emit addTaskRequested(dayWidgets[i].date);
        });
    }
    
    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);
    
    connect(prevButton, &QPushButton::clicked, this, &WeekView::onPrevWeekClicked);
    connect(nextButton, &QPushButton::clicked, this, &WeekView::onNextWeekClicked);
    
    setWeekStart(QDate::currentDate());
}

QDate WeekView::getMonday(const QDate& date) const
{
    QSettings settings;
    int firstDayOfWeek = settings.value("firstDayOfWeek", 1).toInt();
    
    int dayOfWeek = date.dayOfWeek();
    
    if (firstDayOfWeek == 7) {

        int daysToSunday = (dayOfWeek == 7) ? 0 : (7 - dayOfWeek);
        return date.addDays(daysToSunday);
    } else {

        int daysToMonday = (dayOfWeek == 1) ? 0 : (1 - dayOfWeek);
        return date.addDays(daysToMonday);
    }
}

void WeekView::setWeekStart(const QDate& date)
{
    weekStart = getMonday(date);
    refreshWeek();
}

bool WeekView::canGoBack() const
{
    QDate currentMonday = getMonday(QDate::currentDate());
    QDate threeWeeksAgo = currentMonday.addDays(-21);
    return weekStart >= threeWeeksAgo;
}

void WeekView::refreshWeek()
{
    QDate weekStartDate = weekStart;
    QDate currentWeekStart = getMonday(QDate::currentDate());
    
    QLocale locale(QLocale::Russian);

    QString monthName1 = getMonthNameNominative(weekStartDate.month());
    QString weekText = QString::number(weekStartDate.day()) + " " + monthName1;
    
    if (weekStartDate.year() != QDate::currentDate().year()) {
        weekText += " " + QString::number(weekStartDate.year());
    }
    
    weekText += " - ";
    QDate weekEnd = weekStartDate.addDays(6);
    QString monthName2 = getMonthNameNominative(weekEnd.month());
    weekText += QString::number(weekEnd.day()) + " " + monthName2;
    
    if (weekEnd.year() != QDate::currentDate().year()) {
        weekText += " " + QString::number(weekEnd.year());
    }
    
    weekLabel->setText(weekText);
    
    prevButton->setEnabled(canGoBack());
    nextButton->setEnabled(true);
    
    QSettings settings;
    int firstDayOfWeek = settings.value("firstDayOfWeek", 1).toInt();
    
    QVector<int> dayOrder(7);
    for (int i = 0; i < 7; ++i) {
        dayOrder[i] = i;
    }
    
    for (int i = 0; i < 7; ++i) {
        gridLayout->removeWidget(dayWidgets[i].frame);
    }
    
    QStringList dayNames;
    if (firstDayOfWeek == 7) {

        dayNames = {"Воскресенье", "Понедельник", "Вторник", "Среда",
                   "Четверг", "Пятница", "Суббота"};
    } else {

        dayNames = {"Понедельник", "Вторник", "Среда", "Четверг",
                   "Пятница", "Суббота", "Воскресенье"};
    }
    
    for (int i = 0; i < 7; ++i) {
        QLayoutItem* item = gridLayout->itemAtPosition(0, i);
        if (item && item->widget()) {
            QLabel* dayNameLabel = qobject_cast<QLabel*>(item->widget());
            if (dayNameLabel) {
                dayNameLabel->setText(dayNames[i]);
            }
        }
    }
    
    for (int col = 0; col < 7; ++col) {
        int dayIndex = dayOrder[col];
        QDate dayDate = weekStartDate.addDays(col);
        
        setupDayWidget(dayIndex, dayDate);
        
        refreshDayWidget(dayIndex);
        
        dayWidgets[dayIndex].header->setProperty("dayIndex", dayIndex);
        
        gridLayout->addWidget(dayWidgets[dayIndex].frame, 1, col);
    }
}

void WeekView::setupDayWidget(int index, const QDate& date)
{
    DayWidget& dw = dayWidgets[index];
    dw.date = date;
    dw.header->setText(formatDateHeader(date));
    dw.header->setTextInteractionFlags(Qt::TextBrowserInteraction);
}

QString WeekView::formatDateHeader(const QDate& date) const
{
    QLocale locale(QLocale::Russian);
    QString dayName = locale.dayName(date.dayOfWeek(), QLocale::LongFormat);

    QString monthName = getMonthNameNominative(date.month());
    QString dateStr = QString::number(date.day()) + " " + monthName;
    if (date.year() != QDate::currentDate().year()) {
        dateStr += " " + QString::number(date.year());
    }
    return dayName + "\n" + dateStr;
}

void WeekView::refreshDayWidget(int index)
{
    DayWidget& dw = dayWidgets[index];
    
    QLayoutItem* item;
    while ((item = dw.tasksLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    Database& db = Database::instance();
    if (!db.isLoggedIn()) {
        dw.moreLabel->hide();
        return;
    }
    
    auto tasks = db.getTasksForDay(db.getCurrentUserId(), dw.date);
    
    const int MAX_VISIBLE_TASKS = 8;
    bool showMore = tasks.size() > MAX_VISIBLE_TASKS;
    int tasksToShow = showMore ? MAX_VISIBLE_TASKS : tasks.size();
    
    for (int i = 0; i < tasksToShow; ++i) {
        const auto& task = tasks[i];
        QString title = task["title"].toString();
        bool isTimeBound = task["is_time_bound"].toBool();
        QString timeStr;
        
        if (isTimeBound && !task["task_time"].isNull()) {
            QTime time = task["task_time"].toTime();
            timeStr = time.toString("HH:mm") + " ";
        }
        
        QString displayText = timeStr + title;
        QLabel *taskLabel = new QLabel(displayText);
        taskLabel->setWordWrap(true);
        taskLabel->setStyleSheet("padding: 2px;");
        dw.tasksLayout->addWidget(taskLabel);
    }
    
    if (showMore) {
        dw.moreLabel->show();
    } else {
        dw.moreLabel->hide();
    }
}

void WeekView::onPrevWeekClicked()
{

    if (canGoBack()) {

        setWeekStart(weekStart.addDays(-7));
        
        Database& db = Database::instance();
        if (db.isLoggedIn()) {
            QDate currentMonday = getMonday(QDate::currentDate());
            db.cleanupOldData(db.getCurrentUserId(), currentMonday);
        }
    }
}

void WeekView::onNextWeekClicked()
{

    setWeekStart(weekStart.addDays(7));
    
    Database& db = Database::instance();
    if (db.isLoggedIn()) {
        QDate currentMonday = getMonday(QDate::currentDate());
        db.cleanupOldData(db.getCurrentUserId(), currentMonday);
    }
}

bool WeekView::eventFilter(QObject *obj, QEvent *event)
{

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        
        if (mouseEvent->button() == Qt::LeftButton) {

            for (int i = 0; i < dayWidgets.size(); ++i) {
                if (dayWidgets[i].header == obj) {

                    emit dayClicked(dayWidgets[i].date);
                    return true;
                }
            }
        }
    }
    
    return QWidget::eventFilter(obj, event);
}

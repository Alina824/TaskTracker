#include "monthview.h"
#include "database.h"
#include <QLocale>
#include <QFrame>
#include <QTime>
#include <QMouseEvent>
#include <QEvent>
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

MonthView::MonthView(QWidget *parent)
    : QWidget(parent)
{

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QHBoxLayout *labelLayout = new QHBoxLayout;
    monthLabel = new QLabel;
    monthLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    monthLabel->setAlignment(Qt::AlignCenter);
    labelLayout->addStretch();
    labelLayout->addWidget(monthLabel);
    labelLayout->addStretch();
    mainLayout->addLayout(labelLayout);
    
    QHBoxLayout *gridContainerLayout = new QHBoxLayout;
    gridContainerLayout->setContentsMargins(0, 0, 0, 0);
    gridContainerLayout->setSpacing(0);
    
    prevButton = new QPushButton("←");
    prevButton->setMinimumWidth(50);
    prevButton->setMaximumWidth(50);
    gridContainerLayout->addWidget(prevButton);
    
    QWidget *gridContainer = new QWidget;
    gridLayout = new QGridLayout(gridContainer);
    gridLayout->setSpacing(5);
    gridLayout->setContentsMargins(10, 10, 10, 10);
    
    QLocale locale(QLocale::Russian);
    QSettings settings;
    int firstDayOfWeek = settings.value("firstDayOfWeek", 1).toInt();
    
    QStringList dayNames;
    if (firstDayOfWeek == 7) {

        dayNames = {"Вс", "Пн", "Вт", "Ср", "Чт", "Пт", "Сб"};
    } else {

        dayNames = {"Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс"};
    }
    
    for (int i = 0; i < 7; ++i) {
        QLabel *dayNameLabel = new QLabel(dayNames[i]);
        dayNameLabel->setAlignment(Qt::AlignCenter);

        dayNameLabel->setStyleSheet("font-weight: bold; font-size: 10px; padding: 2px;");
        gridLayout->addWidget(dayNameLabel, 0, i);
    }
    
    dayWidgets.resize(42);
    
    gridContainerLayout->addWidget(gridContainer, 1);
    
    nextButton = new QPushButton("→");
    nextButton->setMinimumWidth(50);
    nextButton->setMaximumWidth(50);
    gridContainerLayout->addWidget(nextButton);
    
    mainLayout->addLayout(gridContainerLayout);
    
    connect(prevButton, &QPushButton::clicked, this, &MonthView::onPrevMonthClicked);
    connect(nextButton, &QPushButton::clicked, this, &MonthView::onNextMonthClicked);
    
    setMonth(QDate::currentDate());
}

QDate MonthView::getFirstDayOfMonth(const QDate& date) const
{
    return QDate(date.year(), date.month(), 1);
}

bool MonthView::canGoBack() const
{
    QDate currentMonth = QDate::currentDate();
    QDate threeMonthsAgo = currentMonth.addMonths(-3);
    return this->currentMonth >= threeMonthsAgo;
}

void MonthView::setMonth(const QDate& date)
{

    currentMonth = QDate(date.year(), date.month(), 1);
    refreshMonth();
}

void MonthView::refreshMonth()
{

    monthLabel->setText(formatMonthHeader(currentMonth));
    
    prevButton->setEnabled(canGoBack());
    nextButton->setEnabled(true);
    
    QDate firstDay = getFirstDayOfMonth(currentMonth);
    int firstWeekday = getFirstWeekday(firstDay);
    int daysInMonth = getDaysInMonth(currentMonth);
    
    QDate currentDate = firstDay.addDays(-firstWeekday);
    
    for (int i = 0; i < 42; ++i) {

        bool isCurrentMonth = (currentDate.month() == currentMonth.month() && 
                              currentDate.year() == currentMonth.year());
        
        setupDayWidget(i, currentDate, isCurrentMonth);
        
        refreshDayWidget(i);
        
        currentDate = currentDate.addDays(1);
    }
}

QString MonthView::formatMonthHeader(const QDate& date) const
{

    QString monthName = getMonthNameNominative(date.month());
    QString yearStr = QString::number(date.year());
    if (date.year() != QDate::currentDate().year()) {
        return monthName + " " + yearStr;
    }
    return monthName;
}

void MonthView::setupDayWidget(int index, const QDate& date, bool isCurrentMonth)
{
    if (index >= dayWidgets.size()) {
        return;
    }
    
    DayWidget& dw = dayWidgets[index];
    
    if (dw.frame) {
        gridLayout->removeWidget(dw.frame);
        delete dw.frame;
    }
    
    dw.date = date;
    dw.isCurrentMonth = isCurrentMonth;
    
    dw.frame = new QFrame;
    dw.frame->setFrameStyle(QFrame::Box);
    
    if (isCurrentMonth) {
        dw.frame->setStyleSheet("QFrame { background-color: #f5f5f5; border: 1px solid #ccc; }");
    } else {
        dw.frame->setStyleSheet("QFrame { background-color: #e8e8e8; border: 1px solid #ddd; }");
    }
    
    dw.frame->setMinimumSize(80, 60);
    dw.frame->setMaximumSize(120, 100);
    
    QVBoxLayout *dayLayout = new QVBoxLayout(dw.frame);
    dayLayout->setContentsMargins(3, 3, 3, 3);
    dayLayout->setSpacing(2);
    
    dw.dayNumber = new QLabel(QString::number(date.day()));
    dw.dayNumber->setStyleSheet("font-weight: bold; font-size: 11px;");
    if (!isCurrentMonth) {
        dw.dayNumber->setStyleSheet("font-weight: bold; font-size: 11px; color: #999;");
    }
    dayLayout->addWidget(dw.dayNumber);
    
    dw.tasksLayout = new QVBoxLayout;
    dw.tasksLayout->setAlignment(Qt::AlignTop);
    dw.tasksLayout->setSpacing(1);
    dayLayout->addLayout(dw.tasksLayout);
    
    dayLayout->addStretch();
    
    dw.frame->installEventFilter(this);
    dw.frame->setProperty("dayIndex", index);
    dw.frame->setCursor(Qt::PointingHandCursor);
    
    int row = (index / 7) + 1;
    int col = index % 7;
    gridLayout->addWidget(dw.frame, row, col);
}

void MonthView::refreshDayWidget(int index)
{

    if (index >= dayWidgets.size() || !dayWidgets[index].frame) {
        return;
    }
    
    DayWidget& dw = dayWidgets[index];
    
    QLayoutItem* item;
    while ((item = dw.tasksLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    if (!dw.isCurrentMonth) {
        return;
    }
    
    Database& db = Database::instance();
    if (!db.isLoggedIn()) {
        return;
    }
    
    auto tasks = db.getTasksForDay(db.getCurrentUserId(), dw.date);
    
    const int MAX_VISIBLE_TASKS = 2;
    
    for (int i = 0; i < qMin(tasks.size(), MAX_VISIBLE_TASKS); ++i) {
        const auto& task = tasks[i];
        QString title = task["title"].toString();
        bool isTimeBound = task["is_time_bound"].toBool();
        QString timeStr;
        
        if (isTimeBound && !task["task_time"].isNull()) {
            QTime time = task["task_time"].toTime();
            timeStr = time.toString("HH:mm") + " ";
        }
        
        QString displayText = timeStr + title;
        
        if (displayText.length() > 15) {
            displayText = displayText.left(12) + "...";
        }
        
        QLabel *taskLabel = new QLabel(displayText);
        taskLabel->setWordWrap(false);
        taskLabel->setStyleSheet("font-size: 8px; padding: 1px;");
        taskLabel->setToolTip(task["title"].toString());
        dw.tasksLayout->addWidget(taskLabel);
    }
    
    if (tasks.size() > MAX_VISIBLE_TASKS) {
        QLabel *moreLabel = new QLabel("...");
        moreLabel->setStyleSheet("font-size: 8px; color: #666;");
        moreLabel->setAlignment(Qt::AlignCenter);
        dw.tasksLayout->addWidget(moreLabel);
    }
}

int MonthView::getDaysInMonth(const QDate& date) const
{
    return date.daysInMonth();
}

int MonthView::getFirstWeekday(const QDate& date) const
{
    int weekday = date.dayOfWeek();
    return (weekday == 1) ? 0 : (weekday - 1);
}

void MonthView::onPrevMonthClicked()
{
    if (canGoBack()) {
        currentMonth = currentMonth.addMonths(-1);
        refreshMonth();
        
        Database& db = Database::instance();
        if (db.isLoggedIn()) {
            QDate currentDate = QDate::currentDate();
            db.cleanupOldData(db.getCurrentUserId(), currentDate);
        }
    }
}

void MonthView::onNextMonthClicked()
{
    currentMonth = currentMonth.addMonths(1);
    refreshMonth();
    
    Database& db = Database::instance();
    if (db.isLoggedIn()) {
        QDate currentDate = QDate::currentDate();
        db.cleanupOldData(db.getCurrentUserId(), currentDate);
    }
}

bool MonthView::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            for (int i = 0; i < dayWidgets.size(); ++i) {
                if (dayWidgets[i].frame == obj) {
                    emit dayClicked(dayWidgets[i].date);
                    return true;
                }
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

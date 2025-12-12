#ifndef MONTHVIEW_H
#define MONTHVIEW_H

#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QDate>
#include <QVBoxLayout>
#include <QHBoxLayout>

class MonthView : public QWidget
{
    Q_OBJECT

public:
    explicit MonthView(QWidget *parent = nullptr);
    void refreshMonth();
    
    void setMonth(const QDate& date);
    
    QDate getCurrentMonth() const { return currentMonth; }

signals:
    void dayClicked(const QDate& date);
    
    void addTaskRequested(const QDate& date);

private slots:
    void onPrevMonthClicked();
    
    void onNextMonthClicked();

private:
    QDate currentMonth;
    
    QDate getFirstDayOfMonth(const QDate& date) const;
    
    bool canGoBack() const;
    
    QPushButton *prevButton;
    QPushButton *nextButton;
    QLabel *monthLabel;
    QGridLayout *gridLayout;
    
    struct DayWidget {
        QFrame *frame;
        QLabel *dayNumber;
        QVBoxLayout *tasksLayout;
        QDate date;
        bool isCurrentMonth;
    };
    
    QVector<DayWidget> dayWidgets;
    
    void setupDayWidget(int index, const QDate& date, bool isCurrentMonth);
    
    QString formatMonthHeader(const QDate& date) const;
    
    void refreshDayWidget(int index);
    
    int getDaysInMonth(const QDate& date) const;
    
    int getFirstWeekday(const QDate& date) const;
    
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif

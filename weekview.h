#ifndef WEEKVIEW_H
#define WEEKVIEW_H

#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QDate>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QMouseEvent>

class WeekView : public QWidget
{
    Q_OBJECT

public:
    explicit WeekView(QWidget *parent = nullptr);
    void refreshWeek();
    void setWeekStart(const QDate& date);
    QDate getWeekStart() const { return weekStart; }

signals:
    void dayClicked(const QDate& date);
    void addTaskRequested(const QDate& date);

private slots:
    void onPrevWeekClicked();
    void onNextWeekClicked();

private:
    QDate weekStart;
    QDate getMonday(const QDate& date) const;
    bool canGoBack() const;
    
    QPushButton *prevButton;
    QPushButton *nextButton;
    QLabel *weekLabel;
    QGridLayout *gridLayout;
    QWidget *contentWidget;
    
    struct DayWidget {
        QFrame *frame;
        QLabel *header;
        QPushButton *addButton;
        QVBoxLayout *tasksLayout;
        QLabel *moreLabel;
        QDate date;
    };
    
    QVector<DayWidget> dayWidgets;
    
    void setupDayWidget(int index, const QDate& date);
    QString formatDateHeader(const QDate& date) const;
    void refreshDayWidget(int index);
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif

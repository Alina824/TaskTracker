#ifndef ALLNOTESVIEW_H
#define ALLNOTESVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>

class AllNotesView : public QWidget
{
    Q_OBJECT

public:
    
    explicit AllNotesView(QWidget *parent = nullptr);
    
    void refreshNotes();

signals:
    
    void backRequested();

private slots:
    
    void onBackClicked();

private:
    QScrollArea *scrollArea;
    QWidget *contentWidget;
    QVBoxLayout *contentLayout;
    QPushButton *backButton;
};

#endif

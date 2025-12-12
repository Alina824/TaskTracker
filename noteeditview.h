#ifndef NOTEEDITVIEW_H
#define NOTEEDITVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>

class NoteEditView : public QWidget
{
    Q_OBJECT

public:
    
    explicit NoteEditView(int noteId = 0, QWidget *parent = nullptr);
    
    void loadNote(int noteId);

signals:
    
    void backRequested();

private slots:
    
    void onBackClicked();
    
    void onNameChanged();
    
    void onContentChanged();

private:
    
    void saveNote();
    
    int currentNoteId;
    QLineEdit *nameEdit;
    QTextEdit *contentEdit;
    QPushButton *backButton;
    bool isSaving;
};

#endif

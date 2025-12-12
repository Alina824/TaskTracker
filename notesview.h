#ifndef NOTESVIEW_H
#define NOTESVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>

class NotesView : public QWidget
{
    Q_OBJECT

public:
    
    explicit NotesView(QWidget *parent = nullptr);
    
    void refreshNotes();

signals:
    
    void noteClicked(int noteId);
    
    void backRequested();

public slots:
    
    void onAddNoteClicked();
    
    void onDeleteNoteClicked();

private slots:
    
    void onNoteDoubleClicked(QListWidgetItem* item);

private:
    QListWidget *notesList;
    QPushButton *addButton;
    QPushButton *deleteButton;
    QPushButton *backButton;
};

#endif

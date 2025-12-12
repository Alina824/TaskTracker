#ifndef AUTHDIALOG_H
#define AUTHDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>

class AuthDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AuthDialog(QWidget *parent = nullptr);

    QString getUsername() const { return usernameEdit->text(); }
    
    QString getPassword() const { return passwordEdit->text(); }
    
    bool isLoginMode() const { return tabWidget->currentIndex() == 0; }

signals:
    void loginRequested(const QString& username, const QString& password);
    void registerRequested(const QString& username, const QString& password);

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    QTabWidget *tabWidget;
    
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QLabel *loginErrorLabel;
    
    QLineEdit *regUsernameEdit;
    QLineEdit *regPasswordEdit;
    QLineEdit *regConfirmPasswordEdit;
    QPushButton *registerButton;
    QLabel *registerErrorLabel;
};

#endif

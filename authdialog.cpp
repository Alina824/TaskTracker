#include "authdialog.h"
#include <QMessageBox>

AuthDialog::AuthDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Авторизация");
    setModal(true);
    resize(350, 250);
    
    tabWidget = new QTabWidget(this);
    
    QWidget *loginTab = new QWidget;
    QVBoxLayout *loginLayout = new QVBoxLayout(loginTab);
    
    usernameEdit = new QLineEdit;
    usernameEdit->setPlaceholderText("Имя пользователя");
    passwordEdit = new QLineEdit;
    passwordEdit->setPlaceholderText("Пароль");
    passwordEdit->setEchoMode(QLineEdit::Password);
    
    loginButton = new QPushButton("Войти");
    loginErrorLabel = new QLabel;
    loginErrorLabel->setStyleSheet("color: red;");
    loginErrorLabel->setWordWrap(true);
    loginErrorLabel->hide();
    
    loginLayout->addWidget(new QLabel("Имя пользователя:"));
    loginLayout->addWidget(usernameEdit);
    loginLayout->addWidget(new QLabel("Пароль:"));
    loginLayout->addWidget(passwordEdit);
    loginLayout->addWidget(loginErrorLabel);
    loginLayout->addWidget(loginButton);
    loginLayout->addStretch();
    
    QWidget *registerTab = new QWidget;
    QVBoxLayout *registerLayout = new QVBoxLayout(registerTab);
    
    regUsernameEdit = new QLineEdit;
    regUsernameEdit->setPlaceholderText("Имя пользователя");
    regPasswordEdit = new QLineEdit;
    regPasswordEdit->setPlaceholderText("Пароль");
    regPasswordEdit->setEchoMode(QLineEdit::Password);
    regConfirmPasswordEdit = new QLineEdit;
    regConfirmPasswordEdit->setPlaceholderText("Подтвердите пароль");
    regConfirmPasswordEdit->setEchoMode(QLineEdit::Password);
    
    registerButton = new QPushButton("Зарегистрироваться");
    registerErrorLabel = new QLabel;
    registerErrorLabel->setStyleSheet("color: red;");
    registerErrorLabel->setWordWrap(true);
    registerErrorLabel->hide();
    
    registerLayout->addWidget(new QLabel("Имя пользователя:"));
    registerLayout->addWidget(regUsernameEdit);
    registerLayout->addWidget(new QLabel("Пароль:"));
    registerLayout->addWidget(regPasswordEdit);
    registerLayout->addWidget(new QLabel("Подтвердите пароль:"));
    registerLayout->addWidget(regConfirmPasswordEdit);
    registerLayout->addWidget(registerErrorLabel);
    registerLayout->addWidget(registerButton);
    registerLayout->addStretch();
    
    tabWidget->addTab(loginTab, "Вход");
    tabWidget->addTab(registerTab, "Регистрация");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tabWidget);
    
    connect(loginButton, &QPushButton::clicked, this, &AuthDialog::onLoginClicked);
    connect(registerButton, &QPushButton::clicked, this, &AuthDialog::onRegisterClicked);
    connect(usernameEdit, &QLineEdit::returnPressed, passwordEdit, [this]() { passwordEdit->setFocus(); });
    connect(passwordEdit, &QLineEdit::returnPressed, this, &AuthDialog::onLoginClicked);
    connect(regConfirmPasswordEdit, &QLineEdit::returnPressed, this, &AuthDialog::onRegisterClicked);
}

void AuthDialog::onLoginClicked()
{
    loginErrorLabel->hide();
    
    if (usernameEdit->text().isEmpty() || passwordEdit->text().isEmpty()) {
        loginErrorLabel->setText("Заполните все поля");
        loginErrorLabel->show();
        return;
    }
    
    emit loginRequested(usernameEdit->text(), passwordEdit->text());
}

void AuthDialog::onRegisterClicked()
{
    registerErrorLabel->hide();
    
    if (regUsernameEdit->text().isEmpty() || regPasswordEdit->text().isEmpty() || 
        regConfirmPasswordEdit->text().isEmpty()) {
        registerErrorLabel->setText("Заполните все поля");
        registerErrorLabel->show();
        return;
    }
    
    if (regPasswordEdit->text() != regConfirmPasswordEdit->text()) {
        registerErrorLabel->setText("Пароли не совпадают");
        registerErrorLabel->show();
        return;
    }
    
    emit registerRequested(regUsernameEdit->text(), regPasswordEdit->text());
}

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "edgeproxysetting.h"
#include <QMessageBox>

#ifdef Q_OS_WIN
#include <windows.h>
#include <wininet.h>
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("InternetSettingTool V1.0");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_read_clicked()
{
    bool proxyEnabled;
    QString proxyServer;
    QString proxyOverride;
    bool autoDetect;

    if (EdgeProxySetting::readLANSettings(proxyEnabled, proxyServer, proxyOverride, autoDetect)) {
        // 更新界面控件
        ui->checkBox_proxyEnable->setChecked(proxyEnabled);
        ui->lineEdit_proxyServer->setText(proxyServer);
        ui->lineEdit_exceptTable->setText(proxyOverride);
        ui->checkBox_autoDetect->setChecked(autoDetect);
        // 状态栏提示
        statusBar()->showMessage("设置加载成功", 3000);
    } else {
        QMessageBox::warning(this, "错误", "读取当前设置失败！\n请检查应用程序权限。");
        statusBar()->showMessage("读取设置失败", 3000);
    }
}

void MainWindow::on_pushButton_apply_clicked()
{
    bool proxyEnabled = ui->checkBox_proxyEnable->isChecked();
    QString proxyServer = ui->lineEdit_proxyServer->text().trimmed();
    QString proxyOverride = ui->lineEdit_exceptTable->text().trimmed();
    bool autoDetect = ui->checkBox_autoDetect->isChecked();

    // 验证代理服务器格式（如果启用代理）
    if (proxyEnabled && proxyServer.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "启用代理时需要填写代理服务器地址！");
        ui->lineEdit_proxyServer->setFocus();
        return;
    }

    if (EdgeProxySetting::setProxySettings(proxyEnabled, proxyServer, proxyOverride, autoDetect)) {
        QMessageBox::information(this, "成功",
            "代理设置已更新！\n"
            "• 可能需要重启浏览器使设置生效\n"
            "• 某些应用程序可能需要重新启动");
        statusBar()->showMessage("设置应用成功", 3000);
    } else {
        QMessageBox::warning(this, "错误",
            "更新代理设置失败！\n"
            "可能的原因：\n"
            "• 请尝试以管理员权限运行此程序\n"
            "• 安全软件阻止了设置修改");
        statusBar()->showMessage("设置应用失败", 3000);
    }
}

void MainWindow::on_pushButton_default_clicked()
{
   EdgeProxySetting::setProxySettings(false, "", "<local>", true);
   ui->checkBox_proxyEnable->setChecked(false);
   ui->lineEdit_proxyServer->setText("");
   ui->lineEdit_exceptTable->setText("<local>");
   ui->checkBox_autoDetect->setChecked(true);
}

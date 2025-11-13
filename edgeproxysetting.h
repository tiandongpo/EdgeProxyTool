#ifndef EDGEPROXYSETTING_H
#define EDGEPROXYSETTING_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#include <wininet.h>
#endif

class EdgeProxySetting : public QObject
{
    Q_OBJECT

public:
    explicit EdgeProxySetting(QObject *parent = nullptr) : QObject(parent) {}

    // 读取局域网设置
    static bool readLANSettings(bool &proxyEnabled,
                               QString &proxyServer,
                               QString &proxyOverride,
                               bool &autoDetect) {
        try {
            QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
                              QSettings::NativeFormat);

            // 读取代理启用状态
            proxyEnabled = settings.value("ProxyEnable", 0).toInt() == 1;

            // 读取代理服务器地址
            proxyServer = settings.value("ProxyServer", "").toString();

            // 读取例外列表
            proxyOverride = settings.value("ProxyOverride", "").toString();

            // 读取自动检测设置
            QString autoConfigURL = settings.value("AutoConfigURL", "").toString();
            autoDetect = settings.value("AutoDetect", 0).toInt() == 1 || !autoConfigURL.isEmpty();

            return true;
        } catch (...) {
            return false;
        }
    }

    // 修改代理设置
    static bool setProxySettings(bool proxyEnabled,
                                const QString &proxyServer = "",
                                const QString &proxyOverride = "",
                                bool autoDetect = false) {
        try {
            QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
                              QSettings::NativeFormat);

            // 设置代理启用状态
            settings.setValue("ProxyEnable", proxyEnabled ? 1 : 0);

            // 设置代理服务器
            if (!proxyServer.isEmpty()) {
                settings.setValue("ProxyServer", proxyServer);
            } else if (!proxyEnabled) {
                settings.remove("ProxyServer");
            }

            // 设置例外列表
            if (!proxyOverride.isEmpty()) {
                settings.setValue("ProxyOverride", proxyOverride);
            } else {
                // 默认例外本地地址
                settings.setValue("ProxyOverride", "<local>");
            }

            // 设置自动检测
            settings.setValue("AutoDetect", autoDetect ? 1 : 0);

            // 通知系统设置已更改
            notifySystemProxyChange();

            return true;
        } catch (const std::exception& e) {
            qDebug() << "设置代理失败:" << e.what();
            return false;
        } catch (...) {
            qDebug() << "设置代理失败: 未知错误";
            return false;
        }
    }

    // 启用自动检测设置
    static bool enableAutoDetect(bool enabled) {
        QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
                          QSettings::NativeFormat);

        settings.setValue("AutoDetect", enabled ? 1 : 0);

        notifySystemProxyChange();
        return true;
    }

    // 获取当前设置的详细信息
    static QString getCurrentSettingsInfo() {
        bool proxyEnabled;
        QString proxyServer;
        QString proxyOverride;
        bool autoDetect;

        if (!readLANSettings(proxyEnabled, proxyServer, proxyOverride, autoDetect)) {
            return "读取设置失败";
        }

        QString info = QString("当前代理设置:\n"
                             "────────────────\n"
                             "代理启用: %1\n"
                             "代理服务器: %2\n"
                             "例外列表: %3\n"
                             "自动检测: %4\n"
                             "────────────────")
                     .arg(proxyEnabled ? "是" : "否")
                     .arg(proxyServer.isEmpty() ? "未设置" : proxyServer)
                     .arg(proxyOverride.isEmpty() ? "未设置" : proxyOverride)
                     .arg(autoDetect ? "是" : "否");

        return info;
    }

    // 恢复默认设置（禁用代理）
    static bool restoreDefaultSettings() {
        return setProxySettings(false, "", "<local>", true);
    }

    // 设置特定端口的HTTP代理
    static bool setHttpProxy(const QString &host, quint16 port) {
        QString proxyServer = QString("%1:%2").arg(host).arg(port);
        return setProxySettings(true, proxyServer, "<local>", false);
    }

    // 设置SOCKS代理
    static bool setSocksProxy(const QString &host, quint16 port) {
        QString proxyServer = QString("socks=%1:%2").arg(host).arg(port);
        return setProxySettings(true, proxyServer, "<local>", false);
    }

private:
    // 通知系统代理设置已更改
    static void notifySystemProxyChange() {
        QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
                          QSettings::NativeFormat);
        settings.sync();

        // 使用Windows API通知系统设置已更改
        #ifdef Q_OS_WIN
        InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
        InternetSetOption(NULL, INTERNET_OPTION_REFRESH, NULL, 0);

        // 额外的通知方式，确保所有应用程序都能收到更改通知
        SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
                          (LPARAM)L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
                          SMTO_ABORTIFHUNG, 1000, NULL);
        #endif
    }
};
#endif // EDGEPROXYSETTING_H

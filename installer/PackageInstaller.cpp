/*
* Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
*
* Author:     cuizhen <cuizhen@uniontech.com>
* Maintainer: cuizhen <cuizhen@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "PackageInstaller.h"
#include "manager/PackagesManager.h"
#include "package/Package.h"

#include <QTimer>

#include <QApt/Transaction>
#include <QApt/DebFile>
PackageInstaller::PackageInstaller(QApt::Backend *b)
{
    qInfo() << "Package Installer";
    m_backend = b;
}

void PackageInstaller::appendPackage(Package *packages)
{
    m_packages = packages;
}

bool PackageInstaller::isDpkgRunning()
{
    QProcess proc;

    // 获取当前的进程信息
    proc.start("ps", QStringList() << "-e"
               << "-o"
               << "comm");
    proc.waitForFinished();

    // 获取进程信息的数据
    const QString processOutput = proc.readAllStandardOutput();

    // 查看进程信息中是否存在dpkg 存在说明已经正在安装其他包
    if (processOutput.contains("dpkg")) return true;   //更换判断的方式

    return false;
}

void PackageInstaller::uninstallPackage()
{
    emit signal_startInstall();

    if (isDpkgRunning()) {
        qInfo() << "PackageInstaller" << "dpkg running, waitting...";
        // 缩短检查的时间，每隔1S检查当前dpkg是否正在运行。
        QTimer::singleShot(1000 * 1, this, &PackageInstaller::uninstallPackage);
        return;
    }
    const QStringList rdepends = m_packages->getPackageReverseDependList();     //检查是否有应用依赖到该包

    for (const auto &r : rdepends) {                                          // 卸载所有依赖该包的应用（二者的依赖关系为depends）
        if (m_backend->package(r))
            m_backend->markPackageForRemoval(r);
        else
            qInfo() << "PackageInstaller" << "reverse depend" << r << "error ,please check it!";
    }
    m_backend->markPackageForRemoval(m_packages->getName() + ':' + m_packages->getArchitecture());       //卸载当前包

    m_pTrans = m_backend->commitChanges();

    connect(m_pTrans, &QApt::Transaction::progressChanged, this, &PackageInstaller::signal_installProgress);

    //详细状态信息（安装情况）展示链接
    connect(m_pTrans, &QApt::Transaction::statusDetailsChanged, this, &PackageInstaller::signal_installDetailStatus);

    // trans运行中出现错误
    connect(m_pTrans, &QApt::Transaction::errorOccurred, this, [ = ](QApt::ErrorCode error) {
        emit signal_installError(error, m_pTrans->errorDetails());
    });

    // 卸载结束，处理卸载成功与失败的情况并发送结束信号
    connect(m_pTrans, &QApt::Transaction::finished, this, &PackageInstaller::signal_uninstallFinished);

    // 卸载结束之后 删除指针
    connect(m_pTrans, &QApt::Transaction::finished, m_pTrans, &QApt::Transaction::deleteLater);

    m_pTrans->run();

}

void PackageInstaller::installPackage()
{
    emit signal_startInstall();
    qInfo() << "[PackageInstaller]" << "installPackage";
    if (isDpkgRunning()) {
        qInfo() << "[PackageInstaller]" << "dpkg running, waitting...";
        // 缩短检查的时间，每隔1S检查当前dpkg是否正在运行。
        QTimer::singleShot(1000 * 1, this, &PackageInstaller::installPackage);
        return;
    }

    DependsStatus packageDependsStatus = m_packages->getDependStatus();
    switch (packageDependsStatus) {
    case DependsUnknown:
    case DependsBreak:
    case DependsAuthCancel:
    case ArchBreak:
        qInfo() << "[PackageInstaller]" << "installPackage" << "deal break package";
        dealBreakPackage();
        break;
    case DependsAvailable:
        qInfo() << "[PackageInstaller]" << "installPackage" << "deal available package";
        dealAvailablePackage();
        break;
    case DependsOk:
        qInfo() << "[PackageInstaller]" << "installPackage" << "deal installable package";
        dealInstallablePackage();
        break;
    }
    connect(m_pTrans, &QApt::Transaction::progressChanged, this, &PackageInstaller::signal_installProgress);

    //详细状态信息（安装情况）展示链接
    connect(m_pTrans, &QApt::Transaction::statusDetailsChanged, this, &PackageInstaller::signal_installDetailStatus);

    // trans运行中出现错误
    connect(m_pTrans, &QApt::Transaction::errorOccurred, this, [ = ](QApt::ErrorCode error) {
        emit signal_installError(error, m_pTrans->errorDetails());
    });
    // 卸载结束之后 删除指针
    connect(m_pTrans, &QApt::Transaction::finished, m_pTrans, &QApt::Transaction::deleteLater);

    m_pTrans->run();
}

void PackageInstaller::dealBreakPackage()
{
    DependsStatus packageDependsStatus = m_packages->getDependStatus();
    switch (packageDependsStatus) {
    case DependsBreak:
    case DependsAuthCancel:
        qInfo() << "[PackageInstaller]" << "dealBreakPackage" << "Broken dependencies";
        emit signal_installError(packageDependsStatus, "Broken dependencies");
        return;
    case ArchBreak:
        qInfo() << "[PackageInstaller]" << "dealBreakPackage" << "Unmatched package architecture";
        emit signal_installError(packageDependsStatus, "Unmatched package architecture");
        return;
    default:
        qInfo() << "[PackageInstaller]" << "dealBreakPackage" << "unknown error";
        emit signal_installError(packageDependsStatus, "unknown error");
        return;
    }
}

void PackageInstaller::dealAvailablePackage()
{
    const QStringList availableDepends = m_packages->getPackageAvailableDepends();
    qInfo() << "[PackageInstaller]" << "dealAvailablePackage" << "get available Depends" << availableDepends;
    //获取到可用的依赖包并根据后端返回的结果判断依赖包的安装结果
    for (auto const &p : availableDepends) {
        if (p.contains(" not found")) {                             //依赖安装失败
            emit signal_installError(DependsAvailable, p);
            qInfo() << "[PackageInstaller]" << "dealAvailablePackage" << p;

            return;
        }
        m_backend->markPackageForInstall(p);
    }
    qInfo() << "[PackageInstaller]" << "dealAvailablePackage" << "commit changes";
    m_pTrans = m_backend->commitChanges();
    connect(m_pTrans, &QApt::Transaction::finished, this, &PackageInstaller::installAvailableDepends);
}

void PackageInstaller::installAvailableDepends()
{
    const auto ret = m_pTrans->exitStatus();
    if (ret) {
        qWarning() << m_pTrans->error() << m_pTrans->errorDetails() << m_pTrans->errorString();     //transaction发生错误

        emit signal_installError(m_pTrans->error(), m_pTrans->errorDetails());
    }

    m_packages->setPackageDependStatus(DependsOk);
    installPackage();
}

void PackageInstaller::dealInstallablePackage()
{
    QApt::DebFile deb(m_packages->getPath());

    qInfo() << "[PackageInstaller]" << "dealInstallablePackage" << "install file" << m_packages->getPath();

    m_pTrans = m_backend->installFile(deb);//触发Qapt授权框和安装线程

    connect(m_pTrans, &QApt::Transaction::finished, this, &PackageInstaller::signal_installFinished);
}

PackageInstaller::~PackageInstaller()
{

}

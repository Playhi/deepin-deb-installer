/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PACKAGE_H
#define PACKAGE_H

#include "status/PackageStatus.h"
#include "status/PackageSigntureStatus.h"

#include <QObject>


class Package
{
public:
    Package(QString packagePath);
    Package(int index, QString packagePath);
    Package();

    ~Package();

    /**
     * @brief setPackageIndex 设置包的下标
     * @param index 包的下标
     */
    void setPackageIndex(int index);

    /**
     * @brief setPackagePath 设置包的路径
     * @param packagePath   包的路径
     */
    void setPackagePath(QString packagePath);

    /**
     * @brief setPackageDependStatus 设置包的依赖状态
     * @param packageDependStatus   包的依赖状态
     */
    void setPackageDependStatus(DependsStatus packageDependStatus);

    /**
     * @brief setPackageAvailableDepends 设置包的可用依赖列表
     * @param depends 依赖列表
     */
    void setPackageAvailableDepends(QStringList depends);

    /**
     * @brief setPackageInstallStatus 设置包的安装状态
     * @param packageInstallStatus 包的安装状态
     */
    void setPackageInstallStatus(InstallStatus packageInstallStatus);

    /**
     * @brief setPackageReverseDependsList 设置依赖于这个包的应用列表
     * @param reverseDepends    应用列表
     */
    void setPackageReverseDependsList(QStringList reverseDepends);

    /**
     * @brief getIndex 获取包的下标
     * @return 包的下标
     */
    int             getIndex();

    /**
     * @brief getValid 获取包的有效性
     * @return 包的有效性
     */
    bool            getValid();

    /**
     * @brief getName 获取包的名称
     * @return 包的名字
     */
    QString         getName();

    /**
     * @brief getPath 获取包的路径
     * @return 包的路径
     */
    QString         getPath();

    /**
     * @brief getVersion 获取包的版本
     * @return 包的版本
     */
    QString         getVersion();

    /**
     * @brief getArchitecture 获取包的架构
     * @return 包的架构
     */
    QString         getArchitecture();

    /**
     * @brief getMd5 获取包的MD5值
     * @return 包的md5值
     */
    QByteArray      getMd5();

    /**
     * @brief getDependStatus 获取包的依赖状态
     * @return  包的依赖状态
     */
    DependsStatus   getDependStatus();

    /**
     * @brief getSigntureStatus 获取包的签名状态
     * @return 包的签名状态
     */
    SigntureStatus  getSigntureStatus();

    /**
     * @brief getInstallStatus 获取包的安装状态
     * @return 包的安装状态
     */
    InstallStatus   getInstallStatus();

    /**
     * @brief getPackageAvailableDepends 获取包的可用依赖列表
     * @return 包的可用依赖列表
     */
    QStringList     getPackageAvailableDepends();

    /**
     * @brief getPackageReverseDependList 获取依赖于此应用的 应用列表
     * @return 应用列表
     */
    QStringList     getPackageReverseDependList();

private:
    int             m_index                     = -1;
    bool            m_valid                     = false;
    QString         m_name                      = "";
    QString         m_version                   = "";
    QString         m_architecture              = "";
    QByteArray      m_md5                       = "";
    DependsStatus   m_dependsStatus             = DependsUnknown;
    SigntureStatus  m_signtureStatus            = SigntureUnknown;
    InstallStatus   m_installStatus             = InstallStatusUnknown;

    QString         m_packagePath                = "";
    QStringList     m_packageAvailableDependList = {};
    QStringList     m_packageReverseDepends      = {};

private:

    /**
     * @brief m_pSigntureStatus 签名状态类
     */
    PackageSigntureStatus *m_pSigntureStatus    = nullptr;

};

#endif // PACKAGE_H

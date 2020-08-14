/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
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

#include "packagesmanager.h"
#include "deblistmodel.h"

#include <QPair>
#include <QSet>
#include <QtConcurrent>
#include "utils.h"
using namespace QApt;

QString relationName(const RelationType type)
{
    switch (type) {
    case LessOrEqual:
        return "<=";
    case GreaterOrEqual:
        return ">=";
    case LessThan:
        return "<";
    case GreaterThan:
        return ">";
    case Equals:
        return "=";
    case NotEqual:
        return "!=";
    default:;
    }

    return QString();
}

bool isArchMatches(QString sysArch, const QString &packageArch, const int multiArchType)
{
    Q_UNUSED(multiArchType);

    if (sysArch.startsWith(':')) sysArch.remove(0, 1);

    if (sysArch == "all" || sysArch == "any") return true;

    //    if (multiArchType == MultiArchForeign)
    //        return true;

    return sysArch == packageArch;
}

QString resolvMultiArchAnnotation(const QString &annotation, const QString &debArch,
                                  const int multiArchType = InvalidMultiArchType)
{
    if (annotation == "native" || annotation == "any") return QString();
    if (annotation == "all") return QString();
    if (multiArchType == MultiArchForeign) return QString();

    QString arch;
    if (annotation.isEmpty())
        arch = debArch;
    else
        arch = annotation;

    if (!arch.startsWith(':') && !arch.isEmpty())
        return arch.prepend(':');
    else
        return arch;
}

bool dependencyVersionMatch(const int result, const RelationType relation)
{
    switch (relation) {
    case LessOrEqual:
        return result <= 0;
    case GreaterOrEqual:
        return result >= 0;
    case LessThan:
        return result < 0;
    case GreaterThan:
        return result > 0;
    case Equals:
        return result == 0;
    case NotEqual:
        return result != 0;
    default:;
    }

    return true;
}

Backend *init_backend()
{
    Backend *b = new Backend;

    if (b->init()) return b;

    qFatal("%s", b->initErrorMessage().toStdString().c_str());
//    return nullptr;
}

dealDependThread::dealDependThread(QObject *parent)
{
    Q_UNUSED(parent);
    proc = new QProcess(this);
    connect(proc, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this, &dealDependThread::onFinished);
    connect(proc, &QProcess::readyReadStandardOutput, this, &dealDependThread::on_readoutput);
}

dealDependThread::~dealDependThread()
{
    delete proc;
}

void dealDependThread::setDependsList(QStringList dependList, int index)
{
    m_index = index;
    m_dependsList = dependList;
}

void dealDependThread::setBrokenDepend(QString dependName)
{
    m_brokenDepend = dependName;
}

void dealDependThread::on_readoutput()
{
    QString tmp = proc->readAllStandardOutput().data();
    qDebug() << "安装依赖输出数据:" << tmp;

    if (tmp.contains("StartInstallDeepinwine")) {
        emit DependResult(DebListModel::AuthConfirm, m_index, m_brokenDepend);
        return;
    }

    if (tmp.contains("Not authorized")) {
        bDependsStatusErr = true;
        emit DependResult(DebListModel::CancelAuth, m_index, m_brokenDepend);
    }
}

void dealDependThread::onFinished(int num = -1)
{
    if (bDependsStatusErr) {
        bDependsStatusErr = false;
        return;
    }

    if (num == 0) {
        if (bDependsStatusErr) {
            emit DependResult(DebListModel::AnalysisErr, m_index, m_brokenDepend);
            bDependsStatusErr = false;
            return;
        }
        qDebug() << m_dependsList << "安装成功";
        emit DependResult(DebListModel::AuthDependsSuccess, m_index, m_brokenDepend);
    } else {
        if (bDependsStatusErr) {
            emit DependResult(DebListModel::AnalysisErr, m_index, m_brokenDepend);
            bDependsStatusErr = false;
            return;
        }
        qDebug() << m_dependsList << "install error" << num;
        emit DependResult(DebListModel::AuthDependsErr, m_index, m_brokenDepend);
    }
    emit enableCloseButton(true);
}

void dealDependThread::run()
{
    proc->setProcessChannelMode(QProcess::MergedChannels);
    msleep(100);

    emit DependResult(DebListModel::AuthBefore, m_index, m_brokenDepend);
    proc->start("pkexec", QStringList() << "deepin-deb-installer-dependsInstall" << m_dependsList);
    emit enableCloseButton(false);
}

PackagesManager::PackagesManager(QObject *parent)
    : QObject(parent)
{
    m_backendFuture = QtConcurrent::run(init_backend);
    dthread = new dealDependThread();
    connect(dthread, &dealDependThread::DependResult, this, &PackagesManager::DealDependResult);
    connect(dthread, &dealDependThread::enableCloseButton, this, &PackagesManager::enableCloseButton);
}

bool PackagesManager::isBackendReady() { return m_backendFuture.isFinished(); }

bool PackagesManager::isArchError(const int idx)
{
    Backend *b = m_backendFuture.result();
    DebFile deb(m_preparedPackages[idx]);

    const QString arch = deb.architecture();

    if (arch == "all" || arch == "any") return false;

    bool architectures = !b->architectures().contains(deb.architecture());

    return architectures;
}

const ConflictResult PackagesManager::packageConflictStat(const int index)
{
    DebFile p(m_preparedPackages[index]);

    ConflictResult ConflictResult = isConflictSatisfy(p.architecture(), p.conflicts());
    return ConflictResult;
}

const ConflictResult PackagesManager::isConflictSatisfy(const QString &arch, Package *package)
{
    const QString &name = package->name();
    qDebug() << "check conflict for package" << name << arch;

    const auto ret_installed = isInstalledConflict(name, package->version(), package->architecture());
    if (!ret_installed.is_ok()) return ret_installed;

    qDebug() << "check conflict for local installed package is ok.";

    const auto ret_package = isConflictSatisfy(arch, package->conflicts());

    qDebug() << "check finished, conflict is satisfy:" << package->name() << bool(ret_package.is_ok());

    return ret_package;
}

const ConflictResult PackagesManager::isInstalledConflict(const QString &packageName, const QString &packageVersion,
                                                          const QString &packageArch)
{
    static QList<QPair<QString, DependencyInfo>> sysConflicts;

    if (sysConflicts.isEmpty()) {
        Backend *b = m_backendFuture.result();
        for (Package *p : b->availablePackages()) {
            if (!p->isInstalled()) continue;
            const auto &conflicts = p->conflicts();
            if (conflicts.isEmpty()) continue;

            for (const auto &conflict_list : conflicts)
                for (const auto &conflict : conflict_list)
                    sysConflicts << QPair<QString, DependencyInfo>(p->name(), conflict);
        }
    }

    for (const auto &info : sysConflicts) {
        const auto &conflict = info.second;
        const auto &pkgName = conflict.packageName();
        const auto &pkgVersion = conflict.packageVersion();
        const auto &pkgArch = conflict.multiArchAnnotation();

        if (pkgName != packageName) continue;

        qDebug() << pkgName << pkgVersion << pkgArch;

        // pass if arch not match
        if (!pkgArch.isEmpty() && pkgArch != packageArch && pkgArch != "any" && pkgArch != "native") continue;

        if (pkgVersion.isEmpty()) return ConflictResult::err(info.first);

        const int relation = Package::compareVersion(packageVersion, conflict.packageVersion());
        // match, so is bad
        if (dependencyVersionMatch(relation, conflict.relationType())) return ConflictResult::err(info.first);
    }

    return ConflictResult::ok(QString());
}

const ConflictResult PackagesManager::isConflictSatisfy(const QString &arch, const QList<DependencyItem> &conflicts)
{
    for (const auto &conflict_list : conflicts) {
        for (const auto &conflict : conflict_list) {
            const QString name = conflict.packageName();
            Package *p = packageWithArch(name, arch, conflict.multiArchAnnotation());

            if (!p || !p->isInstalled()) continue;

            // arch error, conflicts
            if (!isArchMatches(arch, p->architecture(), p->multiArchType())) {
                qDebug() << "conflicts package installed: " << arch << p->name() << p->architecture()
                         << p->multiArchTypeString();
                return ConflictResult::err(name);
            }

            const QString conflict_version = conflict.packageVersion();
            const QString installed_version = p->installedVersion();
            const auto type = conflict.relationType();
            const auto result = Package::compareVersion(installed_version, conflict_version);

            // not match, ok
            if (!dependencyVersionMatch(result, type)) continue;

            // test package
            const QString mirror_version = p->availableVersion();
            if (mirror_version == installed_version) continue;

            // mirror version is also break
            const auto mirror_result = Package::compareVersion(mirror_version, conflict_version);
            if (dependencyVersionMatch(mirror_result, type)) {
                qDebug() << "conflicts package installed: " << arch << p->name() << p->architecture()
                         << p->multiArchTypeString() << mirror_version << conflict_version;
                return ConflictResult::err(name);
            }
        }
    }

    return ConflictResult::ok(QString());
}

int PackagesManager::packageInstallStatus(const int index)
{
    if (m_packageInstallStatus.contains(index)) return m_packageInstallStatus[index];

    DebFile *deb = new DebFile(m_preparedPackages[index]);

    const QString packageName = deb->packageName();
    const QString packageArch = deb->architecture();
    Backend *b = m_backendFuture.result();
    Package *p = b->package(packageName + ":" + packageArch);

    int ret = DebListModel::NotInstalled;
    do {
        if (!p) break;

        const QString installedVersion = p->installedVersion();
        if (installedVersion.isEmpty()) break;

        const QString packageVersion = deb->version();
        const int result = Package::compareVersion(packageVersion, installedVersion);

        if (result == 0)
            ret = DebListModel::InstalledSameVersion;
        else if (result < 0)
            ret = DebListModel::InstalledLaterVersion;
        else
            ret = DebListModel::InstalledEarlierVersion;
    } while (false);

    m_packageInstallStatus.insert(index, ret);
    delete deb;
    return ret;
}

void PackagesManager::DealDependResult(int iAuthRes, int iIndex, QString dependName)
{
    if (iAuthRes == DebListModel::AuthDependsSuccess) {
        for (int num = 0; num < m_dependInstallMark.size(); num++) {
            m_packageDependsStatus[m_dependInstallMark.at(num)].status = DebListModel::DependsOk;
        }
        m_errorIndex.clear();
    }
    if (iAuthRes == DebListModel::CancelAuth || iAuthRes == DebListModel::AnalysisErr) {
        for (int num = 0; num < m_dependInstallMark.size(); num++) {
            m_packageDependsStatus[m_dependInstallMark[num]].status = DebListModel::DependsAuthCancel;
        }
        emit enableCloseButton(true);
    }
    if (iAuthRes == DebListModel::AuthDependsErr) {
        for (int num = 0; num < m_dependInstallMark.size(); num++) {
            m_packageDependsStatus[m_dependInstallMark.at(num)].status = DebListModel::DependsBreak;
            if (!m_errorIndex.contains(m_dependInstallMark[num]))
                m_errorIndex.push_back(m_dependInstallMark[num]);
        }
        emit enableCloseButton(true);
    }
    emit DependResult(iAuthRes, iIndex, dependName);
}

PackageDependsStatus PackagesManager::packageDependsStatus(const int index)
{
    if (m_packageDependsStatus.contains(index)) {
        return m_packageDependsStatus[index];
    }

    if (isArchError(index)) {
        m_packageDependsStatus[index].status = 2; // fix:24886
        return PackageDependsStatus::_break(QString());
    }

    DebFile *deb = new DebFile(m_preparedPackages[index]);
    const QString architecture = deb->architecture();
    PackageDependsStatus ret = PackageDependsStatus::ok();

    // conflicts
    const ConflictResult debConflitsResult = isConflictSatisfy(architecture, deb->conflicts());

    if (!debConflitsResult.is_ok()) {
        qDebug() << "depends break because conflict" << deb->packageName();
        ret.package = debConflitsResult.unwrap();
        ret.status = DebListModel::DependsBreak;
    } else {
        const ConflictResult localConflictsResult =
            isInstalledConflict(deb->packageName(), deb->version(), architecture);
        if (!localConflictsResult.is_ok()) {
            qDebug() << "depends break because conflict with local package" << deb->packageName();
            ret.package = localConflictsResult.unwrap();
            ret.status = DebListModel::DependsBreak;
        } else {
            qDebug() << "depends:";
            qDebug() << "Check for package" << deb->packageName();
            QSet<QString> choose_set;
            choose_set << deb->packageName();
            ret = checkDependsPackageStatus(choose_set, deb->architecture(), deb->depends());

            QStringList dependList;
            qDebug() << deb->packageName();
            for (auto ditem : deb->depends()) {
                for (auto dinfo : ditem) {
                    qDebug() << dinfo.packageName();
                    dependList << dinfo.packageName() + ":" + deb->architecture();
                }
            }
            qDebug() << ret.package << "\n";
            if (ret.status == DebListModel::DependsBreak) {
                qDebug() << "查找到依赖不满足:" << ret.package;
                if (ret.package.contains("deepin-wine")) {
                    if (!m_dependInstallMark.contains(index)) {
                        if (!dthread->isRunning()) {
                            m_dependInstallMark.append(index);
                            qDebug() << dependList;
                            dthread->setDependsList(dependList, index);
                            dthread->setBrokenDepend(ret.package);
                            dthread->run();
                        }
                    }
                }
            }
        }
    }
    if (ret.isBreak()) Q_ASSERT(!ret.package.isEmpty());

    m_packageDependsStatus[index] = ret;

    qDebug() << "Check finished for package" << deb->packageName() << ret.status;
    if (ret.status == DebListModel::DependsAvailable) {
        const auto list = packageAvailableDepends(index);
        qDebug() << "available depends:" << list.size() << list;
    }
    delete deb;
    return ret;
}

const QString PackagesManager::packageInstalledVersion(const int index)
{
    Q_ASSERT(m_packageInstallStatus.contains(index));
//    Q_ASSERT(m_packageInstallStatus[index] == DebListModel::InstalledEarlierVersion ||
//             m_packageInstallStatus[index] == DebListModel::InstalledLaterVersion);

    DebFile *deb = new DebFile(m_preparedPackages[index]);

    const QString packageName = deb->packageName();
    const QString packageArch = deb->architecture();
    Backend *b = m_backendFuture.result();
    Package *p = b->package(packageName + ":" + packageArch);
//    Package *p = b->package(m_preparedPackages[index]->packageName());
    delete  deb;
    return p->installedVersion();
}

const QStringList PackagesManager::packageAvailableDepends(const int index)
{
    Q_ASSERT(m_packageDependsStatus.contains(index));
    Q_ASSERT(m_packageDependsStatus[index].isAvailable());

    DebFile *deb = new DebFile(m_preparedPackages[index]);
    QSet<QString> choose_set;
    const QString debArch = deb->architecture();
    const auto &depends = deb->depends();
    packageCandidateChoose(choose_set, debArch, depends);

    // TODO: check upgrade from conflicts
    delete deb;
    return choose_set.toList();
}

void PackagesManager::packageCandidateChoose(QSet<QString> &choosed_set, const QString &debArch,
                                             const QList<DependencyItem> &dependsList)
{
    for (auto const &candidate_list : dependsList) packageCandidateChoose(choosed_set, debArch, candidate_list);
}

void PackagesManager::packageCandidateChoose(QSet<QString> &choosed_set, const QString &debArch,
                                             const DependencyItem &candidateList)
{
    bool choosed = false;

    for (const auto &info : candidateList) {
        Package *dep = packageWithArch(info.packageName(), debArch, info.multiArchAnnotation());
        if (!dep) continue;

        const auto choosed_name = dep->name() + resolvMultiArchAnnotation(QString(), dep->architecture());
        if (choosed_set.contains(choosed_name)) {
            choosed = true;
            break;
        }

        // TODO: upgrade?
        if (!dep->installedVersion().isEmpty()) return;

        if (!isConflictSatisfy(debArch, dep->conflicts()).is_ok()) {
            qDebug() << "conflict error in choose candidate" << dep->name();
            continue;
        }

        // pass if break
        QSet<QString> set = choosed_set;
        set << choosed_name;
        const auto stat = checkDependsPackageStatus(set, dep->architecture(), dep->depends());
        if (stat.isBreak()) {
            qDebug() << "depends error in choose candidate" << dep->name();
            continue;
        }

        choosed = true;
        choosed_set << choosed_name;
        packageCandidateChoose(choosed_set, debArch, dep->depends());
        break;
    }

    Q_ASSERT(choosed);
}

QMap<QString, QString> PackagesManager::specialPackage()
{
    QMap<QString, QString> sp;
    sp.insert("deepin-wine-plugin-virtual", "deepin-wine-helper");
    sp.insert("deepin-wine32", "deepin-wine");

    return sp;
}

const QStringList PackagesManager::packageReverseDependsList(const QString &packageName, const QString &sysArch)
{
    Package *package = packageWithArch(packageName, sysArch);
    Q_ASSERT(package);

    QSet<QString> ret{packageName};
    QQueue<QString> testQueue;

    for (const auto &item : package->requiredByList().toSet())
        testQueue.append(item);
    while (!testQueue.isEmpty()) {
        const auto item = testQueue.first();
        testQueue.pop_front();

        if (ret.contains(item)) continue;

        Package *p = packageWithArch(item, sysArch);
        if (!p || !p->isInstalled()) continue;

        if (p->recommendsList().contains(packageName)) continue;
        if (p->suggestsList().contains(packageName)) continue;
        // fix bug: https://pms.uniontech.com/zentao/bug-view-37220.html dde相关组件特殊处理.
        //修复dde会被动卸载但是不会提示的问题
        //if (item.contains("dde")) continue;
        ret << item;

        // fix bug:https://pms.uniontech.com/zentao/bug-view-37220.html
        if (specialPackage().contains(item)) {
            testQueue.append(specialPackage()[item]);
        }
        // append new reqiure list
        for (const auto &r : p->requiredByList()) {
            if (ret.contains(r) || testQueue.contains(r)) continue;
            Package *subPackage = packageWithArch(r, sysArch);
            if (!subPackage || !subPackage->isInstalled())
                continue;
            if (subPackage->recommendsList().contains(item))
                continue;
            if (subPackage->suggestsList().contains(item))
                continue;
            testQueue.append(r);
        }
    }
    // remove self
    ret.remove(packageName);

    return ret.toList();
}

void PackagesManager::reset()
{
    m_errorIndex.clear();
    m_dependInstallMark.clear();
    m_preparedPackages.clear();
    m_packageInstallStatus.clear();
    m_packageDependsStatus.clear();
    m_appendedPackagesMd5.clear();
    m_backendFuture.result()->reloadCache();
}

void PackagesManager::resetInstallStatus()
{
    m_packageInstallStatus.clear();
    m_packageDependsStatus.clear();
    m_backendFuture.result()->reloadCache();
}

void PackagesManager::resetPackageDependsStatus(const int index)
{
    if (!m_packageDependsStatus.contains(index)) return;

    if (m_packageDependsStatus.contains(index)) {
        if ((m_packageDependsStatus[index].package == "deepin-wine") && m_packageDependsStatus[index].status != DebListModel::DependsOk) {
            return;
        }
    }
    // reload backend cache
    m_backendFuture.result()->reloadCache();

    m_packageDependsStatus.remove(index);
}

void PackagesManager::removePackage(const int index, QList<int> listDependInstallMark)
{
    qDebug() << index << ", size:" << m_preparedPackages.size();
    DebFile *deb = new DebFile(m_preparedPackages[index]);
    const auto md5 = deb->md5Sum();
    delete deb;
    //m_appendedPackagesMd5.remove(m_preparedMd5[index]);
    m_appendedPackagesMd5.remove(md5);
    m_preparedPackages.removeAt(index);
    m_preparedMd5.removeAt(index);

    m_dependInstallMark.clear();
    if (listDependInstallMark.size() > 1) {
        for (int i = 0; i < listDependInstallMark.size(); i++) {
            if (index > listDependInstallMark[i]) {
                m_dependInstallMark.append(listDependInstallMark[i]);
            } else if (index != listDependInstallMark[i]) {
                m_dependInstallMark.append(listDependInstallMark[i] - 1);
            }
        }
    }

    QList<int> t_errorIndex;
    if (m_errorIndex.size() > 0) {
        for (int i = 0; i < m_errorIndex.size(); i++) {
            if (index > m_errorIndex[i]) {
                t_errorIndex.append(m_errorIndex[i]);
            } else if (index != m_errorIndex[i]) {
                t_errorIndex.append(m_errorIndex[i] - 1);
            }
        }
    }
    m_errorIndex.clear();
    m_errorIndex = t_errorIndex;

    m_packageInstallStatus.clear();
    //m_packageDependsStatus.clear();
    if (m_packageDependsStatus.contains(index)) {
        if (m_packageDependsStatus.size() > 1) {
            QMapIterator<int, PackagesManagerDependsStatus::PackageDependsStatus> MapIteratorpackageDependsStatus(m_packageDependsStatus);
            QList<PackagesManagerDependsStatus::PackageDependsStatus> listpackageDependsStatus;
            int iDependIndex = 0;
            while (MapIteratorpackageDependsStatus.hasNext()) {
                MapIteratorpackageDependsStatus.next();
                if (index > MapIteratorpackageDependsStatus.key())
                    listpackageDependsStatus.insert(iDependIndex++, MapIteratorpackageDependsStatus.value());
                else if (index != MapIteratorpackageDependsStatus.key()) {
                    listpackageDependsStatus.append(MapIteratorpackageDependsStatus.value());
                }
            }
            m_packageDependsStatus.clear();
            for (int i = 0; i < listpackageDependsStatus.size(); i++)
                m_packageDependsStatus.insert(i, listpackageDependsStatus[i]);
        } else {
            m_packageDependsStatus.clear();
        }
    }
}

void PackagesManager::removeLastPackage()
{
    qDebug() << "start remove last, curr m_preparedPackages size:" << m_preparedPackages.size();
    //DebFile *deb = m_preparedPackages.last();
    //qDebug() << " remove package name is:" << deb->packageName();
    //const auto md5 = deb->md5Sum();

    m_appendedPackagesMd5.remove(m_preparedMd5.last());
    m_preparedPackages.removeLast();
    m_preparedMd5.removeLast();
    m_packageInstallStatus.clear();
    m_packageDependsStatus.clear();
}

bool PackagesManager::appendPackage(QString debPackage)
{
    qDebug() << "debPackage" << debPackage;
    QApt::DebFile *p = new DebFile(debPackage);

    const auto md5 = p->md5Sum();
    if (m_appendedPackagesMd5.contains(md5)) return false;

    m_preparedPackages << debPackage;
    m_appendedPackagesMd5 << md5;
    m_preparedMd5 << md5;

    delete p;
    return true;
}

const PackageDependsStatus PackagesManager::checkDependsPackageStatus(QSet<QString> &choosed_set,
                                                                      const QString &architecture,
                                                                      const QList<DependencyItem> &depends)
{
    PackageDependsStatus ret = PackageDependsStatus::ok();

    for (const auto &candicate_list : depends) {
        const auto r = checkDependsPackageStatus(choosed_set, architecture, candicate_list);
        ret.maxEq(r);

        if (ret.isBreak()) break;
    }

    return ret;
}

const PackageDependsStatus PackagesManager::checkDependsPackageStatus(QSet<QString> &choosed_set,
                                                                      const QString &architecture,
                                                                      const DependencyItem &candicate)
{
    PackageDependsStatus ret = PackageDependsStatus::_break(QString());

    for (const auto &info : candicate) {
        const auto r = checkDependsPackageStatus(choosed_set, architecture, info);
        ret.minEq(r);

        if (!ret.isBreak()) break;
    }

    return ret;
}

const PackageDependsStatus PackagesManager::checkDependsPackageStatus(QSet<QString> &choosed_set,
                                                                      const QString &architecture,
                                                                      const DependencyInfo &dependencyInfo)
{
    const QString package_name = dependencyInfo.packageName();

    Package *p = packageWithArch(package_name, architecture, dependencyInfo.multiArchAnnotation());

    if (!p) {
        qDebug() << "depends break because package" << package_name << "not available";
        return PackageDependsStatus::_break(package_name);
    }

    qDebug() << DependencyInfo::typeName(dependencyInfo.dependencyType()) << package_name << p->architecture()
             << relationName(dependencyInfo.relationType()) << dependencyInfo.packageVersion();

    const RelationType relation = dependencyInfo.relationType();
    const QString &installedVersion = p->installedVersion();

    if (!installedVersion.isEmpty()) {
        const int result = Package::compareVersion(installedVersion, dependencyInfo.packageVersion());
        if (dependencyVersionMatch(result, relation))
            return PackageDependsStatus::ok();
        else {
            const QString &mirror_version = p->availableVersion();
            if (mirror_version != installedVersion) {
                const auto mirror_result = Package::compareVersion(mirror_version, dependencyInfo.packageVersion());

                if (dependencyVersionMatch(mirror_result, relation)) {
                    qDebug() << "availble by upgrade package" << p->name() << p->architecture() << "from"
                             << installedVersion << "to" << mirror_version;
                    return PackageDependsStatus::available();
                }
            }

            qDebug() << "depends break by" << p->name() << p->architecture() << dependencyInfo.packageVersion();
            qDebug() << "installed version not match" << installedVersion;
            return PackageDependsStatus::_break(p->name());
        }
    } else {
        const int result = Package::compareVersion(p->version(), dependencyInfo.packageVersion());
        if (!dependencyVersionMatch(result, relation)) {
            qDebug() << "depends break by" << p->name() << p->architecture() << dependencyInfo.packageVersion();
            qDebug() << "available version not match" << p->version();
            return PackageDependsStatus::_break(p->name());
        }

        // is that already choosed?
        if (choosed_set.contains(p->name())) return PackageDependsStatus::ok();

        // check arch conflicts
        if (p->multiArchType() == MultiArchSame) {
            Backend *b = backend();
            for (const auto &arch : b->architectures()) {
                if (arch == p->architecture()) continue;

                Package *tp = b->package(p->name() + ":" + arch);
                if (tp && tp->isInstalled()) {
                    qDebug() << "multi arch installed: " << p->name() << p->version() << p->architecture() << "with"
                             << tp->name() << tp->version() << tp->architecture();
                    return PackageDependsStatus::_break(p->name() + ":" + p->architecture());
                }
            }
        }
        // let's check conflicts
        if (!isConflictSatisfy(architecture, p).is_ok()) {
            qDebug() << "depends break because conflict, ready to find providers" << p->name();

            Backend *b = m_backendFuture.result();
            for (auto *ap : b->availablePackages()) {
                if (!ap->providesList().contains(p->name())) continue;

                // is that already provide by another package?
                if (ap->isInstalled()) {
                    qDebug() << "find a exist provider: " << ap->name();
                    return PackageDependsStatus::ok();
                }

                // provider is ok, switch to provider.
                if (isConflictSatisfy(architecture, ap).is_ok()) {
                    qDebug() << "switch to depends a new provider: " << ap->name();
                    choosed_set << ap->name();
                    return PackageDependsStatus::ok();
                }
            }

            qDebug() << "providers not found, still break: " << p->name();
            return PackageDependsStatus::_break(p->name());
        }

        // now, package dependencies status is available or break,
        // time to check depends' dependencies, but first, we need
        // to add this package to choose list
        choosed_set << p->name();

        qDebug() << "Check indirect dependencies for package" << p->name();

        const auto r = checkDependsPackageStatus(choosed_set, p->architecture(), p->depends());
        if (r.isBreak()) {
            choosed_set.remove(p->name());
            qDebug() << "depends break by direct depends" << p->name() << p->architecture() << r.package;
            return PackageDependsStatus::_break(p->name());
        }

        qDebug() << "Check finshed for package" << p->name();

        return PackageDependsStatus::available();
    }
}

Package *PackagesManager::packageWithArch(const QString &packageName, const QString &sysArch,
                                          const QString &annotation)
{
    qDebug() << "package with arch" << packageName << sysArch << annotation;
    Backend *b = m_backendFuture.result();
    Package *p = b->package(packageName + resolvMultiArchAnnotation(annotation, sysArch));
    do {
        // change: 按照当前支持的CPU架构进行打包。取消对deepin-wine的特殊处理
        if (!p) p = b->package(packageName);
        if (p) break;
        for (QString arch : b->architectures()) {
            if (!p) p = b->package(packageName + ":" + arch);
            if (p) break;
        }

    } while (false);

    if (p) return p;

    qDebug() << "check virtual package providers for package" << packageName << sysArch << annotation;
    // check virtual package providers
    for (auto *ap : b->availablePackages())
        if (ap->name() != packageName && ap->providesList().contains(packageName))
            return packageWithArch(ap->name(), sysArch, annotation);

    return nullptr;
}

PackagesManager::~PackagesManager()
{
    delete dthread;
}

PackageDependsStatus PackageDependsStatus::ok() { return {DebListModel::DependsOk, QString()}; }

PackageDependsStatus PackageDependsStatus::available() { return {DebListModel::DependsAvailable, QString()}; }

PackageDependsStatus PackageDependsStatus::_break(const QString &package)
{
    return {DebListModel::DependsBreak, package};
}

PackageDependsStatus::PackageDependsStatus()
    : PackageDependsStatus(DebListModel::DependsOk, QString()) {}

PackageDependsStatus::PackageDependsStatus(const int status, const QString &package)
    : status(status)
    , package(package) {}

PackageDependsStatus PackageDependsStatus::operator=(const PackageDependsStatus &other)
{
    status = other.status;
    package = other.package;

    return *this;
}

PackageDependsStatus PackageDependsStatus::max(const PackageDependsStatus &other)
{
    if (other.status > status) *this = other;

    return *this;
}

PackageDependsStatus PackageDependsStatus::maxEq(const PackageDependsStatus &other)
{
    if (other.status >= status) *this = other;

    return *this;
}

PackageDependsStatus PackageDependsStatus::min(const PackageDependsStatus &other)
{
    if (other.status < status) *this = other;

    return *this;
}

PackageDependsStatus PackageDependsStatus::minEq(const PackageDependsStatus &other)
{
    if (other.status <= status) *this = other;

    return *this;
}

bool PackageDependsStatus::isBreak() const { return status == DebListModel::DependsBreak; }

bool PackageDependsStatus::isAuthCancel() const { return status == DebListModel::DependsAuthCancel; }

bool PackageDependsStatus::isAvailable() const { return status == DebListModel::DependsAvailable; }

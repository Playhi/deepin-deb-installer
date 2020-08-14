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

#include "multipleinstallpage.h"
#include "deblistmodel.h"
#include "packagelistview.h"
#include "packageslistdelegate.h"
#include "workerprogress.h"
#include "utils.h"

#include <QApplication>
#include <QPropertyAnimation>
#include <QTimer>
#include <QVBoxLayout>

#include <DLabel>

MultipleInstallPage::MultipleInstallPage(DebListModel *model, QWidget *parent)
    : QWidget(parent)
    , m_debListModel(model)
    , m_contentFrame(new QWidget(this))
    , m_appsListView(new PackagesListView(this))
    , m_appsListViewBgFrame(new DRoundBgFrame(this, 10, 0))
    , m_installProcessInfoView(new InstallProcessInfoView(this))
    , m_infoControlButton(new InfoControlButton(tr("Show details"), tr("Collapse"), this))
    , m_processFrame(new QWidget(this))
    , m_installProgress(nullptr)
    , m_progressAnimation(nullptr)
    , m_installButton(new DPushButton(this))
    , m_acceptButton(new DPushButton(this))
    , m_backButton(new DPushButton(this))
    , m_contentLayout(new QVBoxLayout(this))
    , m_centralLayout(new QVBoxLayout(this))
      // fix bug:33999 change DButton to DCommandLinkButton for Activity color
    , m_tipsLabel(new DCommandLinkButton("", this))
    , m_dSpinner(new DSpinner(this))
{
    initContentLayout();
    initUI();
    initConnections();
}

void MultipleInstallPage::initContentLayout()
{
    m_contentLayout->addSpacing(10);
    m_contentLayout->setSpacing(0);
    m_contentLayout->setContentsMargins(10, 0, 10, 0);
    m_contentFrame->setLayout(m_contentLayout);
    m_centralLayout->addWidget(m_contentFrame);

    m_centralLayout->setSpacing(0);
    m_centralLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(m_centralLayout);

//#define SHOWBGCOLOR
#ifdef SHOWBGCOLOR
    m_contentFrame->setStyleSheet("QFrame{background: cyan}");
#endif
}

void MultipleInstallPage::initUI()
{
    PackagesListDelegate *delegate = new PackagesListDelegate(m_appsListView);

    //fix bug:33730
    m_appsListViewBgFrame->setFixedSize(460, 186/* + 10*/ + 5);
    QVBoxLayout *appsViewLayout = new QVBoxLayout(this);
    appsViewLayout->setSpacing(0);
    appsViewLayout->setContentsMargins(0, 0, 0, 0);
    m_appsListViewBgFrame->setLayout(appsViewLayout);

    m_appsListView->setModel(m_debListModel);
    m_appsListView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_appsListView->setItemDelegate(delegate);
    appsViewLayout->addSpacing(10);
    appsViewLayout->addWidget(m_appsListView);

    m_installButton->setFixedSize(120, 36);
    m_acceptButton->setFixedSize(120, 36);
    m_backButton->setFixedSize(120, 36);

    m_installButton->setText(tr("Install"));
    m_acceptButton->setText(tr("Done"));
    m_acceptButton->setVisible(false);
    m_backButton->setText(tr("Back"));
    m_backButton->setVisible(false);


    QString mediumFontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansMedium);

    Utils::bindFontBySizeAndWeight(m_installButton, mediumFontFamily, 14, QFont::Medium);
    Utils::bindFontBySizeAndWeight(m_acceptButton, mediumFontFamily, 14, QFont::Medium);
    Utils::bindFontBySizeAndWeight(m_backButton, mediumFontFamily, 14, QFont::Medium);

    m_installButton->setFocusPolicy(Qt::NoFocus);
    m_acceptButton->setFocusPolicy(Qt::NoFocus);
    m_backButton->setFocusPolicy(Qt::NoFocus);

    m_tipsLabel->setFixedHeight(24);
    QString fontFamily = Utils::loadFontFamilyByType(Utils::SourceHanSansNormal);
    Utils::bindFontBySizeAndWeight(m_tipsLabel, fontFamily, 12, QFont::ExtraLight);

    //fix bug:33999 Make the DCommandLinkbutton looks like a Lable O_o
    m_tipsLabel->setEnabled(false);

    m_dSpinner->setMinimumSize(20, 20);
    m_dSpinner->hide();

    m_installProcessInfoView->setVisible(false);
    m_installProcessInfoView->setAcceptDrops(false);
    m_installProcessInfoView->setFixedHeight(200);
    m_installProcessInfoView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_infoControlButton->setVisible(false);

    QVBoxLayout *progressFrameLayout = new QVBoxLayout(this);
    progressFrameLayout->setSpacing(0);
    progressFrameLayout->setContentsMargins(0, 0, 0, 0);
    m_processFrame->setLayout(progressFrameLayout);
    m_installProgress = new WorkerProgress(this);
    m_progressAnimation = new QPropertyAnimation(m_installProgress, "value", this);
    progressFrameLayout->addStretch();
    progressFrameLayout->addWidget(m_installProgress);
    progressFrameLayout->setAlignment(m_installProgress, Qt::AlignHCenter);
    m_processFrame->setVisible(false);
    m_processFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_processFrame->setFixedHeight(53);

    QVBoxLayout *btnsFrameLayout = new QVBoxLayout(this);
    btnsFrameLayout->setSpacing(0);
    btnsFrameLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *btnsLayout = new QHBoxLayout(this);
    btnsLayout->addStretch();
    btnsLayout->addWidget(m_installButton);
    btnsLayout->addWidget(m_backButton);
    btnsLayout->addWidget(m_acceptButton);
    btnsLayout->setSpacing(20);
    btnsLayout->addStretch();
    btnsLayout->setContentsMargins(0, 0, 0, 30);

    QWidget *btnsFrame = new QWidget(this);
    btnsFrameLayout->addWidget(m_processFrame);
    btnsFrameLayout->addStretch();
    btnsFrameLayout->addLayout(btnsLayout);
    btnsFrame->setLayout(btnsFrameLayout);

    m_contentLayout->addWidget(m_appsListViewBgFrame, Qt::AlignHCenter);
    m_contentLayout->addWidget(m_infoControlButton);
    m_contentLayout->setAlignment(m_infoControlButton, Qt::AlignHCenter);
    m_contentLayout->addWidget(m_installProcessInfoView);

    m_contentLayout->addStretch();
    m_contentLayout->addWidget(m_dSpinner);
    m_contentLayout->addSpacing(4);
    m_contentLayout->addWidget(m_tipsLabel);

    //fix bug:33999 keep tips in the middle
    m_contentLayout->setAlignment(m_tipsLabel, Qt::AlignCenter);
    m_tipsLabel->setVisible(false);

    m_contentLayout->addWidget(btnsFrame);
}

void MultipleInstallPage::initConnections()
{
    connect(m_infoControlButton, &InfoControlButton::expand, this, &MultipleInstallPage::showInfo);
    connect(m_infoControlButton, &InfoControlButton::shrink, this, &MultipleInstallPage::hideInfo);
    connect(m_installButton, &DPushButton::clicked, m_debListModel, &DebListModel::installPackages);
    connect(m_installButton, &DPushButton::clicked, this, &MultipleInstallPage::hiddenCancelButton);
    connect(m_backButton, &DPushButton::clicked, this, &MultipleInstallPage::back);
    connect(m_acceptButton, &DPushButton::clicked, qApp, &QApplication::quit);

    connect(m_appsListView, &PackagesListView::onRemoveItemClicked, this, &MultipleInstallPage::onRequestRemoveItemClicked);
//    connect(m_appsListView, &PackagesListView::entered, m_debListModel, &DebListModel::setCurrentIndex);

    connect(m_debListModel, &DebListModel::workerProgressChanged, this, &MultipleInstallPage::onProgressChanged);
    connect(m_debListModel, &DebListModel::appendOutputInfo, this, &MultipleInstallPage::onOutputAvailable);
    connect(m_debListModel, &DebListModel::onStartInstall, this, [ = ] {
        m_processFrame->setVisible(true);
    });
    connect(m_debListModel, &DebListModel::onChangeOperateIndex, this, &MultipleInstallPage::onAutoScrollInstallList);
    connect(m_debListModel, &DebListModel::DependResult, this, &MultipleInstallPage::DealDependResult);
}

void MultipleInstallPage::onWorkerFinshed()
{
    m_acceptButton->setVisible(true);
    m_backButton->setVisible(true);
    m_processFrame->setVisible(false);
}

void MultipleInstallPage::onOutputAvailable(const QString &output)
{
    m_installProcessInfoView->appendText(output.trimmed());

    // change to install
    if (!m_installButton->isVisible()) {
        //m_installButton->setVisible(false);

//        m_processFrame->setVisible(true);
        m_infoControlButton->setVisible(true);
    }
}

void MultipleInstallPage::onProgressChanged(const int progress)
{
    m_progressAnimation->setStartValue(m_installProgress->value());
    m_progressAnimation->setEndValue(progress);
    m_progressAnimation->start();

    // finished
    if (progress == 100) {
        onOutputAvailable(QString());
        QTimer::singleShot(m_progressAnimation->duration(), this, &MultipleInstallPage::onWorkerFinshed);
    }
}

void MultipleInstallPage::onAutoScrollInstallList(int opIndex)
{
    if (opIndex > 1 && opIndex < m_debListModel->getInstallFileSize()) {
        QModelIndex currIndex = m_debListModel->index(opIndex - 1);
        m_appsListView->scrollTo(currIndex, QAbstractItemView::PositionAtTop);
    } else if (opIndex == -1) { //to top
        QModelIndex currIndex = m_debListModel->index(0);
        m_appsListView->scrollTo(currIndex);
    }
}

void MultipleInstallPage::onRequestRemoveItemClicked(const QModelIndex &index)
{
    if (!m_debListModel->isWorkerPrepare()) return;

    const int r = index.row();

    emit requestRemovePackage(r);
}

void MultipleInstallPage::showInfo()
{
    m_contentLayout->setContentsMargins(20, 0, 20, 0);
    m_appsListViewBgFrame->setVisible(false);
    m_appsListView->setVisible(false);
    m_installProcessInfoView->setVisible(true);
    emit hideAutoBarTitle();
}

void MultipleInstallPage::hideInfo()
{
    m_contentLayout->setContentsMargins(10, 0, 10, 0);
    m_appsListViewBgFrame->setVisible(true);
    m_appsListView->setVisible(true);
    m_installProcessInfoView->setVisible(false);
    emit hideAutoBarTitle();
}

void MultipleInstallPage::hiddenCancelButton()
{
    m_backButton->setVisible(false);
    m_installButton->setVisible(false);
}

void MultipleInstallPage::setEnableButton(bool bEnable)
{
    m_installButton->setEnabled(bEnable);
}

void MultipleInstallPage::afterGetAutherFalse()
{
    m_infoControlButton->setVisible(false); //取消安装后只显示安装按钮
    m_processFrame->setVisible(false);
//    m_backButton->setVisible(true);//取消安装之后，只显示安装按钮，
    m_installButton->setVisible(true);
}

void MultipleInstallPage::onScrollSlotFinshed()
{
    int row = m_appsListView->count();
    QModelIndex currIndex;
    if (m_index == -1) {
        if (row > 0) {
            currIndex = m_debListModel->index(row - 1);
            m_appsListView->scrollTo(currIndex, QAbstractItemView::EnsureVisible);
        }
    } else {
        if (m_index == 0) {
            currIndex = m_debListModel->index(0);
        } else if (m_index == row) {
            currIndex = m_debListModel->index(m_index - 1);
        } else {
            currIndex = m_debListModel->index(m_index);
        }
        m_appsListView->scrollTo(currIndex, QAbstractItemView::EnsureVisible);
    }
    m_appsListView->reset();
}

void MultipleInstallPage::setScrollBottom(int index)
{
    m_index = index;
    QTimer::singleShot(1, this, &MultipleInstallPage::onScrollSlotFinshed);
}

void MultipleInstallPage::DealDependResult(int iAuthRes, QString dependName)
{
    qDebug() << "批量处理鉴权结果：" << iAuthRes;
    switch (iAuthRes) {
    case DebListModel::AuthBefore:
        m_appsListView->setEnabled(false);
        m_installButton->setVisible(true);
        m_installButton->setEnabled(false);
        m_dSpinner->stop();
        m_dSpinner->hide();
        m_tipsLabel->setVisible(false);
        break;
    case DebListModel::CancelAuth:
        m_appsListView->setEnabled(true);
        m_installButton->setVisible(true);
        m_installButton->setEnabled(true);
        break;
    case DebListModel::AuthConfirm:
        m_appsListView->setEnabled(false);
        m_tipsLabel->setText(tr("Installing dependencies: %1").arg(dependName));
        m_tipsLabel->setVisible(true);
        m_dSpinner->show();
        m_dSpinner->start();
        m_installButton->setVisible(false);
        break;
    case DebListModel::AuthDependsSuccess:
    case DebListModel::AuthDependsErr:
        m_appsListView->setEnabled(true);
        m_installButton->setVisible(true);
        m_installButton->setEnabled(true);
        m_dSpinner->stop();
        m_dSpinner->hide();
        m_tipsLabel->setVisible(false);
        break;
    case DebListModel::AnalysisErr:
        m_appsListView->setEnabled(true);
        m_installButton->setVisible(true);
        m_installButton->setEnabled(true);
        m_dSpinner->stop();
        m_dSpinner->hide();
        m_tipsLabel->setVisible(false);
        break;
    default:
        break;
    }
}

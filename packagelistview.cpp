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

#include "packagelistview.h"
#include "deblistmodel.h"
#include "utils.h"

#include <QPainter>
#include <QShortcut>

PackagesListView::PackagesListView(QWidget *parent)
    : DListView(parent)
    , m_bLeftMouse(false)
    , m_rightMenu(nullptr)
{
    initUI();
    initConnections();
    initRightContextMenu();
    initShortcuts();
}

void PackagesListView::initUI()
{
    setVerticalScrollMode(ScrollPerPixel);
    setSelectionMode(QListView::SingleSelection);
    setAutoScroll(true);
    setMouseTracking(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);
}

void PackagesListView::initConnections()
{
    connect(this,
            &PackagesListView::onShowContextMenu,
            this,
            &PackagesListView::onListViewShowContextMenu,
            Qt::ConnectionType::QueuedConnection);
}

void PackagesListView::initShortcuts()
{
    QShortcut *deleteShortcut = new QShortcut(QKeySequence::Delete, this);
    deleteShortcut->setContext(Qt::ApplicationShortcut);
    connect(deleteShortcut, SIGNAL(activated()), this, SLOT(onShortcutDeleteAction()));
}

void PackagesListView::leaveEvent(QEvent *e)
{
    DListView::leaveEvent(e);

    emit entered(QModelIndex());
}

void PackagesListView::mouseMoveEvent(QMouseEvent *event)
{
    DListView::mouseMoveEvent(event);
}

void PackagesListView::handleHideShowSelection()
{
    if (visualRect(m_highlightIndex).y() < -1 * (48-1))
    {
        emit onShowHideTopBg(false);
    }
    else
    {
        if (m_highlightIndex.row() > 0)
        {
            if (visualRect(m_highlightIndex).y() < 0)
            {
                emit onShowHideTopBg(true);
            }
            else
            {
                if (visualRect(m_highlightIndex).y() >= 48*3-10-8)
                {
                    if (visualRect(m_highlightIndex).y() >= 48*4-10-8)
                    {
                        emit onShowHideBottomBg(false);
                    }
                    else
                    {
                        emit onShowHideBottomBg(true);
                    }
                }
                else
                {
                    emit onShowHideBottomBg(false);
                }
            }
        }
        else
        {
            emit onShowHideTopBg(true);
        }
    }
}

void PackagesListView::scrollContentsBy(int dx, int dy)
{
    if (-1 == m_highlightIndex.row()) {
        QListView::scrollContentsBy(dx, dy);
        return;
    }

    handleHideShowSelection();

    QListView::scrollContentsBy(dx, dy);
}

void PackagesListView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_bLeftMouse = true;
    }
    else
    {
        m_bLeftMouse = false;
    }

    DListView::mousePressEvent(event);
}

void PackagesListView::mouseReleaseEvent(QMouseEvent *event)
{
    DebListModel *debListModel = qobject_cast<DebListModel *>(this->model());
    if (!debListModel->isWorkerPrepare())
    {
        return;
    }

    DListView::mouseReleaseEvent(event);
}

void PackagesListView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    DListView::setSelection(rect, command);

    QPoint clickPoint(rect.x(), rect.y());
    QModelIndex modelIndex = indexAt(clickPoint);

    m_highlightIndex = modelIndex;
    m_currModelIndex = m_highlightIndex;
    if (!m_bLeftMouse)
    {
        m_bShortcutDelete = false;
        emit onShowContextMenu(modelIndex);
    }
    else
    {
        m_bShortcutDelete = true;
    }

    handleHideShowSelection();
}

void PackagesListView::initRightContextMenu()
{
    if (nullptr == m_rightMenu)
    {
        m_rightMenu = new DMenu(this);

        //给右键菜单添加快捷键Delete
        QAction *deleteAction = new QAction(tr("Delete"), this);

        m_rightMenu->addAction(deleteAction);
        connect(deleteAction, SIGNAL(triggered()), this, SLOT(onRightMenuDeleteAction()));
    }
}

void PackagesListView::onListViewShowContextMenu(QModelIndex index)
{
    Q_UNUSED(index)

    m_bShortcutDelete = false;
    m_currModelIndex = index;
    DMenu *rightMenu = m_rightMenu;

    const int operate_stat = index.data(DebListModel::PackageOperateStatusRole).toInt();
    if (DebListModel::Success == operate_stat ||
        DebListModel::Waiting == operate_stat ||
        DebListModel::Operating == operate_stat)
    {
        return;
    }

    //在当前鼠标位置显示右键菜单
    rightMenu->exec(QCursor::pos());
}

void PackagesListView::onShortcutDeleteAction()
{
    if (-1 == m_currModelIndex.row() || m_rightMenu->isVisible())
    {
        return;
    }

    emit onRemoveItemClicked(m_currModelIndex);
}

void PackagesListView::onRightMenuDeleteAction()
{
    if (-1 == m_currModelIndex.row())
    {
        return;
    }

    emit onRemoveItemClicked(m_currModelIndex);
}

void PackagesListView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this->viewport());
    painter.setRenderHint(QPainter::Antialiasing, true);

    DPalette pa = DApplicationHelper::instance()->palette(this);
    QPainterPath painterPath;

//    qDebug() << this->model()->rowCount() << endl;
    QColor color = pa.color(QPalette::Base);
    if (this->model()->rowCount() <= 3)
    {
        QRect rect = this->rect();
        rect.setY(48);
        painterPath.addRect(rect);
        color = pa.color(QPalette::Base);
    }
    else
    {
        painterPath.addRect(this->rect());
        color = pa.color(QPalette::Window);
    }

    painter.fillPath(painterPath, QBrush(color));

    DListView::paintEvent(event);
}


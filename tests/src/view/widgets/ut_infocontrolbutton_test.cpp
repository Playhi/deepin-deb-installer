/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      zhangkai <zhangkai@uniontech.com>
* Maintainer:  zhangkai <zhangkai@uniontech.com>
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

#include "../deb-installer/view/widgets/infocontrolbutton.h"

#include <ut_Head.h>

#include <QMouseEvent>
#include <QKeyEvent>
#include <QCoreApplication>

#include <gtest/gtest.h>

class InfoControlButton_Test : public UT_HEAD
{
public:
    virtual void SetUp()
    {
        m_infoControlBtn = new InfoControlButton("", "");
    }
    void TearDown()
    {
        delete m_infoControlBtn;
    }
    InfoControlButton *m_infoControlBtn;
};

TEST_F(InfoControlButton_Test, InfoControlButton_UT_setExpandTips)
{
    m_infoControlBtn->setExpandTips("");
}

TEST_F(InfoControlButton_Test, InfoControlButton_UT_setShrinkTips)
{
    m_infoControlBtn->setShrinkTips("");
}

TEST_F(InfoControlButton_Test, InfoControlButton_UT_onMouseRelease)
{
    m_infoControlBtn->onMouseRelease();
    m_infoControlBtn->m_expand = true;
    m_infoControlBtn->onMouseRelease();
    EXPECT_FALSE(m_infoControlBtn->m_expand);
}

TEST_F(InfoControlButton_Test, InfoControlButton_UT_themeChanged)
{
    m_infoControlBtn->themeChanged();
    m_infoControlBtn->m_expand = true;
    m_infoControlBtn->themeChanged();
    EXPECT_TRUE(m_infoControlBtn->m_expand);
}

TEST_F(InfoControlButton_Test, InfoControlButton_UT_mouseReleaseEvent)
{
    QMouseEvent mouseReleaseEvent(QEvent::MouseButtonRelease, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_infoControlBtn->mouseReleaseEvent(&mouseReleaseEvent);
}

TEST_F(InfoControlButton_Test, InfoControlButton_UT_keyPressEvent)
{
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QCoreApplication::sendEvent(m_infoControlBtn, &keyPressEvent);
}

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


#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QHash>
#include <QWidget>

#include <DPalette>
#include <DApplicationHelper>
#include <DFontSizeManager>
#include <QDir>
#include <DDialog>
#include <QPushButton>
#include <DPushButton>
#define dApp (static_cast<DApplication *>(QCoreApplication::instance()))

DWIDGET_USE_NAMESPACE

class DebApplicationHelper : public DGuiApplicationHelper
{
    Q_OBJECT

public:
    static DebApplicationHelper *instance();

    DPalette standardPalette(DGuiApplicationHelper::ColorType type) const;
    DPalette palette(const QWidget *widget, const QPalette &base = QPalette()) const;
    void setPalette(QWidget *widget, const DPalette &palette);
    void resetPalette(QWidget *widget);
};

class Utils : public QObject
{
    Q_OBJECT

public:
    Utils(QObject *parent = nullptr);

    enum FontType {
        SourceHanSansMedium,
        SourceHanSansNormal,
        DefautFont
    };

    enum VerifyResultCode {
        OtherError = -1,     //其他错误
        VerifySuccess,       //验证成功
        DebfileInexistence,  //解压deb文件用的临时目录不存在
        ExtractDebFail,      //提取deb包内容时出错
        DebVerifyFail        //deb包验证失败
    };

    static QHash<QString, QPixmap> m_imgCacheHash;
    static QHash<QString, QString> m_fontNameCache;

    static QString loadFontFamilyByType(FontType fontType);//加载字体类型
    static QFont loadFontBySizeAndWeight(QString fontFamily, int fontSize, int fontWeight);//设置字体大小等
    static void bindFontBySizeAndWeight(QWidget *widget, QString fontFamily, int fontSize, int fontWeight);//绑定字体大小等
    static QString fromSpecialEncoding(const QString &inputStr);//字体编码处理
    static QString holdTextInRect(const QFont &font, QString srcText, const QSize &size);//针对字体截断处理函数
    static QString holdTextInRect(const QFont &font, QString srcText, const int &width);//针对配置界面字体截断处理函数
    static int returnfileIsempty(QString strfilepath, QString strfilename); //返回文件是否存在
    static VerifyResultCode Digital_Verify(QString filepath_name); //验证deb数字签名
    static bool Return_Digital_Verify(QString strfilepath, QString strfilename); //返回验证工具是否存在
};

#endif

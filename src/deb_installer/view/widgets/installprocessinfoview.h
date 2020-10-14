#ifndef INSTALLPROCESSINFOVIEW_H
#define INSTALLPROCESSINFOVIEW_H

#include <QPainter>
#include <QPaintEvent>
#include <QTextEdit>
#include <DPalette>

DGUI_USE_NAMESPACE

class InstallProcessInfoView : public QWidget
{
    Q_OBJECT
public:
    explicit InstallProcessInfoView(int w, int h, QWidget *parent = nullptr);
    virtual ~InstallProcessInfoView() override;

    /**
     * @brief appendTextx 向installProcessInfo中添加数据
     * @param text 要添加的数据
     */
    void appendText(QString text);

    /**
     * @brief setTextFontSize 设置字体大小
     * @param fontSize      字体大小   PS： 此参数无用
     * @param fontWeight    字体大小
     */
    void setTextFontSize(int fontSize, int fontWeight);

    /**
     * @brief setTextColor  设置文字颜色类型
     * @param ct    颜色类型
     */
    void setTextColor(DPalette::ColorType ct);

    /**
     * @brief clearText 清空目前installProcessInfo中的数据
     */
    void clearText();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    /**
     * @brief initUI 初始化ProcessInfo的大小
     * @param w 控件的宽度
     * @param h 控件的高度
     * 此处在SP3之后修改，增加宽度高度参数
     * 为适应配置框的大小与安装器installProcessInfo的大小
     */
    void initUI(int w, int h);

    QTextEdit *m_editor;                               //展示框
    DPalette::ColorType m_colorType;                   //显示的字体的颜色类型
};

#endif // INSTALLPROCESSINFOVIEW_H

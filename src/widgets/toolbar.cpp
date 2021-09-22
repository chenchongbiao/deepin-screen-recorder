/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     Hou Lei <houlei@uniontech.com>
 *
 * Maintainer: Liu Zheng <liuzheng@uniontech.com>
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

#include "toolbar.h"
#include "../utils/baseutils.h"
#include "../utils/saveutils.h"
#include "../utils.h"
#include "../utils/configsettings.h"
#include "../accessibility/acTextDefine.h"
#include "../main_window.h"

#include <DIconButton>

#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QCursor>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QSettings>
#include <QBitmap>

#include <dgraphicsgloweffect.h>

DWIDGET_USE_NAMESPACE

namespace {
const int TOOLBAR_HEIGHT = 70;
const int TOOLBAR_WIDTH = 425;

//const QSize TOOLBAR_WIDGET_SIZE = QSize(530, 70);
//const int BUTTON_SPACING = 3;
//const int BTN_RADIUS = 3;
}

ToolBarWidget::ToolBarWidget(MainWindow* pMainwindow,DWidget *parent)
    : DFloatingWidget(parent)
{
    int t_themeType = ConfigSettings::instance()->value("common", "themeType").toInt();
    setBlurBackgroundEnabled(true);
    blurBackground()->setRadius(30);
    blurBackground()->setMode(DBlurEffectWidget::GaussianBlur);
    blurBackground()->setBlurEnabled(true);
    blurBackground()->setBlendMode(DBlurEffectWidget::InWindowBlend);

    if (t_themeType == 1) {
        blurBackground()->setMaskColor(QColor(255, 255, 255, 76));
    } else if (t_themeType == 2) {
        blurBackground()->setMaskColor(QColor(0, 0, 0, 76));
    }

    setFixedHeight(TOOLBAR_HEIGHT);
    if(Utils::is3rdInterfaceStart){
        setFixedWidth(TOOLBAR_WIDTH);
    }

    m_hSeparatorLine = new DLabel(this);
    m_hSeparatorLine->setObjectName("HorSeparatorLine");
    m_hSeparatorLine->setFixedHeight(1);

    m_mainTool = new MainToolWidget(this);
    //分配pMainwindow主窗口指针给SubToolWidget（ToolTips需要该指针）
    m_subTool = new SubToolWidget(pMainwindow,this);
    QString button_style = "DPushButton{border-radius:30px;} "
                           "DPushButton::hover{border-image: url(:/image/newUI/hover/close-hover.svg)}";

    QPixmap pixmap(":/newUI/normal/close-normal.svg");
    //DIconButton
    m_closeButton = new DImageButton(this);
    Utils::setAccessibility(m_closeButton, AC_TOOLBARWIDGET_CLOSE_BUTTON_TOOL);

    if (t_themeType == 1) {
        m_closeButton->setHoverPic(":/newUI/hover/close-hover.svg");
        m_closeButton->setNormalPic(":/newUI/normal/close-normal.svg");
    } else if (t_themeType == 2) {
        m_closeButton->setHoverPic(":/newUI/dark/hover/close-hover_dark.svg");
        m_closeButton->setNormalPic(":/newUI/dark/normal/close-normal_dark.svg");
    }


    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setMargin(0);
    hLayout->setSpacing(2);

    if (m_deskinfo.waylandDectected()) {
        hLayout->addSpacing(10);
        m_mainTool->hide();
        hLayout->addWidget(m_subTool, 0, Qt::AlignCenter);
    } else {
        hLayout->addWidget(m_mainTool, 0,  Qt::AlignCenter);
        hLayout->addWidget(m_subTool, 0, Qt::AlignCenter);
    }

    hLayout->addWidget(m_closeButton, 0,  Qt::AlignCenter);
    setLayout(hLayout);

    connect(m_mainTool, &MainToolWidget::buttonChecked, this, &ToolBarWidget::setExpand);
    connect(m_closeButton, &DImageButton::clicked, this, &ToolBarWidget::closeButtonSignal);
    connect(m_subTool, &SubToolWidget::keyBoardButtonClicked, this, &ToolBarWidget::keyBoardCheckedSlot);
    connect(m_subTool, &SubToolWidget::mouseBoardButtonClicked, this, &ToolBarWidget::mouseCheckedSignalToToolBar);
    connect(m_subTool, &SubToolWidget::mouseShowButtonClicked, this, &ToolBarWidget::mouseShowCheckedSignalToToolBar);
    connect(m_subTool, SIGNAL(microphoneActionChecked(bool)), this, SIGNAL(microphoneActionCheckedSignal(bool)));
    connect(m_subTool, SIGNAL(systemAudioActionChecked(bool)), this, SIGNAL(systemAudioActionCheckedSignal(bool)));
    connect(m_subTool, SIGNAL(cameraActionChecked(bool)), this, SIGNAL(cameraActionCheckedSignal(bool)));
    connect(m_subTool, SIGNAL(gifActionChecked(bool)), this, SIGNAL(gifActionCheckedSignal(bool)));
    connect(m_subTool, SIGNAL(mp4ActionChecked(bool)), this, SIGNAL(mp4ActionCheckedSignal(bool)));
    connect(m_subTool, SIGNAL(videoFrameRateChanged(int)), this, SIGNAL(frameRateChangedSignal(int)));
    connect(m_subTool, SIGNAL(changeShotToolFunc(const QString &)), this, SIGNAL(shotToolChangedSignal(const QString &)));
}

void ToolBarWidget::paintEvent(QPaintEvent *e)
{
    DFloatingWidget::paintEvent(e);

//    QPainter painter(this);
//    painter.setPen(QColor(255, 255, 255, 76.5));
//    painter.setRenderHint(QPainter::Antialiasing);
//    painter.drawLine(QPointF(BTN_RADIUS, 0), QPointF(this->width() - 1, 0));
}

void ToolBarWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    QSettings settings(this);
    settings.beginGroup("common");
    bool expand = settings.value("expand_savelist", false).toBool();
    settings.endGroup();

    if (expand)
        QTimer::singleShot(0, this, [ = ] { setExpand(expand, "saveList"); });

    DFloatingWidget::showEvent(event);
}

void ToolBarWidget::hideSomeToolBtn()
{
    m_subTool->hideSomeToolBtn();
    m_mainTool->hide();
}

void ToolBarWidget::setScrollShotDisabled()
{
    m_subTool->setScrollShotDisabled();
}
/*
bool ToolBarWidget::isButtonChecked()
{
    return m_expanded;
}

void ToolBarWidget::specifiedSavePath()
{
    m_majToolbar->specificedSavePath();
}
*/
void ToolBarWidget::keyBoardCheckedSlot(bool checked)
{
    emit keyBoardCheckedSignal(checked);
}

void ToolBarWidget::changeArrowAndLineFromBar(int line)
{
    m_subTool->changeArrowAndLineFromSideBar(line);
}

void ToolBarWidget::setRecordButtonDisableFromMain()
{
    m_mainTool->setRecordButtonOut();
}

void ToolBarWidget::setRecordLaunchFromMain(bool recordLaunch)
{
//    qDebug() << "main record mode2";
    m_mainTool->setRecordLauchMode(recordLaunch);
    m_subTool->setRecordLaunchMode(recordLaunch);
}
/*
void ToolBarWidget::setIsZhaoxinPlatform(bool isZhaoxin)
{
    m_subTool->setIsZhaoxinPlatform(isZhaoxin);
}
*/
void ToolBarWidget::setVideoInitFromMain()
{
    m_subTool->setVideoButtonInitFromSub();
}

void ToolBarWidget::shapeClickedFromBar(QString shape)
{
    m_subTool->shapeClickedFromWidget(shape);
}

void ToolBarWidget::setMicroPhoneEnable(bool status)
{
    m_subTool->setMicroPhoneEnable(status);
}

void ToolBarWidget::setSystemAudioEnable(bool status)
{
    m_subTool->setSystemAudioEnable(status);
}

void ToolBarWidget::setCameraDeviceEnable(bool status)
{
    m_subTool->setCameraDeviceEnable(status);
}

void ToolBarWidget::setExpand(bool expand, QString shapeType)
{
    Q_UNUSED(expand);
//    m_subToolbar->switchContent(shapeType);
    m_subTool->switchContent(shapeType);
//    emit expandChanged(expand, shapeType);

//    if (expand) {
//        m_expanded = true;
//        setFixedSize(TOOLBAR_WIDGET_SIZE.width(),
//                                 TOOLBAR_WIDGET_SIZE.height()*2+1);
//        m_hSeparatorLine->show();
//        m_subToolbar->show();
//    }
    emit changeFunctionSignal(shapeType);
    update();
}

ToolBarWidget::~ToolBarWidget() {}


ToolBar::ToolBar(DWidget *parent)
    : DLabel(parent)
{
    m_toolbarWidget = nullptr;
}

void ToolBar::setExpand(bool expand, QString shapeType)
{
    Q_UNUSED(expand);
    emit buttonChecked(shapeType);
    update();
}

void ToolBar::paintEvent(QPaintEvent *e)
{
    DLabel::paintEvent(e);
}

void ToolBar::enterEvent(QEvent *e)
{
    qApp->setOverrideCursor(Qt::ArrowCursor);
    DLabel::enterEvent(e);
}

bool ToolBar::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ApplicationPaletteChange || event->type() == QEvent::PaletteChange) {
        qDebug() << obj;
        qDebug() << "--------------";
    }

    return DLabel::eventFilter(obj, event);
}

void ToolBar::hideSomeToolBtn()
{
    m_toolbarWidget->hideSomeToolBtn();
}

void ToolBar::setScrollShotDisabled()
{
    m_toolbarWidget->setScrollShotDisabled();
}

void ToolBar::showAt(QPoint pos)
{
    if (!isVisible())
        this->show();

    move(pos.x(), pos.y());
}
/*
void ToolBar::specificedSavePath()
{
    m_toolbarWidget->specifiedSavePath();
}
*/
void ToolBar::currentFunctionMode(QString shapeType)
{
    DPalette pa;
    pa = m_confirmButton->palette();
    if (shapeType == "shot") {
        pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
        pa.setColor(DPalette::Dark, QColor(0, 129, 255, 204));
        pa.setColor(DPalette::Light, QColor(0, 129, 255, 204));
        m_confirmButton->setPalette(pa);
        m_confirmButton->setIcon(QIcon(":/newUI/checked/screenshot-checked.svg"));
        Utils::setAccessibility(m_confirmButton, AC_MAINWINDOW_MAINSHOTBTN);
        m_confirmButton->setProperty("isShotState", true);

    } else if (shapeType == "record") {
        pa = m_confirmButton->palette();
        pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
        pa.setColor(DPalette::Dark, QColor(229, 70, 61, 204));
        pa.setColor(DPalette::Light, QColor(229, 70, 61, 204));
        m_confirmButton->setPalette(pa);
        m_confirmButton->setIcon(QIcon(":/newUI/checked/screencap-checked.svg"));
        Utils::setAccessibility(m_confirmButton, AC_MAINWINDOW_MAINRECORDBTN);
        m_confirmButton->setProperty("isShotState", false);

    }
    update();
    emit currentFunctionToMain(shapeType);
}

void ToolBar::keyBoardCheckedToMainSlot(bool checked)
{
    emit keyBoardCheckedToMain(checked);
}

void ToolBar::microphoneActionCheckedToMainSlot(bool checked)
{
    emit microphoneActionCheckedToMain(checked);
}
void ToolBar::systemAudioActionCheckedToMainSlot(bool checked)
{
    emit systemAudioActionCheckedToMain(checked);
}

void ToolBar::changeArrowAndLineFromMain(int line)
{
    m_toolbarWidget->changeArrowAndLineFromBar(line);
}

void ToolBar::initToolBar(MainWindow* pmainWindow)
{
//    setFixedSize(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);
    setFixedHeight(TOOLBAR_HEIGHT);
    m_toolbarWidget = new ToolBarWidget(pmainWindow,this);

    //构建截屏录屏功能触发
    m_confirmButton = new DPushButton(this);
    m_confirmButton->setFocusPolicy(Qt::NoFocus);
    m_confirmButton->setIconSize(QSize(38, 38));
    m_confirmButton->setFixedSize(76, 58);

    DPalette pa;
    pa = m_confirmButton->palette();
    pa.setColor(DPalette::ButtonText, QColor(28, 28, 28, 255));
    pa.setColor(DPalette::Dark, QColor(0, 129, 255, 204));
    pa.setColor(DPalette::Light, QColor(0, 129, 255, 204));
    m_confirmButton->setPalette(pa);
    m_confirmButton->setIcon(QIcon(":/newUI/checked/screenshot-checked.svg"));
    Utils::setAccessibility(m_confirmButton, AC_MAINWINDOW_MAINSHOTBTN);
    m_confirmButton->setProperty("isShotState", true);

    connect(m_confirmButton, &DPushButton::clicked, this, [ = ] {
        if(m_confirmButton->property("isShotState").toBool()) {
            pmainWindow->saveScreenShot();
        } else {
            pmainWindow->startCountdown();
        }
    });



    QHBoxLayout *vLayout = new QHBoxLayout();
    vLayout->setSizeConstraint(QLayout::SetFixedSize);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addStretch();
    vLayout->addWidget(m_toolbarWidget);
    vLayout->addStretch();
    vLayout->addWidget(m_confirmButton);
    setLayout(vLayout);
    update();

    connect(m_toolbarWidget, &ToolBarWidget::expandChanged, this, &ToolBar::setExpand);
    connect(m_toolbarWidget, &ToolBarWidget::saveImage, this, &ToolBar::requestSaveScreenshot);
    connect(m_toolbarWidget, &ToolBarWidget::closeButtonSignal, this, &ToolBar::closeButtonToMain);
    connect(m_toolbarWidget, &ToolBarWidget::colorChanged, this, &ToolBar::updateColor);
    connect(this, &ToolBar::shapePressed, m_toolbarWidget, &ToolBarWidget::shapePressed);
    connect(this, &ToolBar::saveBtnPressed, m_toolbarWidget, &ToolBarWidget::saveBtnPressed);
    connect(m_toolbarWidget, &ToolBarWidget::saveSpecifiedPath, this, &ToolBar::saveSpecifiedPath);
    connect(m_toolbarWidget, &ToolBarWidget::closed, this, &ToolBar::closed);

    connect(m_toolbarWidget, &ToolBarWidget::changeFunctionSignal, this, &ToolBar::currentFunctionMode);
    connect(m_toolbarWidget, &ToolBarWidget::keyBoardCheckedSignal, this, &ToolBar::keyBoardCheckedToMainSlot);
    connect(m_toolbarWidget, &ToolBarWidget::microphoneActionCheckedSignal, this, &ToolBar::microphoneActionCheckedToMainSlot);
    connect(m_toolbarWidget, &ToolBarWidget::systemAudioActionCheckedSignal, this, &ToolBar::systemAudioActionCheckedToMainSlot);
    connect(m_toolbarWidget, &ToolBarWidget::cameraActionCheckedSignal, this, &ToolBar::cameraActionCheckedToMain);
    connect(m_toolbarWidget, &ToolBarWidget::mouseCheckedSignalToToolBar, this, &ToolBar::mouseCheckedToMain);
    connect(m_toolbarWidget, &ToolBarWidget::mouseShowCheckedSignalToToolBar, this, &ToolBar::mouseShowCheckedToMain);
    connect(m_toolbarWidget, &ToolBarWidget::gifActionCheckedSignal, this, &ToolBar::gifActionCheckedToMain);
    connect(m_toolbarWidget, &ToolBarWidget::mp4ActionCheckedSignal, this, &ToolBar::mp4ActionCheckedToMain);
    connect(m_toolbarWidget, &ToolBarWidget::frameRateChangedSignal, this, &ToolBar::frameRateChangedToMain);
    connect(m_toolbarWidget, &ToolBarWidget::shotToolChangedSignal, this, &ToolBar::shotToolChangedToMain);
}

void ToolBar::setRecordButtonDisable()
{
    m_toolbarWidget->setRecordButtonDisableFromMain();
}

void ToolBar::setRecordLaunchMode(bool recordLaunch)
{
//    qDebug() << "main record mode1";
    m_toolbarWidget->setRecordLaunchFromMain(recordLaunch);
}
/*
void ToolBar::setIsZhaoxinPlatform(bool isZhaoxin)
{
    m_toolbarWidget->setIsZhaoxinPlatform(isZhaoxin);
}
*/
void ToolBar::setVideoButtonInit()
{
    m_toolbarWidget->setVideoInitFromMain();
}

void ToolBar::shapeClickedFromMain(QString shape)
{
    // 在工具栏显示之前，触发MainWindow::initShortcut()中的快捷键
    // R O L P T
    // 导致button没有初始化就执行click，导致崩溃。
    if(m_toolbarWidget){
        m_toolbarWidget->shapeClickedFromBar(shape);
    }
}

void ToolBar::setMicroPhoneEnable(bool status)
{
    m_toolbarWidget->setMicroPhoneEnable(status);
}

void ToolBar::setSystemAudioEnable(bool status)
{
    m_toolbarWidget->setSystemAudioEnable(status);
}

void ToolBar::setCameraDeviceEnable(bool status)
{
    m_toolbarWidget->setCameraDeviceEnable(status);
}
/*
bool ToolBar::isButtonChecked()
{
    return m_expanded;
}
*/
ToolBar::~ToolBar()
{
}

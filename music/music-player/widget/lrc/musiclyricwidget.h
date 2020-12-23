/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
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

#pragma once

#include <QHBoxLayout>

#include <DWidget>
#include <DLabel>
#include <DBlurEffectWidget>
#include <QImage>

#include <searchmeta.h>

DWIDGET_USE_NAMESPACE

class Cover;
class LyricLabel;
class MusicImageButton;
class MusicLyricWidget : public DWidget
{
    Q_OBJECT

public:
    explicit MusicLyricWidget(QWidget *parent = Q_NULLPTR);
    ~MusicLyricWidget() override;

    void updateUI();
    // 控件显示动画
    void showAnimation(const QSize &size);
    // 控件关闭动画
    void closeAnimation(const QSize &size);

signals:
    void toggleLyricView();
    void requestContextSearch(const QString &context);

public slots:
    void onProgressChanged(qint64 value, qint64 length);
    void onMusicPlayed();
    void onCoverChanged();

    void setThemeType(int type);

protected:
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    QHBoxLayout *m_leftLayout;
    Cover               *m_cover              = nullptr;
    LyricLabel          *m_lyricview            = nullptr;
    DLabel              *m_nolyric              = nullptr;
    QString              m_defaultCover = ":/common/image/cover_max.svg";
    bool                 m_serachflag = false;
    DBlurEffectWidget   *m_backgroundW = nullptr;
    QImage               m_defaultImage = QImage(":/common/image/cover_max.svg");
};

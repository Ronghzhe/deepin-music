/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "mainwindow.h"

#include <QDebug>
#include <QPainter>
#include <QVBoxLayout>
#include <QListWidgetItem>
#include <QStackedWidget>
#include <QTimer>
#include <QProcess>
#include <QResizeEvent>
#include <QHBoxLayout>
#include <QStackedLayout>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMenu>

#include <dutility.h>
#include <dthememanager.h>
#include <DAboutDialog>

#include "titlebar.h"
#include "footer.h"
#include "importwidget.h"
#include "lyricview.h"
#include "playlistwidget.h"
#include "playlistview.h"
#include "playlistitem.h"
#include "musiclistwidget.h"

#include "../core/music.h"
#include "../core/playlist.h"
#include "../musicapp.h"

#include "helper/widgethellper.h"

DWIDGET_USE_NAMESPACE

static const int s_AnimationDelay   = 350;
static QColor s_normalTitleTop      = QColor(255, 255, 255, 255);
static QColor s_normalTitleBottom   = QColor(0xf8, 0xf8, 0xf8, 255);
static QColor s_lyricTitleTop       = QColor(0, 0, 0, 94);
static QColor s_lyriclTitleBottom   = QColor(0, 0, 0, 102);

class PlayerFramePrivate
{
public:
    QStackedWidget  *stacked    = nullptr;
    QFrame          *content    = nullptr;
    TitleBar        *title      = nullptr;
    ImportWidget    *import     = nullptr;
    MusicListWidget *musicList  = nullptr;
    PlaylistWidget  *playlist   = nullptr;
    Footer          *footer     = nullptr;
    LyricView       *lyric      = nullptr;
};


PlayerFrame::PlayerFrame(QWidget *parent)
    : DWindow(parent), d(new PlayerFramePrivate)
{
    m_titlebarTopColor = s_normalTitleTop;
    m_titlebarBottomColor = s_normalTitleBottom;

    setFocusPolicy(Qt::ClickFocus);
    setObjectName("PlayerFrame");

    d->title = new TitleBar;
    d->title->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setTitlebarWidget(d->title);

    d->content = new QFrame;
    d->content->setObjectName("BackgroundWidget");
    d->content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto contentLayout = new QVBoxLayout(d->content);
    contentLayout->setMargin(0);
    contentLayout->setSpacing(0);

    d->import = new ImportWidget;
    d->musicList = new MusicListWidget;
    d->lyric = new LyricView;
    d->playlist = new PlaylistWidget(d->stacked);
    d->playlist->setFixedWidth(220);
    d->footer = new Footer;
    d->footer->setFixedHeight(60);

    d->stacked = new QStackedWidget;
    d->stacked->setObjectName("ContentWidget");

    d->stacked->addWidget(d->import);
    d->stacked->addWidget(d->musicList);
    d->stacked->addWidget(d->lyric);
    d->stacked->addWidget(d->playlist);

    d->lyric->hide();
    d->import->hide();
    d->playlist->hide();
    d->musicList->hide();

    contentLayout->addWidget(d->stacked);
    contentLayout->addWidget(d->footer);
    setContentWidget(d->content);
    D_THEME_INIT_WIDGET(PlayerFrame);

    resize(1040, 650);
    QImage image = QImage((":/image/cover_max.png"));
    image = WidgetHelper::cropRect(image, this->size());
    setBackgroundImage(WidgetHelper::blurImage(image, 50));

    d->footer->setFocus();
}

PlayerFrame::~PlayerFrame()
{

}

void PlayerFrame::initMusiclist(PlaylistPtr allmusic, PlaylistPtr last)
{
    if (allmusic.isNull() || 0 == allmusic->length()) {
        d->import->show();
        d->footer->enableControl(false);
        d->musicList->hide();
        qWarning() << "no music in all music list" << allmusic;
        d->musicList->setCurrentList(allmusic);
        return;
    }

    if (last.isNull() || 0 == last->length()) {
        qDebug() << "init music with empty playlist:" << last;
    }
    d->import->hide();
    d->musicList->raise();
    d->musicList->show();
    d->musicList->setCurrentList(last);
    d->musicList->onMusiclistChanged(last);
}

void PlayerFrame::initPlaylist(QList<PlaylistPtr > playlists, PlaylistPtr last)
{
    d->playlist->initPlaylist(playlists, last);
}

void PlayerFrame::initFooter(PlaylistPtr current, int mode)
{
    emit d->footer->initFooter(current, mode);
}

void PlayerFrame::binding(Presenter *presenter)
{

    connect(d->title, &TitleBar::searchText,
            presenter, &Presenter::onSearchText);
    connect(d->title, &TitleBar::locateMusic,
            presenter, &Presenter::onLocateMusic);

    // Music list binding
    connect(presenter, &Presenter::selectedPlaylistChanged,
            d->musicList, &MusicListWidget::onMusiclistChanged);
    connect(presenter, &Presenter::musicRemoved,
            d->musicList, &MusicListWidget::onMusicRemoved);

    connect(presenter, &Presenter::musicAdded,
            d->musicList, &MusicListWidget::onMusicAdded);
    connect(presenter, &Presenter::musiclistAdded,
            d->musicList, &MusicListWidget::onMusicListAdded);

    connect(presenter, &Presenter::musicPlayed,
            d->musicList, &MusicListWidget::onMusicPlayed);
    connect(presenter, &Presenter::musiclistMenuRequested,
            d->musicList, &MusicListWidget::onCustomContextMenuRequest);

    connect(d->musicList, &MusicListWidget::musicClicked,
            presenter, &Presenter::onMusicPlay);
    connect(d->musicList, &MusicListWidget::musicAdd,
            presenter, &Presenter::onMusicAdd);
    connect(d->musicList, &MusicListWidget::musicListRemove,
            presenter, &Presenter::onMusicRemove);
    connect(d->musicList, &MusicListWidget::musicListDelete,
            presenter, &Presenter::onMusicDelete);

    connect(d->musicList, &MusicListWidget::playall,
            presenter, &Presenter::onPlayall);
    connect(d->musicList, &MusicListWidget::requestCustomContextMenu,
            presenter, &Presenter::onRequestMusiclistMenu);
    connect(d->musicList, &MusicListWidget::resort,
            presenter, &Presenter::onResort);

    // Play list bindding
    connect(presenter, &Presenter::selectedPlaylistChanged,
            d->playlist, &PlaylistWidget::onCurrentChanged);
    connect(presenter, &Presenter::playlistAdded,
            d->playlist, &PlaylistWidget::onPlaylistAdded);

    connect(d->playlist, &PlaylistWidget::playall,
            presenter, &Presenter::onPlayall);
    connect(d->playlist, &PlaylistWidget::addPlaylist,
            presenter, &Presenter::onPlaylistAdd);
    connect(d->playlist, &PlaylistWidget::selectPlaylist,
            presenter, &Presenter::onSelectedPlaylistChanged);

    // Lyric
    connect(presenter, &Presenter::musicPlayed,
            d->lyric, &LyricView::onMusicPlayed);
        connect(presenter, &Presenter::lyricSearchFinished,
                d->lyric, &LyricView::onLyricChanged);
    connect(presenter, &Presenter::coverSearchFinished,
            d->lyric, &LyricView::onCoverChanged);
    connect(presenter, &Presenter::progrossChanged,
            d->lyric, &LyricView::onProgressChanged);
    connect(presenter, &Presenter::progrossChanged,
            d->lyric, &LyricView::onProgressChanged);
    connect(presenter, &Presenter::coverSearchFinished,
    this, [ = ](const MusicMeta &, const QByteArray & coverData) {
        QImage image = QImage::fromData(coverData);
        if (image.isNull()) {
            return;
        }

        image = WidgetHelper::cropRect(image, this->size());
        setBackgroundImage(WidgetHelper::blurImage(image, 50));
        this->repaint();
    });
    connect(d->lyric, &LyricView::hideLyricView,
            d->footer, &Footer::toggleLyric);

    // Footer Control
    connect(presenter, &Presenter::coverSearchFinished,
            d->footer, &Footer::onCoverChanged);
    connect(presenter, &Presenter::musicPlayed,
            d->footer, &Footer::onMusicPlayed);
    connect(presenter, &Presenter::musicPaused,
            d->footer, &Footer::onMusicPause);
    connect(presenter, &Presenter::musicStoped,
            d->footer, &Footer::onMusicStoped);
    connect(presenter, &Presenter::musicAdded,
            d->footer, &Footer::onMusicAdded);
    connect(presenter, &Presenter::musiclistAdded,
            d->footer, &Footer::onMusicListAdded);
    connect(presenter, &Presenter::musicRemoved,
            d->footer, &Footer::onMusicRemoved);
    connect(presenter, &Presenter::progrossChanged,
            d->footer, &Footer::onProgressChanged);

    connect(d->footer, &Footer::play,
            presenter, &Presenter::onMusicPlay);
    connect(d->footer, &Footer::resume,
            presenter, &Presenter::onMusicResume);
    connect(d->footer, &Footer::pause,
            presenter, &Presenter::onMusicPause);
    connect(d->footer, &Footer::prev,
            presenter, &Presenter::onMusicPrev);
    connect(d->footer, &Footer::next,
            presenter, &Presenter::onMusicNext);
    connect(d->footer, &Footer::changeProgress,
            presenter, &Presenter::onChangeProgress);
    connect(d->footer, &Footer::toggleFavourite,
            presenter, &Presenter::onToggleFavourite);
    connect(d->footer, &Footer::modeChanged,
            presenter, &Presenter::onPlayModeChanged);

    connect(d->footer, &Footer::locate,
    this, [ = ](PlaylistPtr playlist, const MusicMeta & info) {
        d->musicList->onLocate(playlist, info);
        d->playlist->onCurrentChanged(playlist);
        if (d->lyric->isVisible()) {
            d->footer->toggleLyric();
        }
    });

    // Import bindding
    connect(d->import, &ImportWidget::importMusicDirectory,
            presenter, &Presenter::onImportMusicDirectory);
    connect(d->import, &ImportWidget::importFiles,
            this, &PlayerFrame::onSelectImportFiles);
    connect(this, &PlayerFrame::importSelectFiles,
            presenter, &Presenter::onImportFiles);

    // View control
    connect(presenter, &Presenter::selectedPlaylistChanged,
    this, [ = ]() {
        if (d->lyric->isVisible()) {
            WidgetHelper::slideTop2BottomWidget(d->lyric, d->musicList, s_AnimationDelay);
            this->disableControl();
            d->musicList->resize(d->lyric->size());
            d->musicList->show();
        }
    });

    connect(presenter, &Presenter::requestImportFiles,
            this, &PlayerFrame::onSelectImportFiles);

    connect(d->footer, &Footer::toggleLyric,
    this, [ = ]() {
        if (d->lyric->isVisible()) {
            // change to optical
            m_titlebarTopColor = s_normalTitleTop;
            m_titlebarBottomColor = s_normalTitleBottom;

            WidgetHelper::slideTop2BottomWidget(
                d->lyric, d->musicList, s_AnimationDelay);
            d->musicList->resize(d->lyric->size());
            d->musicList->raise();
            d->musicList->show();
            this->repaint();
        } else {
            m_titlebarTopColor = s_lyricTitleTop;
            m_titlebarBottomColor = s_lyriclTitleBottom;

            WidgetHelper::slideBottom2TopWidget(
                d->musicList, d->lyric, s_AnimationDelay);
            d->lyric->resize(d->musicList->size());
            d->lyric->raise();
            d->lyric->show();
            this->repaint();
        }
        this->disableControl();

        // hide playlist
        if (d->playlist->isVisible())  {
            emit d->footer->togglePlaylist();
        }
    });

    connect(d->footer, &Footer::togglePlaylist,
    this, [ = ]() {
        d->playlist->resize(d->playlist->width(), d->stacked->height() - d->footer->height());
        QRect start(this->width(), 0, d->playlist->width(), d->playlist->height());
        QRect end(this->width() - d->playlist->width(), 0, d->playlist->width(), d->playlist->height());
        d->playlist->raise();
        if (d->playlist->isVisible()) {
            WidgetHelper::slideEdgeWidget(d->playlist, end, start, s_AnimationDelay, true);
        } else {
            WidgetHelper::slideEdgeWidget(d->playlist, start, end, s_AnimationDelay);
        }
        this->disableControl();
        d->playlist->setFocus();
    });

    connect(presenter, &Presenter::setPlaylistVisible,
    this, [ = ](bool visible) {
        // show playlist
        if (!d->playlist->isVisible() && visible)  {
            emit d->footer->togglePlaylist();
        }
        if (d->playlist->isVisible() && !visible)  {
            emit d->footer->togglePlaylist();
        }
    });

    connect(presenter, &Presenter::showMusiclist,
    this, [ = ]() {
        if (d->import->isVisible()) {
            WidgetHelper::slideRight2LeftWidget(d->import, d->musicList, s_AnimationDelay);
            this->disableControl();
        }
        d->musicList->resize(d->import->size());
        d->musicList->show();
    });

    connect(presenter, &Presenter::metaInfoClean,
    this, [ = ]() {
        if (d->musicList->isVisible()) {
            if (d->playlist->isVisible())  {
                emit d->footer->togglePlaylist();
            }

            d->import->setFixedSize(d->musicList->size());
            WidgetHelper::slideRight2LeftWidget(d->musicList, d->import, s_AnimationDelay);
            this->disableControl();
        }
        d->import->show();
    });

    initMenu();
}

void PlayerFrame::mousePressEvent(QMouseEvent *event)
{
    qDebug() << event;
    // TODO hide all
    if (d->playlist->isVisible()) {
        emit d->footer->togglePlaylist();
    }

    DWindow::mousePressEvent(event);
}

void PlayerFrame::resizeEvent(QResizeEvent *e)
{
    DWindow::resizeEvent(e);
    QSize newSize = DWindow::size();
    d->stacked->setFixedSize(newSize.width(), newSize.height() - titlebarHeight());
    d->musicList->setFixedSize(newSize.width(),
                               newSize.height() - titlebarHeight() - d->footer->height() - 1);
    d->lyric->setFixedSize(newSize.width(),
                           newSize.height() - titlebarHeight() - d->footer->height() - 1);
    d->title->setFixedSize(newSize.width(), titlebarHeight() - 2);

    if (d->playlist->isVisible()) {
        d->playlist->resize(d->playlist->width(), d->stacked->height() - d->footer->height());
        QRect start(this->width(), 0, d->playlist->width(), d->playlist->height());
        QRect end(this->width() - d->playlist->width(), 0, d->playlist->width(), d->playlist->height());
        d->playlist->setGeometry(end);
    }
}

void PlayerFrame::paintEvent(QPaintEvent *e)
{
    {
        int radius = this->radius() ;
        int windowExtern = 40 + 1 * 2;

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);

        QRect windowRect = QWidget::rect().marginsRemoved(
                               QMargins(windowExtern, windowExtern - 10, windowExtern, windowExtern + 10));

        QPoint topLeft(windowRect.x(), windowRect.y());
        QPoint bottomRight(windowRect.x() + windowRect.width(), windowRect.y() + windowRect.height());
        QPainterPath border;
        border.addRoundedRect(windowRect, radius, radius);

        QLinearGradient linearGradient(topLeft, QPoint(topLeft.x(), bottomRight.y()));
        linearGradient.setColorAt(0.0, Qt::white);
        linearGradient.setColorAt(0.2, Qt::white);
        linearGradient.setColorAt(1.0, Qt::white);

        QPen borderPen(QColor(0, 0, 0, 38));

        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QBrush(linearGradient));
        painter.strokePath(border, borderPen);
        painter.fillPath(border, QBrush(linearGradient));
    }

    DWindow::paintEvent(e);

    // draw header
    {
        int radius = 3 ;
        int windowExtern = 40 + 1 * 2;

        QPainter titlePainter(this);
        titlePainter.setRenderHint(QPainter::Antialiasing);
        titlePainter.setRenderHint(QPainter::HighQualityAntialiasing);

        QRect winRect = QWidget::rect().marginsRemoved(
                            QMargins(windowExtern, windowExtern - 10  , windowExtern, windowExtern + 10));

        QPoint titleTopLeft(winRect.x(), winRect.y());

        QRect topLeftRect(titleTopLeft,
                          QPoint(winRect.x() + 2 * radius, winRect.y() + 2 * radius));
        QRect topRightRect(QPoint(winRect.x() + winRect.width(), winRect.y()),
                           QPoint(winRect.x() + winRect.width() - 2 * radius,
                                  winRect.y() + 2 * radius));

        QPainterPath titleBorder;
        titleBorder.moveTo(winRect.x() + radius, winRect.y());
        titleBorder.lineTo(winRect.x() + winRect.width() - radius, winRect.y());
        titleBorder.arcTo(topRightRect, 90.0, 90.0);
        titleBorder.lineTo(winRect.x() + winRect.width(), winRect.y() + radius);
        titleBorder.lineTo(winRect.x() + winRect.width(), winRect.y() + 39);
        titleBorder.lineTo(winRect.x(), winRect.y() + 39);
        titleBorder.lineTo(winRect.x() , winRect.y() + radius);
        titleBorder.arcTo(topLeftRect, 90.0, 90.0);
        titleBorder.closeSubpath();

        QLinearGradient linearGradient(QPointF(0.0, 0.0), QPointF(0.0, 1.0));
        linearGradient.setColorAt(0.0, m_titlebarTopColor);
        linearGradient.setColorAt(1.0, m_titlebarBottomColor);

        QPen borderPen(QColor(255, 255, 255, 25));

        titlePainter.setBrush(QBrush(linearGradient));
        titlePainter.fillPath(titleBorder, QBrush(linearGradient));
        titlePainter.strokePath(titleBorder, borderPen);
        QLine line(titleTopLeft.x(), winRect.y() + 39,
                   winRect.x() + winRect.width(), winRect.y() + 39);

        titlePainter.setPen(QPen(QColor(0, 0, 0, 255 * 0.3), 0.5));
        titlePainter.drawLine(line);
    }
}

void PlayerFrame::onSelectImportFiles()
{
    QFileDialog fileDlg;
    auto musicDir =  QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    fileDlg.setDirectory(musicDir.first());

    fileDlg.setViewMode(QFileDialog::Detail);
    fileDlg.setFileMode(QFileDialog::Directory);
    if (QFileDialog::Accepted == fileDlg.exec()) {
        emit importSelectFiles(fileDlg.selectedFiles());
    }
}

void PlayerFrame::initMenu()
{
    auto m_newlist = new QAction(tr("New songlist"), this);
    connect(m_newlist, &QAction::triggered, this, [ = ](bool) {
        if (!d->playlist->isVisible()) {
            emit d->footer->togglePlaylist();
        }
        emit d->playlist->addPlaylist(true);
    });

    auto m_addmusic = new QAction(tr("Add music"), this);
    connect(m_addmusic, &QAction::triggered, this, [ = ](bool) {

        if (d->lyric->isVisible()) {
            WidgetHelper::slideTop2BottomWidget(d->lyric, d->musicList, s_AnimationDelay);
            this->disableControl();
            d->musicList->resize(d->lyric->size());
            d->musicList->show();
            if (!d->playlist->isVisible()) {
                emit d->footer->togglePlaylist();
            }
        }
        this->onSelectImportFiles();
    });

    auto m_settings = new QAction(tr("Settings"), this);
    connect(m_settings, &QAction::triggered, this, [ = ](bool) {

    });

    auto m_colorMode = new QAction(tr("Deep color mode"), this);
    connect(m_colorMode, &QAction::triggered, this, [ = ](bool) {

    });

    auto m_about = new QAction(tr("About"), this);
    connect(m_about, &QAction::triggered, this, [ = ](bool) {
        QString descriptionText = tr("Deepin Music Player is a beautiful design and "
                                     "simple function local music player. "
                                     "It supports viewing lyrics when playing, "
                                     "playing lossless music and creating customizable songlist, etc.");
        QString acknowledgementLink = "https://www.deepin.org/acknowledgments/deepin-music#thanks";

        auto *aboutDlg = new DAboutDialog(this);
        aboutDlg->setWindowIcon(QPixmap(":/image/deepin-music.svg"));
        aboutDlg->setProductIcon(QPixmap(":/image/about_icon.png"));
        aboutDlg->setProductName("Deepin Music");
        aboutDlg->setVersion(tr("Version: 3.0"));
        aboutDlg->setDescription(descriptionText);
        aboutDlg->setAcknowledgementLink(acknowledgementLink);
        aboutDlg->show();
    });

    QAction *m_help = new QAction(tr("Help"), this);
    connect(m_help, &QAction::triggered,
    this, [ = ](bool) {
        static QProcess *m_manual = nullptr;
        if (NULL == m_manual) {
            m_manual =  new QProcess(this);
            const QString pro = "dman";
            const QStringList args("dde");
            connect(m_manual, static_cast<void(QProcess::*)(int)>(&QProcess::finished), this, [ = ](int) {
                m_manual->deleteLater();
                m_manual = nullptr;
            });
            m_manual->start(pro, args);
        }
    });

    QAction *m_close = new QAction(tr("Exit"), this);
    connect(m_close, &QAction::triggered, this, [ = ](bool) {
        this->close();
    });

    titleBarMenu()->addAction(m_newlist);
    titleBarMenu()->addAction(m_addmusic);
    titleBarMenu()->addSeparator();

    titleBarMenu()->addAction(m_colorMode);
    titleBarMenu()->addAction(m_settings);
    titleBarMenu()->addSeparator();

    titleBarMenu()->addAction(m_about);
    titleBarMenu()->addAction(m_help);
    titleBarMenu()->addAction(m_close);
}

void PlayerFrame::disableControl()
{
    d->footer->enableControl(false);
    QTimer::singleShot(s_AnimationDelay, this, [ = ]() {
        d->footer->enableControl(true);
    });
}
import QtQuick 2.4

Item {

    property var songsView

    function playMusicByUrl(url) {
        MusicManageWorker.playSong(url);
        MediaPlayer.playLocalMedia(url);
    }

    Connections {
        target: songsView

        onPlayMusicByUrl: {
            playMusicByUrl(url);
        }
    }

}
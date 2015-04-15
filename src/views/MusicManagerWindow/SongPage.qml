import QtQuick 2.4
import DMusic 1.0

Rectangle {
    id: root

    SongListView {
        id: songListView
        datamodel: SongListModel
    }

    SongController {
        songsView: songListView.view    
    }
}
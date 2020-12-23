/*
 * Copyright (C) 2020 chengdu Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <zhaoyongming@uniontech.com>
 *
 * Maintainer: Iceyer <zhaoyongming@uniontech.com>
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
#include "musicsearchlistmodel.h"
#include <QDebug>

MusicSearchListModel::MusicSearchListModel(int rows, int columns, QObject *parent):
    QStandardItemModel(rows, columns, parent)
{

}

MusicSearchListModel::~MusicSearchListModel()
{

}

bool MusicSearchListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QStandardItemModel::setData(index, value, role);
}

//Qt::DropActions MusicSearchListModel::supportedDropActions() const
//{
//    return Qt::CopyAction | Qt::MoveAction;
//}

Qt::ItemFlags MusicSearchListModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (index.isValid()) {
        return Qt::ItemIsDragEnabled | defaultFlags;
    } else {
        return Qt::ItemIsDropEnabled | defaultFlags;
    }
}

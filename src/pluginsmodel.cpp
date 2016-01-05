/***************************************************************************
                          pluginsmodel.cpp  -  description
                             -------------------
    copyright            : (C) 2016 by Pino Toscano
    email                : toscano.pino@tiscali.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "pluginsmodel.h"
#include "pluginbase.h"

#include <klocalizedstring.h>

#include <QStringList>

namespace {

enum NodeType { NTRoot, NTClass, NTInstance };
class InstanceNode;

class BaseNode
{
public:
    virtual ~BaseNode() { qDeleteAll(children); }

    BaseNode *parent;
    QList<BaseNode *> children;
    NodeType type : 3;

protected:
    BaseNode(NodeType t, BaseNode *p) : parent(p), type(t) {
      if (parent) { parent->children.append(this); }
    }
};

class RootNode : public BaseNode
{
public:
    RootNode() : BaseNode(NTRoot, 0) {}
};

class ClassNode : public BaseNode
{
public:
    ClassNode(RootNode *parent) : BaseNode(NTClass, parent) {}

    InstanceNode* findInstance(const QString &instanceName, int *index) const;

    QString className;
    QString classDescription;
};

class InstanceNode : public BaseNode
{
public:
    InstanceNode(ClassNode *parent) : BaseNode(NTInstance, parent) {}

    PluginBase *plugin;
};

InstanceNode* ClassNode::findInstance(const QString &instanceName, int *index = 0) const
{
    const int childCount = children.count();
    for (int i = 0; i < childCount; ++i) {
        InstanceNode *in = static_cast<InstanceNode *>(children.at(i));
        if (instanceName == in->plugin->name()) {
            if (index) {
                *index = i;
            }
            return in;
        }
    }
    if (index) {
        *index = -1;
    }
    return 0;
}

}

struct PluginsModel::Private
{
    Private(PluginsModel *qq) : q(qq) {}

    QModelIndex indexForNode(BaseNode *node) const;
    ClassNode* find(const QString &className) const;
    void clear();

    PluginsModel *q;
    RootNode root;
};

QModelIndex PluginsModel::Private::indexForNode(BaseNode *node) const
{
    if (node->parent) {
        const int id = node->parent->children.indexOf(node);
        if (id >= 0 && id < node->parent->children.count()) {
           return q->createIndex(id, 0, node);
        }
    }
    return QModelIndex();
}

ClassNode* PluginsModel::Private::find(const QString &className) const
{
    foreach (BaseNode *n, root.children) {
        ClassNode *cn = static_cast<ClassNode *>(n);
        if (className == cn->className) {
            return cn;
        }
    }
    return 0;
}

void PluginsModel::Private::clear()
{
    qDeleteAll(root.children);
    root.children.clear();
}

PluginsModel::PluginsModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(new Private(this))
{
}

PluginsModel::~PluginsModel()
{
    delete d;
}

int PluginsModel::columnCount(const QModelIndex &parent) const
{
    BaseNode *node = parent.isValid() ? reinterpret_cast<BaseNode *>(parent.internalPointer()) : &d->root;
    return !node->children.isEmpty() ? 1 : 0;
}

QVariant PluginsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    BaseNode *node = reinterpret_cast<BaseNode *>(index.internalPointer());

    if (node->type == NTClass) {
        ClassNode *cn = static_cast<ClassNode *>(node);
        int childCount;
        switch (role) {
        case Qt::DisplayRole:
            childCount = cn->children.count();
            if (childCount == 0) {
                return i18n("<b>%1</b> (no instances)<br />%2",
                            cn->className, cn->classDescription);
            } else {
                return i18np("<b>%2</b> (%1 instance)<br />%3", "<b>%2</b> (%1 instances)<br />%3",
                             childCount, cn->className, cn->classDescription);
            }
        case ClassNameRole:
            return cn->className;
        }
    } else if (node->type == NTInstance) {
        InstanceNode *in = static_cast<InstanceNode *>(node);
        switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case InstanceNameRole:
            return in->plugin->name();
        case ClassNameRole:
            return static_cast<ClassNode *>(in->parent)->className;
        }
    }
    return QVariant();
}

Qt::ItemFlags PluginsModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags baseFlags = QAbstractItemModel::flags(index);
    if (index.isValid()) {
        BaseNode *node = reinterpret_cast<BaseNode *>(index.internalPointer());
        if (node->type == NTInstance) {
            baseFlags |= Qt::ItemIsEditable;
        }
    }
    return baseFlags;
}

QModelIndex PluginsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column != 0) {
        return QModelIndex();
    }

    BaseNode *node = parent.isValid() ? reinterpret_cast<BaseNode *>(parent.internalPointer()) : &d->root;
    if (row < node->children.count()) {
        return createIndex(row, column, node->children.at(row));
    }

    return QModelIndex();
}

QModelIndex PluginsModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    BaseNode *node = reinterpret_cast<BaseNode *>(index.internalPointer());
    return d->indexForNode(node->parent);
}

int PluginsModel::rowCount(const QModelIndex &parent) const
{
    BaseNode *node = parent.isValid() ? reinterpret_cast<BaseNode *>(parent.internalPointer()) : &d->root;
    return node->children.count();
}

bool PluginsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    BaseNode *node = reinterpret_cast<BaseNode *>(index.internalPointer());

    if (node->type == NTInstance) {
        InstanceNode *in = static_cast<InstanceNode *>(node);
        switch (role) {
        case Qt::EditRole:
            in->plugin->setName(value.toString());
            emit dataChanged(index, index);
            return true;
        }
    }
    return false;
}

void PluginsModel::setPluginClasses(const QMap<QString, PluginClassInfo> &classes)
{
    beginResetModel();
    d->clear();
    const QMap<QString, PluginClassInfo>::const_iterator end_cls = classes.end();
    for (QMap<QString, PluginClassInfo>::const_iterator it = classes.begin(); it != end_cls; ++it) {
        ClassNode *cn = new ClassNode(&d->root);
        cn->className = it.key();
        cn->classDescription = (*it).description;
    }
    endResetModel();
}

QModelIndex PluginsModel::addPlugin(PluginBase *plugin)
{
    ClassNode *cn = d->find(plugin->pluginClassName());
    if (!cn) {
        return QModelIndex();
    }

    const QModelIndex cnIndex = d->indexForNode(cn);
    const int childCount = cn->children.count();
    beginInsertRows(cnIndex, childCount, childCount);
    InstanceNode *in = new InstanceNode(cn);
    in->plugin = plugin;
    endInsertRows();
    return d->indexForNode(in);
}

void PluginsModel::removePlugin(PluginBase *plugin)
{
    ClassNode *cn = d->find(plugin->pluginClassName());
    if (!cn) {
        return;
    }

    int index;
    InstanceNode* in = cn->findInstance(plugin->name(), &index);
    if (!in) {
        return;
    }

    const QModelIndex cnIndex = d->indexForNode(cn);
    beginRemoveRows(cnIndex, index, index);
    delete cn->children.takeAt(index);
    endRemoveRows();
    // the parent has a text with the instance count, update it
    emit dataChanged(cnIndex, cnIndex);
}

void PluginsModel::notifyPluginRenamed(PluginBase *plugin)
{
    ClassNode *cn = d->find(plugin->pluginClassName());
    if (!cn) {
        return;
    }

    InstanceNode* in = cn->findInstance(plugin->name());
    if (!in) {
        return;
    }

    const QModelIndex inIndex = d->indexForNode(in);
    emit dataChanged(inIndex, inIndex);
}

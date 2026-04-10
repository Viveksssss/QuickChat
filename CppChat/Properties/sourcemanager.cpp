#include "sourcemanager.h"
#include <QDir>
#include <QString>

SourceManager::SourceManager() {}

QPixmap SourceManager::getPixmap(const QString &path) const
{
    auto it = pixCache.find(path);
    if (it != pixCache.end())
        return it.value();

    QString rawPath = path;
    if (path.endsWith("_rounded"))
        rawPath = path.left(path.size() - 8);

    QPixmap rawPix(rawPath);
    if (rawPix.isNull())
        return QPixmap();

    QPixmap rounded = rawPix.scaled(320, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 一次性缓存两张
    pixCache.insert(rawPath, rawPix);
    pixCache.insert(rawPath + "_rounded", rounded);

    return path.endsWith("_rounded") ? rounded : rawPix;
}

void SourceManager::addPixmap(const QString &path)
{
    auto it = pixCache.find(path);
    if (it != pixCache.end())
    {
        return;
    }

    QPixmap rawPix(path);
    if (rawPix.isNull())
        return;

    pixCache.insert(path, rawPix);
}

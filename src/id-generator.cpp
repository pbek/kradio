
#include <time.h>
#include <QString>
#include <QFile>

static QString dev_urandom = "/dev/urandom";

QString generateRandomID(size_t length)
{
    QString stime, srandom;
    stime.setNum(time(NULL));

    size_t min_length = stime.length() + 2;

    if (length < min_length) {
        length = min_length;
    }
    size_t bytes = (length - stime.length() - 1) / 2 + 1;

    QFile file (dev_urandom);
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.read(bytes);
    file.close();

    for (size_t i = 0; i < bytes; ++i)
        srandom += QString().sprintf("%02X", (unsigned char)data.at(i));

    return stime + srandom;
}


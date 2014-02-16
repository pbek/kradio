#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <QtCore/QString>
#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <klocale.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <QtCore/QRegExp>
#include <time.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdio.h>

#define dev_urandom  "/dev/urandom"

QString createStationID()
{
    const int buffersize = 32;
    unsigned char buffer[buffersize];

    QString stime, srandom = "";
    stime.setNum(time(NULL));

    int fd = open (dev_urandom, O_RDONLY);
    int err = read(fd, buffer, buffersize);
    if (err != 0) {
        fprintf(stderr, "cannot read from %s: %s\n", dev_urandom, strerror(err));
        exit(-1);
    }
    close(fd);
    for (int i = 0; i < buffersize; ++i)
        srandom += QString().sprintf("%02X", (unsigned int)buffer[i]);

//    kDebug() << i18n("generated StationID: ") << stime << srandom << endl;

    return stime + srandom;
}


        #define stationIDElement  "stationID"


bool convertFile(const QString &file)
{
    ////////////////////////////////////////////////////////////////////////
    // read input
    ////////////////////////////////////////////////////////////////////////

    QFile presetFile (file);

    if (! presetFile.open(IO_ReadOnly)) {
        kDebug() << "convertFile: error opening preset file" << file << "for reading";
        return false;
    }

    QString xmlData;

    // make sure that qtextstream is gone when we close presetFile
    {
        QTextStream ins(&presetFile);
//         ins.setEncoding(QTextStream::Locale);
        xmlData = ins.readAll();
    }

    if (xmlData.indexOf("<format>", 0, Qt::CaseInsensitive) >= 0) {
        kDebug() << "file " << file << " already in new format" << endl;
        // but add <?xml line at beginning if missing

        {
            presetFile.reset();
            QTextStream ins(&presetFile);
            ins.setCodec("UTF-8");
//             ins.setEncoding(QTextStream::UnicodeUTF8);
            xmlData = ins.readAll();
        }

        if (xmlData.indexOf("<?xml", 0, Qt::CaseInsensitive) < 0) {
            xmlData = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + xmlData;
        }

        int p = 0;
        int f = 0;
        while ( (f = xmlData.indexOf("<" stationIDElement "></" stationIDElement ">", p, Qt::CaseInsensitive) ) >= 0) {
            xmlData.insert(f + 2 + QString(stationIDElement).length(), createStationID());
        }

    } else {

        ////////////////////////////////////////////////////////////////////////
        // convert file
        ////////////////////////////////////////////////////////////////////////

        QRegExp qselect("<quickselect>.*</quickselect>");
        QRegExp docking("<dockingmenu>.*</dockingmenu>");
        QRegExp station("<station>(.*)</station>");
        QRegExp stationlist("<stationlist>");
        QRegExp emptyLines("\\n\\s*\\n");


        qselect.setMinimal(true);
        docking.setMinimal(true);
        station.setMinimal(true);

        xmlData = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + xmlData;
        xmlData.replace(stationlist, "<stationlist>\n\t\t<format>kradio-1.0</format>");
        xmlData.replace(qselect, "");
        xmlData.replace(docking, "");
        xmlData.replace(station, "<FrequencyRadioStation>\n"
                             "\t\t\t<" stationIDElement "></" stationIDElement ">"
                             "\\1</FrequencyRadioStation>"
                    );

        int p = 0;
        int f = 0;
        while ( (f = xmlData.indexOf("<" stationIDElement "></" stationIDElement ">", p, Qt::CaseInsensitive) ) >= 0) {
            xmlData.insert(f + 2 + QString(stationIDElement).length(), createStationID());
        }

        xmlData.replace(emptyLines, "\n");
    }

    presetFile.close();


    ////////////////////////////////////////////////////////////////////////
    // write output
    ////////////////////////////////////////////////////////////////////////

    if (! presetFile.open(IO_WriteOnly)) {
        kDebug() << "convertFile: error opening preset file" << file << "for writing";
       return false;
    }

    QTextStream outs(&presetFile);
    outs.setCodec("UTF-8");
//     outs.setEncoding(QTextStream::UnicodeUTF8);

    outs << xmlData;

    if (presetFile.error() != QFile::NoError) {
        kDebug() << "StationList::writeXML: error writing preset file" << file
                 << "(" << presetFile.error() << ")";
        return false;
    }

    return true;
}


static const char *description = "convert-presets";

int main(int argc, char *argv[])
{
    KAboutData aboutData("kradio4-convert-presets", "kradio4-convert-presets", ki18n("kradio4-convert-presets"),
                         KRADIO_VERSION, ki18n(description), KAboutData::License_GPL,
                         ki18n("(c) 2003-2010 Martin Witte"),
                         KLocalizedString(),
                         "http://sourceforge.net/projects/kradio");
    aboutData.addAuthor(ki18n("Martin Witte"),  KLocalizedString(), "emw-kradio@nocabal.de");

    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add("q", ki18n("quiet mode - no informational output"), 0);
    options.add("+[preset files]", ki18n("preset files to convert"));
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication a (false);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    for (int i = 0; i < args->count(); ++i) {
        const QString x = args->arg(i);
        if (! convertFile(x)) {
            return -1;
        } else {
            if (! args->isSet("q"))
                kDebug() << x << ": ok" << endl;
        }
    }
    if (args->count() == 0) {
        kDebug() << "no input" << endl;
        return -1;
    }

    return 0;
}

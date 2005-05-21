#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <qstring.h>
#include <qtextstream.h>
#include <qfile.h>
#include <klocale.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <qregexp.h>
#include <time.h>
#include <sys/fcntl.h>
#include <unistd.h>

#define dev_urandom  "/dev/urandom"

QString createStationID()
{
    const int buffersize = 32;
    unsigned char buffer[buffersize];

    QString stime, srandom = "";
    stime.setNum(time(NULL));

    int fd = open (dev_urandom, O_RDONLY);
    read(fd, buffer, buffersize);
    close(fd);
    for (int i = 0; i < buffersize; ++i)
        srandom += QString().sprintf("%02X", (unsigned int)buffer[i]);

//    kdDebug() << i18n("generated StationID: ") << stime << srandom << endl;

    return stime + srandom;
}




bool convertFile(const QString &file)
{
    ////////////////////////////////////////////////////////////////////////
    // read input
    ////////////////////////////////////////////////////////////////////////

    QFile presetFile (file);

    if (! presetFile.open(IO_ReadOnly)) {
        kdDebug() << "convertFile: "
                  << i18n("error opening preset file")
                  << " " << file << " "
                  << i18n("for reading") << endl;
        return false;
    }

    QString xmlData;

    // make sure that qtextstream is gone when we close presetFile
    {
        QTextStream ins(&presetFile);
        ins.setEncoding(QTextStream::Locale);
        xmlData = ins.read();
    }

    if (xmlData.find("<format>", 0, false) >= 0) {
        kdDebug() << "file " << file << " already in new format" << endl;
        // but add <?xml line at beginning if missing

        {
            presetFile.reset();
            QTextStream ins(&presetFile);
            ins.setEncoding(QTextStream::UnicodeUTF8);
            xmlData = ins.read();
        }

        if (xmlData.find("<?xml", 0, false) < 0) {
            xmlData = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + xmlData;
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

        #define stationIDElement  "stationID"

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
        while ( (f = xmlData.find("<" stationIDElement "></" stationIDElement ">", p) ) >= 0) {
            xmlData.insert(f + 2 + QString(stationIDElement).length(), createStationID());
        }

        xmlData.replace(emptyLines, "\n");
    }

    presetFile.close();


    ////////////////////////////////////////////////////////////////////////
    // write output
    ////////////////////////////////////////////////////////////////////////

    if (! presetFile.open(IO_WriteOnly)) {
        kdDebug() << "convertFile: "
                  << i18n("error opening preset file")
                  << " " << file << " "
                  << i18n("for writing") << endl;
       return false;
    }

    QTextStream outs(&presetFile);
    outs.setEncoding(QTextStream::UnicodeUTF8);

    outs << xmlData;

    if (presetFile.status() != IO_Ok) {
        kdDebug() << "StationList::writeXML: "
                  << i18n("error writing preset file")
                  << " " << file
                  << " (" << presetFile.state() << ")"
                  << endl;
        return false;
    }

    return true;
}


static const char *description = "convert-presets";

static KCmdLineOptions options[] =
{
  { "q", I18N_NOOP("be quiet"), 0},
  { "+[preset files]", I18N_NOOP("preset file to convert"), 0 },
  KCmdLineLastOption
};

int main(int argc, char *argv[])
{
    KAboutData aboutData("convert-presets", I18N_NOOP("convert-presets"),
                         VERSION, description, KAboutData::License_GPL,
                         "(c) 2003-2005 Martin Witte",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  "", "witte@kawo1.rwth-aachen.de");

    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

    KApplication a (false, false);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    for (int i = 0; i < args->count(); ++i) {
        const char *x = args->arg(i);
        if (! convertFile(x)) {
            return -1;
        } else {
            if (! args->isSet("q"))
                kdDebug() << x << ": ok" << endl;
        }
    }
    if (args->count() == 0) {
        kdDebug() << "no input" << endl;
        return -1;
    }

    return 0;
}

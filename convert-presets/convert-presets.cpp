#include <QtCore/QCoreApplication>
#include <QtCore/QString>
#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/QCommandLineParser>
#include <QtCore/QtGlobal>

#include <KLocalizedString>
#include <KAboutData>
#include <time.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

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

//    fprintf(stderr, "generated StationID: %s, %s\n",
//                    qPrintable(stime), qPrintable(srandom));

    return stime + srandom;
}


        #define stationIDElement  "stationID"


bool convertFile(const QString &file)
{
    ////////////////////////////////////////////////////////////////////////
    // read input
    ////////////////////////////////////////////////////////////////////////

    QFile presetFile (file);

    if (! presetFile.open(QIODevice::ReadOnly)) {
        fprintf(stderr, "error opening preset file %s for reading\n", qPrintable(file));
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
        fprintf(stdout, "%s already in new format\n", qPrintable(file));
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

    if (! presetFile.open(QIODevice::WriteOnly)) {
        fprintf(stderr, "error opening preset file %s for writing\n", qPrintable(file));
       return false;
    }

    QTextStream outs(&presetFile);
    outs.setCodec("UTF-8");
//     outs.setEncoding(QTextStream::UnicodeUTF8);

    outs << xmlData;

    if (presetFile.error() != QFile::NoError) {
        fprintf(stderr, "error writing preset file %s\n",
                        qPrintable(file));
        return false;
    }

    return true;
}


static const char *description = "convert-presets";

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    KAboutData aboutData(QStringLiteral("kradio5-convert-presets"),
			 i18n("kradio5-convert-presets"),
                         QStringLiteral(KRADIO_VERSION),
			 i18n(description),
			 KAboutLicense::LicenseKey::GPL,
                         i18n("(c) 2003-2020 Martin Witte"),
                         NULL,
                         "http://sourceforge.net/projects/kradio");
    aboutData.addAuthor(i18n("Martin Witte"),  NULL, "emw-kradio@nocabal.de");

    KAboutData::setApplicationData(aboutData);

    QCommandLineParser cmdLineParser;
    QCommandLineOption optionQuiet("q", i18n("quiet mode - no informational output"));
    cmdLineParser.addHelpOption();
    cmdLineParser.addVersionOption();
    cmdLineParser.addOption(optionQuiet);
    cmdLineParser.addPositionalArgument("files", i18n("preset files to convert"), "[preset files...]");
    
    cmdLineParser.process(app);

    QStringList  filenames = cmdLineParser.positionalArguments();

    for (const auto & fname : filenames) {
        if (! convertFile(fname)) {
            return -1;
        }
	else {
	  if (!cmdLineParser.isSet(optionQuiet))
	    fprintf(stdout, "%s: ok\n", qPrintable(fname));
        }
    }
    if (filenames.size() == 0) {
        fprintf(stderr, "no input\n");
        return -1;
    }

    return 0;
}

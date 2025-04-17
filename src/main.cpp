#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "agent.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName("AndreWaehlisch");
    app.setApplicationName("bthtscan");

    QFile outputFile("bthtscan.csv");

    if (!outputFile.open(QFile::WriteOnly | QFile::Append)) {
        qDebug() << "COULD NOT OPEN OUR OUTPUT FILE";
        exit(1);
    } else {
        qDebug() << "File open:" << outputFile.fileName();
    }

    QTextStream dataStream(&outputFile);

    const agent myagent(dataStream);
    return app.exec();
}

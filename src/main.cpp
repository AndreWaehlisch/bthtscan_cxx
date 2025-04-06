#include <QCoreApplication>
#include <QFile>
#include <QDataStream>
#include <QIODevice>
#include <QDebug>

#include "agent.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName("AndreWaehlisch");
    app.setApplicationName("bthtscan");

    QFile outputFile("test.output");

    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qDebug() << "COULD NOT OPEN OUR OUTPUT FILE";
        exit(1);
    } else {
        qDebug() << "File open:" << outputFile.fileName();
    }

    QDataStream dataStream(&outputFile);
    dataStream.setVersion(QDataStream::Qt_5_15);

    const agent myagent(dataStream, outputFile);
    return app.exec();
}

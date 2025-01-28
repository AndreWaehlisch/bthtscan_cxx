#include <QApplication>
#include <QTemporaryFile>
#include <QDataStream>
#include <QIODevice>
#include <QDebug>

#include "agent.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("AndreWaehlisch");
    app.setApplicationName("bthtscan");

    QTemporaryFile outputFile("test.output");

    //if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
    if (!outputFile.open()) {
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

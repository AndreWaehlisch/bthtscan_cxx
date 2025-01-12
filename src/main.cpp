#include <QApplication>

#include "agent.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("AndreWaehlisch");
    app.setApplicationName("bthtscan");

    const agent myagent;
    return app.exec();
}

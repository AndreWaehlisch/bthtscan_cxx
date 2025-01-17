#ifndef AGENT_H
#define AGENT_H

#include <QObject>
#include <QList>
#include <QStringList>
#include <QBluetoothDeviceInfo>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothLocalDevice>

#include "ble_agent.h"

class agent : public QObject
{
    Q_OBJECT

public:
    agent();

private:
    static const QStringList targetIDs;
    QList<QBluetoothDeviceInfo> foundList;
    QBluetoothDeviceDiscoveryAgent discoveryAgent;
    QBluetoothLocalDevice localDevice;
    QList<BLE_agent *> agentList;

private slots:
    void discoveryFinished();
    void discoveryErrorOccurred(QBluetoothDeviceDiscoveryAgent::Error);
    void discoveryDeviceDiscovered(const QBluetoothDeviceInfo &);
};

#endif // AGENT_H

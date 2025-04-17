#ifndef AGENT_H
#define AGENT_H

#include <QObject>
#include <QList>
#include <QStringList>
#include <QBluetoothDeviceInfo>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothLocalDevice>
#include <QTextStream>

// forward decleration so we avoid circular dependency
class BLE_agent;

class agent : public QObject
{
    Q_OBJECT

public:
    agent(QTextStream &);
    QTextStream &dataStream;

private:
    static const QStringList targetIDs;
    QList<QBluetoothDeviceInfo> foundList;
    QBluetoothDeviceDiscoveryAgent discoveryAgent;
    QBluetoothLocalDevice localDevice;
    QList<BLE_agent *> agentList;
    QList<QBluetoothDeviceInfo> pairingList;
    void do_BLE_connection(const QBluetoothDeviceInfo &);

private slots:
    void discoveryFinished();
    void discoveryErrorOccurred(QBluetoothDeviceDiscoveryAgent::Error);
    void discoveryDeviceDiscovered(const QBluetoothDeviceInfo &);
    void pairingFinished(const QBluetoothAddress &, QBluetoothLocalDevice::Pairing);
};

#endif // AGENT_H

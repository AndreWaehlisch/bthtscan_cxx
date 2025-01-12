#ifndef AGENT_H
#define AGENT_H

#include <QObject>
#include <QStringList>
#include <QBluetoothDeviceInfo>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothLocalDevice>
#include <QBluetoothAddress>

enum exitCodes {
    BLUETOOTH_OFF = 1,
    NOT_ALL_REQUIRED_DEVICES_FOUND,
    DISCOVERY_ERROR,
    PAIRING_ERROR
};

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

private slots:
    void discoveryFinished();
    void discoveryErrorOccurred(QBluetoothDeviceDiscoveryAgent::Error);
    void discoveryDeviceDiscovered(const QBluetoothDeviceInfo &);
    void localPairingFinished(const QBluetoothAddress &, QBluetoothLocalDevice::Pairing);
    void localPairingErrorOccurred(QBluetoothLocalDevice::Error);
};

#endif // AGENT_H

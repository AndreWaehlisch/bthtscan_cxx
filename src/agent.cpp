#include <QDebug>

#include "agent.h"

const QStringList agent::targetIDs = {"98:B0:8B:CC:A0:09"};

agent::agent() : QObject(nullptr)
{
    if (localDevice.hostMode() == QBluetoothLocalDevice::HostPoweredOff) {
        qDebug() << "Bluetooth is off. Exiting!";
        exit(BLUETOOTH_OFF);
    } else {
        qDebug() << "local host mode:" << localDevice.hostMode();
    }

    discoveryAgent.setLowEnergyDiscoveryTimeout(30000);

    connect(&discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &agent::discoveryFinished);
    connect(&discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred, this, &agent::discoveryErrorOccurred);
    connect(&discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &agent::discoveryDeviceDiscovered);
    connect(&localDevice, &QBluetoothLocalDevice::pairingFinished, this, &agent::localPairingFinished);
    connect(&localDevice, &QBluetoothLocalDevice::errorOccurred, this, &agent::localPairingErrorOccurred);

    qDebug() << "Going to scan for" << discoveryAgent.lowEnergyDiscoveryTimeout() / 1000 << "seconds";
    discoveryAgent.start(); // TODO: filter for BLE devices
}

void agent::discoveryFinished()
{
    qDebug() << "Discovery done, num detected:" << discoveryAgent.discoveredDevices().length();
    if (targetIDs.length() != foundList.length()) {
        qDebug() << "could not discover all required devices, exiting...";
        exit(NOT_ALL_REQUIRED_DEVICES_FOUND);
    }
}

void agent::discoveryErrorOccurred(QBluetoothDeviceDiscoveryAgent::Error error)
{
    qDebug() << "Discovery error!" << error;
    exit(DISCOVERY_ERROR);
}

void agent::discoveryDeviceDiscovered(const QBluetoothDeviceInfo &info)
{
    const QString deviceID = info.address().toString();

    if (targetIDs.contains(deviceID) && (!foundList.contains(info))) {
        qDebug() << "device!!!" << info.name() << deviceID;
        foundList.append(info);

        if (targetIDs.length() == foundList.length()) {
            qDebug() << "all devices discovered, stopping discovery.";
            discoveryAgent.stop();

            qDebug() << ">>>HERE IS SOME INFO ABOUT THE FOUND DEVICES<<<";
            foreach (QBluetoothDeviceInfo device_i, foundList) {
                qDebug() << " > ID:" << device_i.address() << "NAME:" << device_i.name() << "TYPE:" << device_i.coreConfigurations() << "SIGNALSTRENGTH:" << device_i.rssi();
            }

            qDebug() << "Trying to pair all devices...";
            foreach (QBluetoothDeviceInfo device_i, foundList) {
                localDevice.requestPairing(device_i.address(), QBluetoothLocalDevice::AuthorizedPaired);
            }
        }
    }
}

void agent::localPairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing)
{
    if (pairing == QBluetoothLocalDevice::Unpaired) {
        qDebug() << "Could not pair requested device:" << address << pairing;
    } else {
        qDebug() << "Successfully paired:" << address << pairing;
    }
}

void agent::localPairingErrorOccurred(QBluetoothLocalDevice::Error error)
{
    qDebug() << "Pairing error!" << error;
    exit(PAIRING_ERROR);
}

#include <QtGlobal>
#include <QDebug>

#include "global.h"
#include "agent.h"
#include "ble_agent.h"

const QStringList agent::targetIDs = {"0C:EF:F6:EF:A3:4E"}; // MAC addresses

agent::agent() : QObject(nullptr)
{
    if (localDevice.hostMode() == QBluetoothLocalDevice::HostPoweredOff) {
        qDebug() << "Bluetooth is off. Exiting!";
        exit(BLUETOOTH_OFF);
    } else {
        qDebug() << "local host mode:" << localDevice.hostMode();
    }

    discoveryAgent.setLowEnergyDiscoveryTimeout(61000);

    connect(&discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &agent::discoveryFinished);
    connect(&discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &agent::discoveryDeviceDiscovered);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(&discoveryAgent, QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(&QBluetoothDeviceDiscoveryAgent::error), this, &agent::discoveryErrorOccurred);
#else
    connect(&discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred, this, &agent::discoveryErrorOccurred);
#endif

    qDebug() << "Going to scan for" << discoveryAgent.lowEnergyDiscoveryTimeout() / 1000 << "seconds";
    discoveryAgent.start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
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

            qDebug() << "Trying to connect to all devices...";
            foreach (QBluetoothDeviceInfo device_i, foundList) {
                if (localDevice.pairingStatus(device_i.address()) == QBluetoothLocalDevice::Unpaired) {
                    qDebug() << "NOTE: device is UNpaired...";
                    //    pairingList.append(device_i);
                    //    localDevice.requestPairing(device_i.address(), QBluetoothLocalDevice::AuthorizedPaired);
                } else {
                    qDebug() << "NOTE: device is paired";
                    do_BLE_connection(device_i);
                }
            }
        }
    }
}

void agent::pairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing)
{
    for (qsizetype i = 0; i < pairingList.length(); i++) {
        QBluetoothDeviceInfo device_i = pairingList[i];
        qDebug() << "Pairing finished for device the following device, going to init BLE connection:" << device_i.address();

        if (address == device_i.address()) {
            do_BLE_connection(device_i);
            pairingList.remove(i);
            return;
        }
    }

    qDebug() << "Pairing finished for an unknown device. Smells like trouble...";
}

void agent::do_BLE_connection(const QBluetoothDeviceInfo &device_i)
{
    BLE_agent *new_BLE_agent = new BLE_agent(device_i, this);
    agentList.append(new_BLE_agent);
}

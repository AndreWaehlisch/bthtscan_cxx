#include <QtGlobal>
#include <QDebug>

#include "global.h"
#include "agent.h"
#include "ble_agent.h"

const QStringList agent::targetIDs = {"0C:EF:F6:EF:A3:4E"};

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
                qDebug() << "manuData" << device_i.manufacturerData();
		#if QT_VERSION > QT_VERSION_CHECK(6, 3, 0)
                qDebug() << "servData" << device_i.serviceData();
		#endif
            }

            qDebug() << "Trying to connect to all devices...";
            foreach (QBluetoothDeviceInfo device_i, foundList) {
                BLE_agent *new_BLE_agent = new BLE_agent(device_i, this);
                agentList.append(new_BLE_agent);
            }
        }
    }
}

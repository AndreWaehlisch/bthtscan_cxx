#include "global.h"
#include "ble_agent.h"

BLE_agent::BLE_agent(const QBluetoothDeviceInfo device, QObject *parent) : QObject(parent)
{
    controller = QLowEnergyController::createCentral(device, parent);

    connect(controller, &QLowEnergyController::serviceDiscovered, this, &BLE_agent::serviceDiscovered);
    connect(controller, &QLowEnergyController::discoveryFinished, this, &BLE_agent::discoveryFinished);


#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(controller, QOverload<QLowEnergyController::Error>::of(&QLowEnergyController::error), this, &BLE_agent::errorOccurred);
#else
    connect(controller, &QLowEnergyController::errorOccurred, this, &BLE_agent::errorOccurred);
#endif

    connect(controller, &QLowEnergyController::connected, this, &BLE_agent::connected);
    connect(controller, &QLowEnergyController::disconnected, this, &BLE_agent::disconnected);

    qDebug() << "Controller preperation done, connecting now!" << device.address() << device.name() << controller->state() << controller->localAddress();
    controller->connectToDevice();
}

void BLE_agent::serviceDiscovered(const QBluetoothUuid &newService)
{
    qDebug() << "Service discovered :-) " << newService << newService.toUInt16() << QString::number(newService.toUInt16(), 16);

    if (newService == QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::DeviceInformation)) {
        QLowEnergyService * service = controller->createServiceObject(newService);
        servicesList.append(service);
        qDebug() << "DeviceInformation Service detected. Service name:" << service->serviceName();

        connect(service, &QLowEnergyService::stateChanged, this, &BLE_agent::serviceStateChanged);
        connect(service, &QLowEnergyService::characteristicChanged, this, &BLE_agent::updateValue);
        connect(service, &QLowEnergyService::descriptorWritten, this, &BLE_agent::descriptorWritten);

        service->discoverDetails();
    } else {
        QLowEnergyService * service = controller->createServiceObject(newService);
        //servicesList.append(service);
        qDebug() << "Some different Service detected. Service name:" << service->serviceName();
    }
}

void BLE_agent::discoveryFinished()
{
    qDebug() << "Service discovery finished.";
}

void BLE_agent::errorOccurred(QLowEnergyController::Error error)
{
    qDebug() << "Discovery error!" << error << controller->errorString();
    exit(BLE_ERROR);
}

void BLE_agent::connected()
{
    qDebug() << "Controller connected! Now search for services...";
    controller->discoverServices();
}

void BLE_agent::disconnected()
{
    qDebug() << "BLE controller disconnected! This is trouble.." << this->controller->remoteAddress() << this->controller->remoteName();
}

void BLE_agent::serviceStateChanged(QLowEnergyService::ServiceState newState)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (newState != QLowEnergyService::DiscoveringServices) {
#else
    if (newState != QLowEnergyService::RemoteServiceDiscovering) {
#endif
        qDebug() << "Service state changed" << newState;

        if (newState == QLowEnergyService::ServiceDiscovered) {
            foreach (QLowEnergyCharacteristic characteristic, servicesList[0]->characteristics()) {
                qDebug() << "Characteristic: " << characteristic.value() << characteristic.name();
            }
        }

        qDebug() << "All characteristics listed.";
    }
}

void BLE_agent::updateValue(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    qDebug() << "Service value updated!" << characteristic.name() << newValue;
}

void BLE_agent::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    qDebug() << "Service descriptor written!" << descriptor.name() << newValue;
}

void BLE_agent::serviceError(QLowEnergyService::ServiceError error)
{
    qDebug() << "Service ERROR:" << error;
}

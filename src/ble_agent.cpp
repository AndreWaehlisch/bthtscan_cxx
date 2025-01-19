#include <QLowEnergyCharacteristic>
#include <QLowEnergyDescriptor>
#include <QByteArray>
#include <QDateTime>

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
    //qDebug() << "Service discovered :-) " << newService << newService.toUInt16() << newService.toString() << QString::number(newService.toUInt16(), 16);

    if (newService.toString() == "{de8a5aac-a99b-c315-0c80-60d4cbb51225}") {
        QLowEnergyService * service = controller->createServiceObject(newService);
        servicesList.append(service);
        qDebug() << "Special Service detected:" << service->serviceName() << service->serviceUuid();

        connect(service, &QLowEnergyService::stateChanged, this, &BLE_agent::serviceStateChanged);

        service->discoverDetails();
    } else {
        //QLowEnergyService * service = controller->createServiceObject(newService);
        //servicesList.append(service);
        //qDebug() << "Some different Service detected. Service name:" << service->serviceName();
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
        qDebug() << "";
        qDebug() << "Service state changed" << newState;

        if (newState == QLowEnergyService::ServiceDiscovered) {
            QLowEnergyCharacteristic characteristic = servicesList[0]->characteristic(QBluetoothUuid::fromString("d52246df-98ac-4d21-be1b-70d5f66a5ddb")); // see here: https://shelly-api-docs.shelly.cloud/docs-ble/common/#common-gatt-services-and-characteristics
            if (characteristic.isValid()) {
                qDebug() << "We found our 1st Characteristic!" << characteristic.uuid() << characteristic.value() << characteristic.name() << characteristic.properties() << "--- Descriptor list length:" << characteristic.descriptors().length();
                const QByteArray bytes = characteristic.value();
                qDebug() << "TEST" << bytes << "--__--" << bytes.toHex();
                if (bytes.length() > 1) {
                    QByteArray::const_iterator it = bytes.cbegin();
                    it++; // skip first byte "D"

                    while (it != bytes.cend()) {
                        switch (*it) {
                        case 0x00: {
                            it++; // look at the next byte

                            const quint8 packetID = static_cast<quint8>(*it);
                            qDebug() << "packet ID is:" << packetID;
                            break;
                        }
                        case 0x01: {
                            it++; // look at the next byte

                            const quint8 battery = static_cast<quint8>(*it);
                            qDebug() << "Battery level is: " << battery << "%";
                            break;
                        }
                        case 0x2E: {
                            it++; // look at the next byte

                            const quint8 humidity = static_cast<quint8>(*it);
                            qDebug() << "Humidity level is:" << humidity << "%";
                            break;
                        }
                        case 0x45: {
                            it++; // look at the next 2 bytes

                            qint16 temp = static_cast<qint16>(it[1]) << 8; // add first byte to the int16
                            temp |= static_cast<qint8>(it[0]); // add second byte
                            qDebug() << "Temperature:" << temp * 0.1 << "°C";
                            it++; // temp is 2 bytes, so advance one additional time
                            break;
                        }
                        default:
                            qDebug() << "default case in iterator reached !!!";
                            break;
                        }

                        it++;
                    };
                } else {
                    qDebug() << "Payload too small!";
                    exit(PAYLOAD_TOO_SMALL);
                }
                qDebug() << "Done with payload.";
            } else {
                qDebug() << "Required Characteristic 1 NOT found!";
                exit(CHARACTERISTIC_NOT_FOUND);
            }

            characteristic = servicesList[0]->characteristic(QBluetoothUuid::fromString("d56a3410-115e-41d1-945b-3a7f189966a1")); // see here: https://shelly-api-docs.shelly.cloud/docs-ble/common/#common-gatt-services-and-characteristics
            if (characteristic.isValid()) {
                const QByteArray bytes = characteristic.value();

                // reverse byte order
                QByteArray reversedBytes;
                for (QByteArray::const_reverse_iterator it = bytes.crbegin(); it != bytes.crend(); it++) {
                    reversedBytes.append(*it);
                }

                // convert our 4 bytes to int32
                QDataStream stream(reversedBytes);
                qint32 converted;
                stream >> converted;

                // convert int32 to QDateTime
                const QDateTime dateTimeStamp = QDateTime::fromSecsSinceEpoch(converted);

                qDebug() << "We found our 2nd Characteristic!" << characteristic.uuid() << characteristic.value();
                qDebug() << "UNIX time stamp:" << dateTimeStamp << bytes << converted;
            } else {
                qDebug() << "Required Characteristic 2 NOT found!";
                exit(CHARACTERISTIC_NOT_FOUND);
            }
        } else {
            qDebug() << "Did not list characteristics. Length serviceList:" << servicesList[0]->characteristics().length();
        }
    }
}

void BLE_agent::serviceError(QLowEnergyService::ServiceError error)
{
    qDebug() << "Service ERROR:" << error;
}

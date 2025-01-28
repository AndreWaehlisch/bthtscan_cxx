#include <QLowEnergyCharacteristic>
#include <QLowEnergyDescriptor>
#include <QByteArray>
#include <QDateTime>
#include <QDataStream>

#include "global.h"
#include "ble_agent.h"

BLE_agent::BLE_agent(const QBluetoothDeviceInfo device, QObject *parent) : QObject(parent)
{
    controller = QLowEnergyController::createCentral(device, parent);

    connect(controller, &QLowEnergyController::serviceDiscovered, this, &BLE_agent::serviceDiscovered);
    connect(controller, &QLowEnergyController::discoveryFinished, this, &BLE_agent::discoveryFinished);
    connect(controller, &QLowEnergyController::stateChanged, this, &BLE_agent::stateChanged);


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
    if (newService.toString() == "{de8a5aac-a99b-c315-0c80-60d4cbb51225}") {
        QLowEnergyService * service = controller->createServiceObject(newService);
        this->service = service;
        qDebug() << "Our Service detected:" << service->serviceName() << service->serviceUuid();

        connect(service, &QLowEnergyService::stateChanged, this, &BLE_agent::serviceStateChanged);

        service->discoverDetails();
    }
}

void BLE_agent::discoveryFinished()
{
    qDebug() << "Service discovery finished." << controller->state();
}

void BLE_agent::errorOccurred(QLowEnergyController::Error error)
{
    qDebug() << "Discovery error!" << error << controller->errorString() << controller->state();
}

void BLE_agent::connected()
{
    qDebug() << "Controller connected! Now search for services...";
    controller->discoverServices();
}

void BLE_agent::disconnected()
{
    qDebug() << "BLE controller disconnected! This may be trouble.." << this->controller->remoteAddress() << this->controller->remoteName();
}

void BLE_agent::stateChanged(QLowEnergyController::ControllerState state)
{
    qDebug() << "State changed. New state:" << state;

    // retry connection
    if (state == QLowEnergyController::UnconnectedState) {
        controller->connectToDevice();
    }
}

void BLE_agent::serviceStateChanged(QLowEnergyService::ServiceState newState)
{
    if (newState == QLowEnergyService::ServiceDiscovered) {
        qDebug() << "";
        qDebug() << "Service DISCOVERED" << newState;

        // see here: https://shelly-api-docs.shelly.cloud/docs-ble/common/#common-gatt-services-and-characteristics
        QLowEnergyCharacteristic characteristic = service->characteristic(QBluetoothUuid(QString("d52246df-98ac-4d21-be1b-70d5f66a5ddb")));

        if (characteristic.isValid()) {
            qDebug() << "We found our 1st Characteristic!" << characteristic.uuid() << characteristic.value() << characteristic.name() << characteristic.properties() << "--- Descriptor list length:" << characteristic.descriptors().length();
            const QByteArray bytes = characteristic.value();
            qDebug() << "TEST" << bytes << "--__--" << bytes.toHex();
            if (bytes.length() > 1) {
                QByteArray::const_iterator it = bytes.cbegin();
                it++; // skip first byte "D"

                // see https://shelly-api-docs.shelly.cloud/docs-ble/Devices/ht/
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

                        // reverse byte order
                        QByteArray reversed2Bytes;
                        reversed2Bytes.append(it[1]);
                        reversed2Bytes.append(it[0]);

                        // convert our 2 bytes to int16
                        const qint16 temp = qFromBigEndian<qint16>(reversed2Bytes);
                        qDebug() << "Temperature:" << temp * 0.1 << "°C" << reversed2Bytes.toInt();
                        it++; // we used 2 bytes, so advance one additional time
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

        // see here: https://shelly-api-docs.shelly.cloud/docs-ble/common/#common-gatt-services-and-characteristics
        characteristic = service->characteristic(QBluetoothUuid(QString("d56a3410-115e-41d1-945b-3a7f189966a1")));

        if (characteristic.isValid()) {
            const QByteArray bytes = characteristic.value();

            // reverse byte order
            QByteArray reversed4Bytes;
            for (QByteArray::const_reverse_iterator it = bytes.crbegin(); it != bytes.crend(); it++) {
                reversed4Bytes.append(*it);
            }

            // convert our 4 bytes to int32
            const qint32 converted = qFromBigEndian<qint32>(reversed4Bytes);

            // convert int32 to QDateTime
            const QDateTime dateTimeStamp = QDateTime::fromSecsSinceEpoch(converted);

            qDebug() << "We found our 2nd Characteristic!" << characteristic.uuid() << characteristic.value();
            qDebug() << "UNIX time stamp:" << dateTimeStamp << bytes << converted;
        } else {
            qDebug() << "Required Characteristic 2 NOT found!";
            exit(CHARACTERISTIC_NOT_FOUND);
        }
    }
}

void BLE_agent::serviceError(QLowEnergyService::ServiceError error)
{
    qDebug() << "Service ERROR:" << error;
}

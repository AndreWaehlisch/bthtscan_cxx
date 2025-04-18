#include <QtEndian>

#include "global.h"
#include "agent.h"
#include "ble_agent.h"

BLE_agent::BLE_agent(const QBluetoothDeviceInfo device, agent *parent) : QObject(parent), parent(parent)
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &BLE_agent::newServiceRead);

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

void BLE_agent::newServiceRead()
{
    service->readCharacteristic(characteristic1);
    //service->readCharacteristic(characteristic2);
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
    qDebug() << "Controller error!" << error << controller->errorString() << controller->state();
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

void BLE_agent::processCharacteristic1(const QLowEnergyCharacteristic &characteristic, const QByteArray & bytes)
{
    qDebug() << "We found our 1st Characteristic!" << characteristic.uuid() << characteristic.value() << characteristic.name() << characteristic.properties() << "--- Descriptor list length:" << characteristic.descriptors().length();

    quint8 packetID = 0;
    quint8 battery = 127; // if this is not overwritten below, this value indicates a failure state
    quint8 humidity = 127;
    qint16 temp = -127;

    qDebug() << "TEST" << bytes << "--__--" << bytes.toHex();
    if (bytes.length() > 1) {
        QByteArray::const_iterator it = bytes.cbegin();
        it++; // skip first byte "D"

        // see https://shelly-api-docs.shelly.cloud/docs-ble/Devices/ht/
        while (it != bytes.cend()) {
            switch (*it) {
            case 0x00: {
                it++; // look at the next byte

                packetID = static_cast<quint8>(*it);
                qDebug() << "packet ID is:" << packetID;
                break;
            }
            case 0x01: {
                it++; // look at the next byte

                battery = static_cast<quint8>(*it);
                qDebug() << "Battery level is: " << battery << "%";
                break;
            }
            case 0x2E: {
                it++; // look at the next byte

                humidity = static_cast<quint8>(*it);
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
                temp = qFromBigEndian<qint16>(reversed2Bytes.constData());
                qDebug() << "Temperature:" << temp * 0.1 << "°C";
                it++; // we used 2 bytes, so advance one additional time
                break;
            }
            default:
                qDebug() << "default case in iterator reached !!!" << bytes.length();
                break;
            }

            it++;
        };

        writeSensorData1(packetID, battery, humidity, temp); // TODO: handle characteristic failing, in that case NO data should be written, also NO datetime should be written below
    } else {
        qDebug() << "Payload too small!";
        exit(PAYLOAD_TOO_SMALL);
    }
    qDebug() << "Done with payload.";
}

/*
void BLE_agent::processCharacteristic2(const QLowEnergyCharacteristic &characteristic, const QByteArray & bytes)
{
    // reverse byte order
    QByteArray reversed4Bytes;
    for (QByteArray::const_reverse_iterator it = bytes.crbegin(); it != bytes.crend(); it++) {
        reversed4Bytes.append(*it);
    }

    // convert our 4 bytes to int32
    const qint32 converted = qFromBigEndian<qint32>(reversed4Bytes.constData());

    // convert int32 to QDateTime
    const QDateTime dateTimeStamp = QDateTime::fromSecsSinceEpoch(converted);

    qDebug() << "We found our 2nd Characteristic!" << characteristic.uuid() << characteristic.value();
    qDebug() << "UNIX time stamp:" << dateTimeStamp << bytes << converted;

    writeSensorData2(dateTimeStamp); // TODO: handle characteristic failing, still the current datetime should be saved
}
*/

void BLE_agent::serviceStateChanged(QLowEnergyService::ServiceState newState)
{
    if (newState == QLowEnergyService::ServiceDiscovered) {
        qDebug() << "";
        qDebug() << "Service DISCOVERED" << newState;

        // sensor data; see here: https://shelly-api-docs.shelly.cloud/docs-ble/common/#common-gatt-services-and-characteristics
        characteristic1 = service->characteristic(QBluetoothUuid(QString("d52246df-98ac-4d21-be1b-70d5f66a5ddb")));

        if (characteristic1.isValid()) {
            const QByteArray bytes = characteristic1.value();
            processCharacteristic1(characteristic1, bytes);
            connect(service, &QLowEnergyService::characteristicRead, this, &BLE_agent::processCharacteristic1);
        } else {
            qDebug() << "Required Characteristic 1 NOT found/valid!";
            exit(CHARACTERISTIC_NOT_FOUND);
        }

        // in principle the device gives a timestamp, but this was buggy, so just use the current time
        const QDateTime now = QDateTime::currentDateTime();
        writeSensorData2(now);
        qDebug() << "Current DateTime timestamp" << now;

        /*
        // timestamp; see here: https://shelly-api-docs.shelly.cloud/docs-ble/common/#common-gatt-services-and-characteristics
        characteristic2 = service->characteristic(QBluetoothUuid(QString("d56a3410-115e-41d1-945b-3a7f189966a1")));

        if (characteristic2.isValid()) {
            const QByteArray bytes = characteristic2.value();
            processCharacteristic2(characteristic2, bytes);
            connect(service, &QLowEnergyService::characteristicRead, this, &BLE_agent::processCharacteristic2);
        } else {
            qDebug() << "Required Characteristic 2 NOT found/valid!";
            exit(CHARACTERISTIC_NOT_FOUND);
        }
        */

        // check every 60s for new value
        timer->start(60000);
    }
}

void BLE_agent::serviceError(QLowEnergyService::ServiceError error)
{
    qDebug() << "Service ERROR:" << error;
}

void BLE_agent::writeSensorData1(const quint8 packetID, const quint8 battery, const quint8 humidity, const qint16 temp)
{
    parent->dataStream << packetID << ",";
    parent->dataStream << battery << ",";
    parent->dataStream << humidity << ",";
    parent->dataStream << temp << ",";
}

void BLE_agent::writeSensorData2(const QDateTime dateTimeStamp)
{
    parent->dataStream << dateTimeStamp.toString("hh:mm:ss-dd.MM.yyyy") << "\n";
    parent->dataStream.flush();
}

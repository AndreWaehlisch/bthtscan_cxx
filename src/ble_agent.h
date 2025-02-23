#ifndef BLE_AGENT_H
#define BLE_AGENT_H

#include <QObject>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QBluetoothUuid>
#include <QLowEnergyService>
#include <QByteArray>
#include <QDateTime>
#include <QLowEnergyCharacteristic>
#include <QTimer>

// forward decleration so we avoid circular dependency
class agent;

class BLE_agent : public QObject
{
    Q_OBJECT

public:
    BLE_agent(const QBluetoothDeviceInfo, agent *);

private:
    const agent *parent;
    QTimer *timer;
    QLowEnergyController *controller;
    QLowEnergyService *service;
    QLowEnergyCharacteristic characteristic1;
    QLowEnergyCharacteristic characteristic2;
    void newServiceRead();
    void processCharacteristic1(const QLowEnergyCharacteristic &, const QByteArray &);
    void processCharacteristic2(const QLowEnergyCharacteristic &, const QByteArray &);
    void writeSensorData1(const QByteArray, const quint8, const quint8, const quint8, const qint16);
    void writeSensorData2(const QDateTime);

private slots:
    void serviceDiscovered(const QBluetoothUuid &);
    void discoveryFinished();
    void errorOccurred(QLowEnergyController::Error);
    void connected();
    void disconnected();
    void serviceStateChanged(QLowEnergyService::ServiceState);
    void serviceError(QLowEnergyService::ServiceError);
    void stateChanged(QLowEnergyController::ControllerState);
};

#endif // BLE_AGENT_H

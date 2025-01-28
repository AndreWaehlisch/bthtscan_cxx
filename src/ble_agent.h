#ifndef BLE_AGENT_H
#define BLE_AGENT_H

#include <QObject>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QBluetoothUuid>
#include <QLowEnergyService>
#include <QByteArray>
#include <QDateTime>

// forward decleration so we avoid circular dependency
class agent;

class BLE_agent : public QObject
{
    Q_OBJECT

public:
    BLE_agent(const QBluetoothDeviceInfo, agent *);

private:
    const agent *parent;
    QLowEnergyController *controller;
    QLowEnergyService *service;
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

#ifndef BLE_AGENT_H
#define BLE_AGENT_H

#include <QObject>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QBluetoothUuid>
#include <QLowEnergyService>

class BLE_agent : public QObject
{
    Q_OBJECT

public:
    BLE_agent(const QBluetoothDeviceInfo, QObject *);

private:
    QLowEnergyController *controller;
    QLowEnergyService *service;

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

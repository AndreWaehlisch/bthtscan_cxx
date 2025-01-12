#include <QApplication>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QBluetoothLocalDevice>

#include "agent.h"

int main(int argc, char *argv[])
{
	QBluetoothDeviceDiscoveryAgent *agent = new QBluetoothDeviceDiscoveryAgent();
	const agent myagent = agent();
	return 0;
}

#include <QtCore/qcoreapplication.h>
#include <QtCore/qcommandlineoption.h>
#include <QtCore/qcommandlineparser.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qthread.h>

#include <QtNetwork/qtcpsocket.h>

#include <QtSerialPort/qserialport.h>

#include "config.h"

#define RETRY_INTERVAL 3000

void run(QTcpSocket *socket, QSerialPort *serial, QTextStream &out);

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("Tcp Serial");
    app.setApplicationVersion(TCP_SERIAL_VERSION_STR);

    QCommandLineOption helpOption("help", "Show this help.");
    QCommandLineOption versionOption("version", "Show version.");
    QCommandLineOption hostOption({"h", "host"}, "Address to use for TCP connection.", "address", "localhost");
    QCommandLineOption portOption({"p", "port"}, "Port to use for TCP Connection.", "port");
    QCommandLineOption serialOption({"s", "serial"}, "Serial port to use.", "port");
    QCommandLineOption autoReconnectOption({"a", "auto-reconnect"}, "Enable automatic reconnection.");

    QCommandLineParser parser;
    parser.setApplicationDescription("TcpSerial");
    parser.addOptions({helpOption, versionOption});
    parser.addOptions({hostOption, portOption, serialOption});
    parser.addOptions({autoReconnectOption});
    parser.process(app);

    QTextStream out(stdout), err(stderr);

    if (parser.isSet(helpOption) || !parser.isSet(portOption) || !parser.isSet(serialOption)) {
        parser.showHelp();
        return 0;
    } else if (parser.isSet(versionOption)) {
        parser.showVersion();
        return 0;
    }

    out << parser.value(hostOption) << ':' << parser.value(portOption)
        << " <-> " << parser.value(serialOption) << Qt::endl;

    QTcpSocket socket;
    QSerialPort serial(parser.value(serialOption));

    for (int i(-1); i < 2; ++i) {
        if (socket.state() == QAbstractSocket::UnconnectedState) {
            if (i >= 0) {
                out << "Disconnected" << Qt::endl;
                QThread::msleep(RETRY_INTERVAL);
            }

            out << "Connecting to " << parser.value(hostOption) << "..." << Qt::endl;
            socket.connectToHost(parser.value(hostOption), parser.value(portOption).toUInt());
            if (socket.waitForConnected()) {
                out << "Connected" << Qt::endl;
                i = 0;
            } else {
                err << socket.errorString() << Qt::endl;
            }
        } else if (!serial.isOpen()) {
            if (i >= 0) {
                out << "Disconnected" << Qt::endl;
                QThread::msleep(RETRY_INTERVAL);
            }

            out << "Opening port " << parser.value(serialOption) << "..." << Qt::endl;
            if (serial.open(QIODevice::ReadWrite)) {
                out << "Opened" << Qt::endl;
                i = 0;
            } else {
                err << serial.errorString() << Qt::endl;
            }
        } else {
            run(&socket, &serial, out);
            i = (parser.isSet(autoReconnectOption) ? 0 : 3 - 1);
        }
    }

    return 0;
}

void run(QTcpSocket *socket, QSerialPort *serial, QTextStream &out)
{
    if (socket->waitForReadyRead(500)) {
        out << "Tcp -> Serial: Forwarding " << socket->bytesAvailable()
            << " bytes of data" << Qt::endl;
        serial->write(socket->readAll());
        serial->waitForBytesWritten();
    }

    if (serial->waitForReadyRead(500)) {
        out << "Serial -> Tcp: Forwarding " << serial->bytesAvailable()
            << " bytes of data" << Qt::endl;
        socket->write(serial->readAll());
        socket->waitForBytesWritten();
    }
}

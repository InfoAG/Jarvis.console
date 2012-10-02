#ifndef INPUTWORKER_H
#define INPUTWORKER_H

#include "JarvisClient.h"
#include "TerminalPrinter.h"

class InputWorker : public QObject
{
    Q_OBJECT

private:
    JarvisClient &client;
    TerminalPrinter &printer;

public:
    InputWorker(JarvisClient &client, TerminalPrinter &printer) : client(client), printer(printer) {};

public slots:
    void doWork() {
        QTextStream qtin(stdin);
        QString input;
        for (;;) {
            input = qtin.readLine();
            if (input.startsWith("/enter ")) QMetaObject::invokeMethod(&client, "enterRoom", Q_ARG(QString, input.right(input.length() - 7)));
            else if (input.startsWith("/leave ")) QMetaObject::invokeMethod(&printer, "leaveRoom", Q_ARG(QString, input.right(input.length() - 7)));
            else if (input.startsWith("/open ")) QMetaObject::invokeMethod(&printer, "openRoom", Q_ARG(QString, input.right(input.length() - 6)));
            else if (input == "/modules") QMetaObject::invokeMethod(&printer, "printModules");
            else if (input.startsWith("/unload ")) QMetaObject::invokeMethod(&client, "unloadPkg", Q_ARG(QString, input.right(input.length() - 8)));
            else if (input.startsWith("/load ")) QMetaObject::invokeMethod(&client, "loadPkg", Q_ARG(QString, input.right(input.length() - 6)));
            else if (input.startsWith("/delete ")) QMetaObject::invokeMethod(&client, "deleteRoom", Q_ARG(QString, input.right(input.length() - 8)));
            else if (input == "/clients") QMetaObject::invokeMethod(&printer, "printClients");
            else if (input == "/rooms") QMetaObject::invokeMethod(&printer, "printRooms");
            else if (input == "/variables") QMetaObject::invokeMethod(&printer, "printVariables");
            else if (input == "/functions") QMetaObject::invokeMethod(&printer, "printFunctions");
            else QMetaObject::invokeMethod(&printer, "msgToRoom", Q_ARG(QString, input));
        }
    }
};

#endif // INPUTWORKER_H

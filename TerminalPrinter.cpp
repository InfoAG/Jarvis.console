 #include "TerminalPrinter.h"

TerminalPrinter::TerminalPrinter(JarvisClient &client) : client(client), qtout(stdout)
{
    connect(&client, SIGNAL(msgInRoom(const QString &, const QString &, const QString &)), SLOT(msgInRoom(const QString &, const QString &, const QString &)));
    connect(&client, SIGNAL(newFunction(const QString &, const QString &, const QStringList &, const QString &)), SLOT(newFunction(const QString &, const QString &, const QStringList &, const QString &)));
    connect(&client, SIGNAL(newRoom(const QString &)), SLOT(newRoom(const QString &)));
    connect(&client, SIGNAL(deletedRoom(const QString &)), SLOT(deletedRoom(const QString &)));
    connect(&client, SIGNAL(newVariable(const QString &, const QString &, const QString &)), SLOT(newVariable(const QString &, const QString &, const QString &)));
    connect(&client, SIGNAL(newClient(const QString &, const QString &)), SLOT(newClient(const QString &, const QString &)));
    connect(&client, SIGNAL(clientLeft(const QString &, const QString &)), SLOT(clientLeft(const QString &, const QString &)));
    connect(&client, SIGNAL(error(JarvisClient::ClientError)), SLOT(error(JarvisClient::ClientError)));
    connect(&client, SIGNAL(pkgLoaded(const ModulePackage &)), SLOT(pkgLoaded(const ModulePackage &)));
    connect(&client, SIGNAL(pkgUnloaded(const QString &)), SLOT(pkgUnloaded(const QString &)));
    connect(&client, SIGNAL(enteredRoom(const QString &, const Room &)), SLOT(enteredRoom(const QString &, const Room &)));
    connect(&client, SIGNAL(receivedInitInfo(const QStringList &, const QList<ModulePackage> &)), SLOT(receivedInitInfo(const QStringList &, const QList<ModulePackage> &)));
    connect(&client, SIGNAL(disconnected()), SLOT(disconnected()));
}

void TerminalPrinter::newRoom(const QString &name)
{
    qtout << "\nNew Room:\t" << name << "\n";
    qtout << "(" << currentRoom << ")->";
    qtout.flush();
    serverRooms.append(name);
}

void TerminalPrinter::newFunction(const QString &room, const QString &identifier, const QStringList &arguments, const QString &def)
{
    qtout << "\nNew function definition (room " << room << "):\t" << identifier << "(" << arguments.front();
    for (QStringList::const_iterator it = arguments.begin() + 1; it != arguments.end(); ++it) qtout << "," << *it;
    qtout << ")=" << def << "\n";
    qtout << "(" << currentRoom << ")->";
    qtout.flush();
    roomByName[room].functions.insert(identifier, qMakePair(arguments, def));
}

void TerminalPrinter::newVariable(const QString &room, const QString &identifier, const QString &definition)
{
    qtout << "\nNew variable definition (room " << room << "):\t" << identifier << "=" << definition << "\n";
    qtout << "(" << currentRoom << ")->";
    qtout.flush();
    roomByName[room].variables.insert(identifier, definition);
}

void TerminalPrinter::newClient(const QString &room, const QString &name)
{
    qtout << "\nNew client (room " << room << "):\t" << name << "\n";
    qtout << "(" << currentRoom << ")->";
    qtout.flush();
    roomByName[room].clients.append(name);
}

void TerminalPrinter::clientLeft(const QString &room, const QString &name)
{
    qtout << "\nClient left (room " << room << "):\t" << name << "\n";
    qtout << "(" << currentRoom << ")->";
    qtout.flush();
    roomByName[room].clients.removeOne(name);
}

void TerminalPrinter::msgInRoom(const QString &room, const QString &sender, const QString &msg)
{
    qtout << "\n[" << room << "] " << sender << ":\t" << msg << "\n";
    qtout << "(" << currentRoom << ")->";
    qtout.flush();
}

void TerminalPrinter::error(JarvisClient::ClientError error)
{
    qtout << "\nClient Error " << error << "\n";
    qtout << "(" << currentRoom << ")->";
    qtout.flush();
}

void TerminalPrinter::pkgLoaded(const ModulePackage &pkg)
{
    qtout << "\nPackage loaded:\n";
    printPackage(pkg);
    qtout << "(" << currentRoom << ")->";
    qtout.flush();
    pkgs.append(pkg);
}

void TerminalPrinter::pkgUnloaded(const QString &name)
{
    qtout << "\nPackage unloaded: " << name << "\n";
    qtout << "(" << currentRoom << ")->";
    qtout.flush();
    pkgs.erase(std::remove_if(pkgs.begin(), pkgs.end(), [&](const ModulePackage &pkg) { return pkg.name == name; }));
}

void TerminalPrinter::enteredRoom(const QString &name, const Room &info)
{
    qtout << "\nEntered room " << name << "; Clients:\n";
    for (const auto &client : info.clients) qtout << client << "\t";
    qtout << "\nVariables:\n";
    doPrintVars(info);
    qtout << "\nFunctions:\n";
    doPrintFuncs(info);
    qtout << "\n(" << currentRoom << ")->";
    qtout.flush();
    roomByName.insert(name, info);
}

void TerminalPrinter::receivedInitInfo(const QStringList &rooms, const QList<ModulePackage> &pkgs)
{
    qtout << "InitInfo:\n\n";
    qtout << "Rooms:\n";
    for (const auto &room : rooms) {
        qtout << room << "\t";
        serverRooms.append(room);
    }
    qtout << "\nPackages:\n";
    for (const auto &pkg : pkgs) {
        printPackage(pkg);
    }
    this->pkgs = pkgs;
    qtout << "(" << currentRoom << ")->";
    qtout.flush();
}

void TerminalPrinter::printClients()
{
    if (! currentRoom.isEmpty()) {
        for (const auto &client : roomByName[currentRoom].clients) qtout << client << "\t";
        qtout << "\n(" << currentRoom << ")->";
        qtout.flush();
    }
}

void TerminalPrinter::printRooms()
{
    for (const auto &room : serverRooms) qtout << room << "\t";
    qtout << "\n(" << currentRoom << ")->";
    qtout.flush();
}

void TerminalPrinter::deletedRoom(const QString &name)
{
    qtout << "Deleted room " << name << "\n";
    qtout.flush();
    roomByName.remove(name);
    serverRooms.removeOne(name);
    if (currentRoom == name) currentRoom.clear();
    qtout << "(" << currentRoom << ")->";
}

void TerminalPrinter::printModules()
{
    for (const auto &pkg : pkgs) printPackage(pkg);
    qtout << "\n(" << currentRoom << ")->";
    qtout.flush();
}

void TerminalPrinter::leaveRoom(const QString &name)
{
    if (roomByName.contains(name)) {
        roomByName.remove(name);
        if (currentRoom == name) currentRoom.clear();
        client.leaveRoom(name);
        qtout << "Left room " + name;
    } else qtout << "I'm not in a room called " + name;
    qtout << "\n(" << currentRoom << ")->";
    qtout.flush();
}

void TerminalPrinter::printPackage(const ModulePackage &pkg)
{
    qtout << "Package Name\tModule Name\tModule Description\n";
    qtout << pkg.name << "\n";
    qtout << "\tTerminals:\n";
    for (const auto &mod : pkg.terminals) {
        qtout << "\t\t" << mod.name << "\t" << mod.description << "\n";
    }
    qtout << "\tBinary Operators:\n";
    for (const auto &mod : pkg.binaryOperators) {
        printOperator(mod);
        qtout << "\n\t\t\tassociativity:\t";
        if (mod.associativity == BinaryOperatorModule::LEFT)
            qtout << "left";
        else if (mod.associativity == BinaryOperatorModule::RIGHT) qtout << "right";
        else qtout << "<dynamic>";
        qtout << "\n";
    }
    qtout << "\tUnary Operators:\n";
    for (const auto &mod : pkg.unaryOperators) {
        printOperator(mod);
        qtout << "\n\t\t\talignment:\t";
        if (mod.alignment == UnaryOperatorModule::PRE)
            qtout << "pre";
        else if (mod.alignment == UnaryOperatorModule::POST) qtout << "post";
        else qtout << "<dynamic>";
        qtout << "\n";
    }
    qtout << "\tFunctions:\n";
    for (const auto &mod : pkg.functions) {
        qtout << "\t\t" << mod.name << "\t" << mod.description << "\n";
        qtout << "\t\t\tmatches:\t";
        if (mod.matches == nullptr)
            qtout << "<dynamic>";
        else
            qtout << mod.matches->first << "\t" << QString::number(mod.matches->second);
        qtout << "\n\t\t\tpriority:\t";
        if (mod.priority.first)
            qtout << mod.priority.second;
        else
            qtout << "<dynamic>";
        qtout << "\n";
    }
    qtout.flush();
}

void TerminalPrinter::printOperator(const OperatorModule &mod)
{
    qtout << "\t\t" << mod.name << "\t" << mod.description << "\n";
    qtout << "\t\t\tmatches:\t";
    if (mod.matches == nullptr)
        qtout << "<dynamic>";
    else
        qtout << *mod.matches;
    qtout << "\n\t\t\tpriority:\t";
    if (mod.priority.first)
        qtout << QString::number(mod.priority.second);
    else
        qtout << "<dynamic>";
    qtout << "\n\t\t\tneedsParseForMatch:\t" << ((mod.needsParseForMatch) ? "true" : "false");
    qtout << "\n";
}

void TerminalPrinter::doPrintVars(const Room &room)
{
    for (auto it = room.variables.begin(); it != room.variables.end(); ++it) qtout << it.key() << "=" << it.value() << "\n";
}

void TerminalPrinter::doPrintFuncs(const Room &room)
{
    for (auto it = room.functions.begin(); it != room.functions.end(); ++it) {
        qtout << it.key() << "(" << it.value().first.front();
        for (auto it_args = it.value().first.begin() + 1; it_args != it.value().first.end(); ++it_args) qtout << "," << *it_args;
        qtout << ")=" << it.value().second << "\n";
    }
}

void TerminalPrinter::openRoom(const QString &name)
{
    if (roomByName.contains(name)) currentRoom = name;
    else qtout << "Enter the room before opening it.";
    qtout << "\n(" << currentRoom << ")->";
    qtout.flush();
}

 #include "TerminalPrinter.h"

TerminalPrinter::TerminalPrinter(JarvisClient &client) : client(client), qtout(stdout)
{
    connect(&client, SIGNAL(msgInRoom(const QString &, const QString &, const QString &)), SLOT(msgInRoom(const QString &, const QString &, const QString &)));
    connect(&client, SIGNAL(declaredFunction(const QString &, const FunctionSignature &, const QString &)), SLOT(declaredFunction(const QString &, const FunctionSignature &, const QString &)));
    connect(&client, SIGNAL(definedFunction(const QString &, const QString &, const QList<QPair<QString, QString>> &, const QString &)), SLOT(definedFunction(const QString &, const QString &, const QList<QPair<QString, QString>> &, const QString &)));
    connect(&client, SIGNAL(newRoom(const QString &)), SLOT(newRoom(const QString &)));
    connect(&client, SIGNAL(deletedRoom(const QString &)), SLOT(deletedRoom(const QString &)));
    connect(&client, SIGNAL(declaredVariable(const QString &, const QString &, const QString &)), SLOT(declaredVariable(const QString &, const QString &, const QString &)));
    connect(&client, SIGNAL(definedVariable(const QString &, const QString &, const QString &)), SLOT(definedVariable(const QString &, const QString &, const QString &)));
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

void TerminalPrinter::declaredFunction(const QString &room, const FunctionSignature &signature, const QString &returnType)
{
    qtout << "\nNew function declaration (room " << room << "):\t" << returnType << " " << signature.identifier << "(";
    if (! signature.argumentTypes.empty()) {
        for (auto it = signature.argumentTypes.begin(); it != signature.argumentTypes.end() - 1; ++it) qtout << *it << ", ";
        qtout << signature.argumentTypes.back();
    }
    qtout << ")";
    qtout.flush();
    roomByName[room].functions.insert(signature, FunctionDefinition(returnType));
}

void TerminalPrinter::definedFunction(const QString &room, const QString &identifier, const QList<QPair<QString, QString>> &arguments, const QString &definition)
{
    qtout << "\nNew function definition (room " << room << "):\t" << identifier << "(";
    QStringList argTypes, argStrings;
    for (auto itArg = arguments.begin(); itArg != arguments.end(); ++itArg) {
        argTypes.append(std::move(itArg->first));
        argStrings.append(std::move(itArg->second));
        qtout << itArg->first << " " << itArg->second;
        if (itArg != arguments.end() - 1) qtout << ", ";
    }
    qtout << ")=" << definition << "\n";
    qtout << "(" << currentRoom << ")->";
    qtout.flush();
    auto it = roomByName[room].functions.find(FunctionSignature{identifier, argTypes});
    it->argumentNames = std::move(argStrings);
    it->definition = std::move(definition);
}

void TerminalPrinter::declaredVariable(const QString &room, const QString &identifier, const QString &type)
{
    qtout << "\nNew variable declaration (room " << room << "):\t" << type << " " << identifier << "\n";
    qtout << "(" << currentRoom << ")->";
    qtout.flush();
    roomByName[room].variables.insert(identifier, qMakePair(type, QString()));
}

void TerminalPrinter::definedVariable(const QString &room, const QString &identifier, const QString &definition)
{
    qtout << "\nNew variable definition (room " << room << "):\t" << identifier << "=" << definition << "\n";
    qtout << "(" << currentRoom << ")->";
    qtout.flush();
    roomByName[room].variables[identifier].second = definition;
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
    for (auto it = room.variables.begin(); it != room.variables.end(); ++it) qtout << it.value().first << " " << it.key() << "=" << it.value().second << "\n";
}

void TerminalPrinter::doPrintFuncs(const Room &room)
{
    for (auto it = room.functions.begin(); it != room.functions.end(); ++it) {
        qtout << it.value().returnType << " " << it.key().identifier << "(";
        if (it.value().argumentNames.empty()) {
            if (! it.key().argumentTypes.empty()) {
                for (auto itArgType = it.key().argumentTypes.begin(); itArgType != it.key().argumentTypes.end() - 1; ++itArgType)
                    qtout << *itArgType << ", ";
                qtout << it.key().argumentTypes.back();
            }
        } else {
            auto itArgStrs = it.value().argumentNames.begin();
            for (const auto &argType : it.key().argumentTypes) {
                qtout << argType << " " << *(itArgStrs++);
                if (itArgStrs != it.value().argumentNames.end()) qtout << ", ";
            }
        }
        qtout << ")=" << it.value().definition << "\n";
    }
}

void TerminalPrinter::openRoom(const QString &name)
{
    if (roomByName.contains(name)) currentRoom = name;
    else qtout << "Enter the room before opening it.";
    qtout << "\n(" << currentRoom << ")->";
    qtout.flush();
}

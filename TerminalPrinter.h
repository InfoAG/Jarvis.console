#ifndef TERMINALPRINTER_H
#define TERMINALPRINTER_H

#include <QObject>
#include "JarvisClient.h"
#include "ModulePackage.h"
#include <QTextStream>

class TerminalPrinter : public QObject
{
    Q_OBJECT

private:
    JarvisClient &client;
    QTextStream qtout;
    QString currentScope;
    QStringList serverScopes;
    QMap<QString, Scope>  scopeByName;
    QList<ModulePackage> pkgs;
    void printPackage(const ModulePackage &pkg);

    void doPrintVars(const Scope &scope);
    void doPrintFuncs(const Scope &scope);

public:
    explicit TerminalPrinter(JarvisClient &client);
    
signals:
    
public slots:
    void newScope(const QString &name);
    void newFunction(const QString &scope, const QString &identifier, const QStringList &arguments, const QString &def);
    void newVariable(const QString &scope, const QString &identifier, const QString &definition);
    void newClient(const QString &scope, const QString &name);
    void clientLeft(const QString &scope, const QString &name);
    void msgInScope(const QString &scope, const QString &sender, const QString &msg);
    void error(JarvisClient::ClientError error);
    void pkgLoaded(const QVariant &pkg);
    void pkgUnloaded(const QString &name);
    void enteredScope(const QString &name, const QVariant &info);
    void receivedInitInfo(const QVariant &scopes, const QVariant &pkgs);
    void openScope(const QString &name);
    void printClients();
    void printModules();
    void leaveScope(const QString &name);
    void printScopes();
    void printVariables() { if (! currentScope.isEmpty()) doPrintVars(scopeByName[currentScope]); qtout << "(" << currentScope << ")->"; qtout.flush(); }
    void printFunctions() { if (! currentScope.isEmpty()) doPrintFuncs(scopeByName[currentScope]); qtout << "(" << currentScope << ")->"; qtout.flush(); }
    void msgToScope(const QString &msg) { if (! currentScope.isEmpty()) QMetaObject::invokeMethod(&client, "msgToScope", Q_ARG(QString, currentScope), Q_ARG(QString, msg)); }
    void deletedScope(const QString &name);
    void disconnected() { qtout << "\nDisconnected!\n"; qtout.flush(); }
    
};

#endif // TERMINALPRINTER_H

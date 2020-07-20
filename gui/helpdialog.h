#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>
#include <QTextBrowser>

namespace Ui {
    class HelpDialog;
}

class QHelpEngine;

class HelpBrowser : public QTextBrowser {
public:
    HelpBrowser(QWidget* parent = 0) : QTextBrowser(parent), mHelpEngine(nullptr) {}
    void setHelpEngine(QHelpEngine *helpEngine);
    QVariant loadResource(int type, const QUrl& name);
private:
    QHelpEngine* mHelpEngine;
};

class HelpDialog : public QDialog {
    Q_OBJECT

public:
    explicit HelpDialog(QWidget *parent = nullptr);
    ~HelpDialog();

private:
    Ui::HelpDialog *mUi;
    QHelpEngine* mHelpEngine;
};

#endif // HELPDIALOG_H

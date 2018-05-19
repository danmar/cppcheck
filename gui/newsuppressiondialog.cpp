#include "newsuppressiondialog.h"
#include "ui_newsuppressiondialog.h"
#include "cppcheck.h"
#include "errorlogger.h"


NewSuppressionDialog::NewSuppressionDialog(QWidget *parent) :
    QDialog(parent),
    mUI(new Ui::NewSuppressionDialog)
{
    mUI->setupUi(this);

    class QErrorLogger : public ErrorLogger {
    public:
        virtual void reportOut(const std::string &/*outmsg*/) {}
        virtual void reportErr(const ErrorLogger::ErrorMessage &msg) {
            errorIds << QString::fromStdString(msg._id);
        }
        QStringList errorIds;
    };

    QErrorLogger errorLogger;
    CppCheck cppcheck(errorLogger,false);
    cppcheck.getErrorMessages();
    errorLogger.errorIds.sort();

    mUI->mComboErrorId->addItems(errorLogger.errorIds);
    mUI->mComboErrorId->setCurrentIndex(-1);
    mUI->mComboErrorId->setCurrentText("");
}

NewSuppressionDialog::~NewSuppressionDialog()
{
    delete mUI;
}

Suppressions::Suppression NewSuppressionDialog::getSuppression() const
{
    Suppressions::Suppression ret;
    ret.errorId = mUI->mComboErrorId->currentText().toStdString();
    ret.fileName = mUI->mTextFileName->text().toStdString();
    ret.lineNumber = mUI->mTextLineNumber->text().toInt();
    ret.symbolName = mUI->mTextSymbolName->text().toStdString();
    return ret;
}

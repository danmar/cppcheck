#include "newsuppressiondialog.h"
#include "ui_newsuppressiondialog.h"
#include "cppcheck.h"
#include "errorlogger.h"
#include "suppressions.h"

NewSuppressionDialog::NewSuppressionDialog(QWidget *parent) :
    QDialog(parent),
    mUI(new Ui::NewSuppressionDialog)
{
    mUI->setupUi(this);

    class QErrorLogger : public ErrorLogger {
    public:
        void reportOut(const std::string &/*outmsg*/) override {}
        void reportErr(const ErrorMessage &msg) override {
            errorIds << QString::fromStdString(msg.id);
        }
        void bughuntingReport(const std::string &/*str*/) override {}
        QStringList errorIds;
    };

    QErrorLogger errorLogger;
    CppCheck cppcheck(errorLogger, false, nullptr);
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
    if (ret.errorId.empty())
        ret.errorId = "*";
    ret.fileName = mUI->mTextFileName->text().toStdString();
    if (!mUI->mTextLineNumber->text().isEmpty())
        ret.lineNumber = mUI->mTextLineNumber->text().toInt();
    ret.symbolName = mUI->mTextSymbolName->text().toStdString();
    return ret;
}

void NewSuppressionDialog::setSuppression(const Suppressions::Suppression &suppression)
{
    setWindowTitle(tr("Edit suppression"));
    mUI->mComboErrorId->setCurrentText(QString::fromStdString(suppression.errorId));
    mUI->mTextFileName->setText(QString::fromStdString(suppression.fileName));
    mUI->mTextLineNumber->setText(suppression.lineNumber > 0 ? QString::number(suppression.lineNumber) : QString());
    mUI->mTextSymbolName->setText(QString::fromStdString(suppression.symbolName));
}

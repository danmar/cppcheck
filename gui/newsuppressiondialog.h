#ifndef NEWSUPPRESSIONDIALOG_H
#define NEWSUPPRESSIONDIALOG_H

#include <QDialog>
#include "suppressions.h"

namespace Ui {
    class NewSuppressionDialog;
}

class NewSuppressionDialog : public QDialog {
    Q_OBJECT

public:
    explicit NewSuppressionDialog(QWidget *parent = 0);
    NewSuppressionDialog(const NewSuppressionDialog &) = delete;
    ~NewSuppressionDialog();
    NewSuppressionDialog &operator=(const NewSuppressionDialog &) = delete;

    Suppressions::Suppression getSuppression() const;

private:
    Ui::NewSuppressionDialog *mUI;
};

#endif // NEWSUPPRESSIONDIALOG_H

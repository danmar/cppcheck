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

    /**
     * @brief Translate the user input in the GUI into a suppression
     * @return Cppcheck suppression
     */
    Suppressions::Suppression getSuppression() const;

    /**
     * @brief Update the GUI so it corresponds with the given
     * Cppcheck suppression
     * @param suppression Cppcheck suppression
     */
    void setSuppression(const Suppressions::Suppression &suppression);

private:
    Ui::NewSuppressionDialog *mUI;
};

#endif // NEWSUPPRESSIONDIALOG_H


#ifndef selectfilesdialogH
#define selectfilesdialogH

#include <QDialog>

namespace Ui {
    class SelectFilesDialog;
}

class SelectFilesDialog : public QDialog {
public:
    explicit SelectFilesDialog(QWidget *w);
    ~SelectFilesDialog();

private:
    Ui::SelectFilesDialog *ui;
};

#endif



#ifndef selectfilesdialogH
#define selectfilesdialogH

#include <QDialog>
#include <QStringList>

namespace Ui {
    class SelectFilesDialog;
}

class SelectFilesModel;

class SelectFilesDialog : public QDialog {
public:
    explicit SelectFilesDialog(QWidget *w);
    ~SelectFilesDialog();

    QStringList getFiles() const;

private:
    Ui::SelectFilesDialog *ui;
    SelectFilesModel *selectfilesmodel;
};

#endif


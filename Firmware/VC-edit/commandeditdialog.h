#ifndef COMMANDEDITDIALOG_H
#define COMMANDEDITDIALOG_H

// The edit window for commands

#include "vccommands.h"
#include <QDialog>

namespace Ui {
class commandEditDialog;
}

class commandEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit commandEditDialog(QWidget *parent = 0, VCcommands *VCc = 0, int pg = 0, int sw = 0, int item = 0);
    ~commandEditDialog();

public slots:
    void ShowListWidgetContextMenu(const QPoint &pos);
    void updateEditCommandScreen();

private slots:
    // Actions in widgets
    void on_comboBoxPage_activated(int index);
    void on_comboBoxSwitch_activated(int index);
    void on_commandListWidget_currentRowChanged(int currentRow);
    void on_lineEditCustomLabel_textEdited(const QString &arg1);
    void on_lineEditCustomLabel_editingFinished();

    // Pressing buttons
    void on_pushButtonNewCommand_clicked();
    void on_pushButtonDeleteCommand_clicked();
    void on_pushButtonClose_clicked();
    void on_toolButtonMoveUp_clicked();
    void on_toolButtonMoveDown_clicked();

protected:
    // To catch arrow up and down from lineEditCustomLabel, we need an eventFilter
    bool eventFilter(QObject* obj, QEvent *event);

private:
    Ui::commandEditDialog *ui;
    void loadWindowSettings();
    void saveWindowSettings();
    void buildPage(bool first_time);
    void updateUpDownButtons();
    void checkFocus();

    VCcommands *MyVCcommands;
    int currentPage = 0;
    int currentSwitch = 0;
    int currentItem = 0;
    bool rebuilding = false;
    bool labelEdited = false;
};

#endif // COMMANDEDITDIALOG_H

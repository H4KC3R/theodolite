#ifndef DIALOG_H
#define DIALOG_H
#include <QDialog>

struct starThdData
{
QString cirle;
QString MeasureObj;
QString Hz_angle;
QString V_angle;
};


namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
    void getData();

private:
    Ui::Dialog *ui;
signals:
    void starThdDataReady(starThdData starthd_data);
};

#endif // DIALOG_H

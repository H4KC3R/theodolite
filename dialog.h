#ifndef DIALOG_H
#define DIALOG_H
#include <QDialog>
#include <QRegularExpression>

struct StarThdData
{
    QString cirle;
    QString MeasureObj;
    QString HzAngle;
    QString VAngle;
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
    void starThdDataReady(StarThdData starthdData);
};

#endif // DIALOG_H

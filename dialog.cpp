#include "dialog.h"
#include "ui_dialog.h"


Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    QStringList starThdCircle;
    starThdCircle << "КРУГ(КЛ)" << "КРУГ(КП)";
    ui->CircleComboBox->insertItems(0, starThdCircle);
    QStringList starThdMeasure;
    starThdMeasure << "Прибор" << "Теодолит №1" << "Теодолит №2";
    ui->MeasureObjComboBox->insertItems(0, starThdMeasure);



    QRegExp rxAngle("[0-3]{1}[0-9]{2} [0-9]{2} [0-9]{2}");
    QValidator *gmsValidator = new QRegExpValidator(rxAngle, this);

    ui->HzAngleLineEdit->setValidator(gmsValidator);
    ui->VAngleLineEdit->setValidator(gmsValidator);


    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &Dialog::getData);
}


void Dialog::getData()
{
    StarThdData thdData;
    thdData.cirle = ui->CircleComboBox->currentText();
    thdData.MeasureObj= ui->MeasureObjComboBox->currentText();
    thdData.HzAngle = ui->HzAngleLineEdit->text();
    thdData.VAngle = ui->VAngleLineEdit->text();
    emit starThdDataReady(thdData);
}
Dialog::~Dialog()
{
    delete ui;
}

#include "dialog.h"
#include "ui_dialog.h"


Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    QStringList star_thd_circle;
    star_thd_circle << "КРУГ(КЛ)" << "КРУГ(КП)";
    ui->CircleComboBox->insertItems(0, star_thd_circle);
    QStringList star_thd_measure;
    star_thd_measure << "Прибор" << "Теодолит №1" << "Теодолит №2";
    ui->MeasureObjComboBox->insertItems(0, star_thd_measure);



    QRegExp rxAngle("[0-3]{1}[0-9]{2} [0-9]{2} [0-9]{2}");
    QValidator *gms_validator = new QRegExpValidator(rxAngle, this);

    ui->HzAngleLineEdit->setValidator(gms_validator);
    ui->VAngleLineEdit->setValidator(gms_validator);


    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &Dialog::getData);
}


void Dialog::getData()
{
    StarThdData thd_data;
    thd_data.cirle = ui->CircleComboBox->currentText();
    thd_data.MeasureObj= ui->MeasureObjComboBox->currentText();
    thd_data.Hz_angle = ui->HzAngleLineEdit->text();
    thd_data.V_angle = ui->VAngleLineEdit->text();
    emit starThdDataReady(thd_data);
}
Dialog::~Dialog()
{
    delete ui;
}

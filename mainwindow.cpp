#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <meanskofcn.h>






MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    qRegisterMetaType<DataFromThread>();
    settings= new QSettings(QDir::currentPath()+"/"+"config.ini",QSettings::IniFormat,this);

    setStyle();


    initializeValidators();
    initializeConnectionTable();
    initializeComboBoxes();

    theodolite_1.reset(new Theodolite());
    theodolite_1->setObjectName("theodolite_1");
    if(!theodolite_1->connectLibrary("GeoComS2K.dll"))
    {
        ui->textEdit->append("Неверно задан путь к библиотеке Geocom\n");
    }

    theodolite_2.reset(new Theodolite());
    theodolite_2->setObjectName("theodolite_2");
    if(!theodolite_2->connectLibrary("GeoComS2K2.dll"))
    {
        ui->textEdit_2->append("Неверно задан путь к библиотеке Geocom\n");
    }

    starThdDialog = new Dialog(this);


    connect(ui->openDirectoryButton,&QPushButton::clicked,this,&MainWindow::on_openDirectoryButton_clicked);
    connect(ui->chooseDirectoryButton,&QPushButton::clicked,this,&MainWindow::on_chooseDirectoryButton_clicked);
    connect(ui->startMeasuresButton,&QPushButton::clicked,this,&MainWindow::on_startMeasuresButton_clicked);
    connect(ui->endMeasuresButton,&QPushButton::clicked,this,&MainWindow::on_endMeasuresButton_clicked);

    connect(ui->ConnectButton,&QPushButton::clicked,this,&MainWindow::connectTheodolit);
    connect(ui->ConnectButton_2,&QPushButton::clicked,this,&MainWindow::connectTheodolit);


    connect(ui->resetHzPushButton,&QPushButton::clicked,this,&MainWindow::resetOrientationHandler);
    connect(ui->resetHzPushButton_2,&QPushButton::clicked,this,&MainWindow::resetOrientationHandler);

    connect(theodolite_1.data(),&Theodolite::connected,this,&MainWindow::reportAboutConnection);
    connect(theodolite_2.data(),&Theodolite::connected,this,&MainWindow::reportAboutConnection);


    connect(theodolite_1.data(),&Theodolite::error_report,this,&MainWindow::reportAboutError);
    connect(theodolite_2.data(),&Theodolite::error_report,this,&MainWindow::reportAboutError);

    connect(ui->starThdButton,&QPushButton::clicked,this,&MainWindow::on_starThdButton_clicked);

    connect(ui->mesButton, &QPushButton::clicked,this, &MainWindow::makeMeasureNoSave);
    connect(ui->mesButton_2, &QPushButton::clicked,this, &MainWindow::makeMeasureNoSave);


    connect(ui->mesAndSaveButton,&QPushButton::clicked,this,&MainWindow::makeMeasureAndSave);
    connect(ui->mesAndSaveButton_2,&QPushButton::clicked,this,&MainWindow::makeMeasureAndSave);

    connect(this,&MainWindow::readyDataFromThread,this,&MainWindow::dataFromThreadHandler,Qt::QueuedConnection);
    connect(ui->parallelMesPushButton,&QPushButton::clicked,this,&MainWindow::parallelHandlerNosave);
    connect(ui->parallelMesAndSaveButton,&QPushButton::clicked,this,&MainWindow::parallelHandlerDosave);

    connect(starThdDialog,&Dialog::starThdDataReady,this,&MainWindow::saveStarThdData);

    connect(ui->parallelResetHzPushButton,&QPushButton::clicked,this,&MainWindow::parallelResetOrientation);
    connect(ui->chooseFilePushButton,&QPushButton::clicked,this,&MainWindow::on_chooseFilePushButton_clicked);
    connect(ui->performPushButton,&QPushButton::clicked,this,&MainWindow::on_performPushButton_clicked);
    connect(ui->nextPushButton,&QPushButton::clicked,this,&MainWindow::on_nextPushButton_clicked);

    loadSettings();

}


void MainWindow::initializeValidators()
{
    QRegExp rxAngle("[0-3]{1}[0-9]{2} [0-9]{2} [0-9]{2}");
    QValidator *gms_validator = new QRegExpValidator(rxAngle, this);
    ui->HzAngleValueLineEdit->setValidator(gms_validator);
    ui->HzAngleValueLineEdit_2->setValidator(gms_validator);

}

void MainWindow::setStyle()
{
    QFile file(":/sources/style.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        qApp->setStyleSheet(stream.readAll());
        file.close();
    }
}

void MainWindow::initializeComboBoxes()
{
    QStringList report_list_thd1;
    QStringList report_list_thd2;

    report_list_thd1<<"Теодолит №2"<<"Компенсатор"<<"Теодолит Зв"<<"Куб прибора А"<<"Куб прибора Б"<<"Куб прибора В"<<"Куб прибора Г"
                   <<"Куб прибора Д"<<"Куб стенда А"<<"Куб стенда Б"<<"Куб стенда В"<<"Куб стенда Г"
                  <<"Куб стенда Д"<<"Лекальную линейку"<<"Плоское зеркало";

    report_list_thd2<<"Теодолит №1"<<"Компенсатор"<<"Теодолит Зв"<<"Куб прибора А"<<"Куб прибора Б"<<"Куб прибора В"<<"Куб прибора Г"
                   <<"Куб прибора Д"<<"Куб стенда А"<<"Куб стенда Б"<<"Куб стенда В"<<"Куб стенда Г"
                  <<"Куб стенда Д"<<"Лекальную линейку"<<"Плоское зеркало";

    ui->ReportComboBox->insertItems(0, report_list_thd1);
    ui->ReportComboBox_2->insertItems(0, report_list_thd2);

    QStringList comPorts;
    for(qint32 i = COM_1;i <= COM_24;i ++)
    {
        comPorts.append(QString("COM_%1").arg(i + 1));
    }
    ui->comPortComboBox->insertItems(0, comPorts);
    ui->comPortComboBox_2->insertItems(0, comPorts);
}

void MainWindow::initializeConnectionTable()
{
    ui->tableWidget->setItem(0,0, new QTableWidgetItem);
    ui->tableWidget->setItem(0,1, new QTableWidgetItem);
    ui->tableWidget->item(0,0)->setText("Отключен");
    ui->tableWidget->item(0,0)->setBackground(QColor(255,0,0,70));
    ui->tableWidget->item(0,1)->setText("Отключен");
    ui->tableWidget->item(0,1)->setBackground(QColor(255,0,0,70));
}

void MainWindow::saveSettings()
{
    settings->setValue("directory_name",directory_name);
    settings->setValue("thd1_inclination",ui->HzAngleValueLineEdit->text());
    settings->setValue("thd2_inclination",ui->HzAngleValueLineEdit_2->text());
    settings->setValue("check_compensator",ui->checkCompensator->isChecked());
    settings->setValue("thd1_port",ui->comPortComboBox->currentIndex());
    settings->setValue("thd2_port",ui->comPortComboBox_2->currentIndex());
    settings->sync();
}



void MainWindow::loadSettings()
{
    directory_name = settings->value("directory_name","").toString();
    ui->directoryNameLineEdit->setText(settings->value("directory_name","Директория не выбрана...").toString());
    ui->HzAngleValueLineEdit->setText(settings->value("thd1_inclination","000 00 00").toString());
    ui->HzAngleValueLineEdit_2->setText(settings->value("thd2_inclination","000 00 00").toString());
    ui->checkCompensator->setChecked(settings->value("check_compensator","true").toBool());
    ui->comPortComboBox->setCurrentIndex(settings->value("thd1_port","3").toInt());
    ui->comPortComboBox_2->setCurrentIndex(settings->value("thd2_port","7").toInt());
}




void MainWindow::resetOrientation(QSharedPointer<Theodolite> theodolite, QLineEdit* getAngleLineEdit, QTextEdit* theodolitesTextEdit)
{
    if(!theodolite->isConnected())
    {
        theodolitesTextEdit->append("Теодолит не подключен");
        return;
    }
    QString gms_angle = getAngleLineEdit->text();
    if(!GMS::checkStringGMSAngle(gms_angle))
    {
        theodolitesTextEdit->append("Угол введен неверно.");
        return;
    }
    QStringList split_gms = gms_angle.split(" ");
    GMS trans_angle;
    trans_angle.setGradus(split_gms.at(0).toInt());
    trans_angle.setMinutes(split_gms.at(1).toInt());
    trans_angle.setSeconds(split_gms.at(2).toDouble());

    double orientation_angle = trans_angle.transToRad();
    theodolite->resetHz(orientation_angle);


}

void MainWindow::parallelResetOrientation()
{
    std::thread thd1(&MainWindow::resetOrientation,this,theodolite_1,ui->HzAngleValueLineEdit,ui->textEdit);
    thd1.detach();
    std::thread thd2(&MainWindow::resetOrientation,this,theodolite_2,ui->HzAngleValueLineEdit_2,ui->textEdit_2);
    thd2.detach();
    notePerformOfOperation(RESET_HZ_PARRALEL);
}


void MainWindow::resetOrientationHandler()
{
    QPushButton* pb = dynamic_cast<QPushButton*>(sender());
    if(!pb->objectName().contains("2"))
    {
        resetOrientation(theodolite_1,ui->HzAngleValueLineEdit,ui->textEdit);
        notePerformOfOperation(RESET_HZ_FIRST_THD);
        return;

    }
    else
    {
        resetOrientation(theodolite_2,ui->HzAngleValueLineEdit_2,ui->textEdit_2);
        notePerformOfOperation(RESET_HZ_SECOND_THD);
    }

}


void MainWindow::on_chooseDirectoryButton_clicked()
{
    directory_name = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                       "/home",
                                                       QFileDialog::ShowDirsOnly
                                                       | QFileDialog::DontResolveSymlinks);
    ui->directoryNameLineEdit->setText(directory_name);

}

void MainWindow::on_openDirectoryButton_clicked()
{
    auto url = QUrl::fromLocalFile(directory_name);
    QDesktopServices::openUrl(url);
}





void MainWindow::on_startMeasuresButton_clicked()
{
    if(directory_name.isEmpty())
    {
        ui->textEdit->append("Директория не выбрана!");
        ui->textEdit_2->append("Директория не выбрана!");
        return;
    }

    QFile directory (directory_name);
    if(!directory.exists())
    {
        ui->textEdit->append("Директория не существует!");
        ui->textEdit_2->append("Директория не существует!");
        return;
    }

    if(measures_started)
    {
        ui->textEdit->append("Завершите предыдущие измерения прежде чем начать новые");
        ui->textEdit_2->append("Завершите предыдущие измерения прежде чем начать новые");
        return;
    }
    ui->textEdit->clear();
    ui->textEdit_2->clear();


    QDate current_date = QDate::currentDate();
    QTime current_time = QTime::currentTime();
    report_filename = "/"+current_date.toString("yy.MM.dd")+"_"+current_time.toString("hh-mm-ss")+"_Measures"+".txt";
    protocol_filename = "/"+current_date.toString("yy.MM.dd")+"_"+current_time.toString("hh-mm-ss")+"_Protocol"+".txt";
    measures_started = true;
    ui->textEdit->append("Измерения начаты\n");
    ui->textEdit_2->append("Измерения начаты\n");
}


/*Измерения с записью в файл окончены*/
void MainWindow::on_endMeasuresButton_clicked()
{
    measures_started = false;
    ui->textEdit->append("Измерения закончены\n");
    num_of_measures = 0;

    QFile data(directory_name + protocol_filename);
    if (data.open(QFile::WriteOnly | QFile::Append | QIODevice::Text))
    {
        QTextStream out(&data);
        out<<ui->textEdit->toPlainText();
        out<<ui->textEdit_2->toPlainText();
    }
    report_filename.clear();
    protocol_filename.clear();
}




void MainWindow::on_starThdButton_clicked()
{
    starThdDialog->exec();
}



void MainWindow::reportAboutConnection(QString message)
{

    if(sender()->objectName() == theodolite_1->objectName())
    {

        ui->textEdit->append(message);
        ui->textEdit->append("Теодолит №1 успешно подключен\n");
        ui->tableWidget->item(0,0)->setBackground(QColor(0,255,0,70));
        ui->tableWidget->item(0,0)->setText("Подключен");
        return;
    }
    else
    {
        ui->textEdit_2->append(message);
        ui->textEdit_2->append("Теодолит №2 успешно подключен\n");
        ui->tableWidget->item(0,1)->setBackground(QColor(0,255,0,70));
        ui->tableWidget->item(0,1)->setText("Подключен");
    }



}




void MainWindow::reportAboutError(QString er_message)
{
    if(sender()->objectName() == theodolite_1->objectName())
    {
        ui->textEdit->append("Теодолит №1: произошла ошибка\n");
        ui->textEdit->append(er_message);
        return;
    }
    else
    {
        ui->textEdit_2->append("Теодолит №2: произошла ошибка\n");
        ui->textEdit_2->append(er_message);
    }

}





void MainWindow::saveStarThdData(starThdData Star_data)
{
    if(!measures_started)
    {
        QMessageBox::information(NULL,"Ошибка","Измерения не начаты");
        return;
    }
    QString Hz_angle = Star_data.Hz_angle;
    QString V_angle = Star_data.V_angle;

    if(!GMS::checkStringGMSAngle(Hz_angle) || !GMS::checkStringGMSAngle(V_angle))
    {
        QMessageBox::information(NULL,"Ошибка","Углы заданы неверно");
        return;
    }
    QStringList split_Hz_angle=Hz_angle.split(" ");
    QStringList split_V_angle=V_angle.split(" ");



    GMS Hz_angle_gms;
    Hz_angle_gms.setGradus(split_Hz_angle.at(0).toInt());
    Hz_angle_gms.setMinutes(split_Hz_angle.at(1).toInt());
    Hz_angle_gms.setSeconds(split_Hz_angle.at(2).toDouble());

    GMS V_angle_gms;
    V_angle_gms.setGradus(split_V_angle.at(0).toDouble());
    V_angle_gms.setMinutes(split_V_angle.at(1).toDouble());
    V_angle_gms.setSeconds(split_V_angle.at(2).toDouble());


    double hz_angle_grad = Hz_angle_gms.transToGrad();
    double v_angle_grad = V_angle_gms.transToGrad();
    const QString protocol_text = QString::number(num_of_measures++)+"\t"+tr("Зв.Теодолит")+"\t"+Star_data.cirle
            +"\t"+Star_data.MeasureObj+"\t"+zeroPadding(hz_angle_grad,10)+"\t"+zeroPadding(v_angle_grad,10)+"\t"
            +zeroPadding(Hz_angle_gms.getGradus())+" "+QString::number(Hz_angle_gms.getMinutes())+" "+QString::number(Hz_angle_gms.getSeconds(),'f',3)+"\t"
            +zeroPadding(V_angle_gms.getGradus())+" "+QString::number(V_angle_gms.getMinutes())+" "+QString::number(V_angle_gms.getSeconds(),'f',3)+"\t"+"\n";

    saveProtocolText(protocol_text);
    notePerformOfOperation(MEASURE_STAR_THD);


}

void MainWindow::connectTheodolit()
{
    QPushButton *pb = dynamic_cast<QPushButton*>(sender());
    if(!pb->objectName().contains("2"))
    {
        if(theodolite_1->isConnected()) return;

        else
        {
            COM_PORT thd1_port = static_cast <COM_PORT> (ui->comPortComboBox->currentIndex());
            if(!theodolite_1->enableConnection(thd1_port))
            {
                ui->textEdit->append("Подключение не удалось\n");
            }
        }
    }
    else
    {
        if(theodolite_2->isConnected())  return;

        else
        {
            COM_PORT thd2_port = static_cast <COM_PORT> (ui->comPortComboBox_2->currentIndex());
            if(!theodolite_2->enableConnection(thd2_port))
            {
                ui->textEdit_2->append("Подключение не удалось\n");
            }
        }
    }
}


MeasureCharacteristics MainWindow::calculateMeasureCharact(MeasuresFromTheodolite measure_vector)
{
    MeasureCharacteristics meas_chrc;
    auto min_max_pair = std::minmax_element(measure_vector.begin(),measure_vector.end(),
                                            [](auto &a,auto &b){return a.getHzAngle() < b.getHzAngle();});

    // если находимся в области нуля(т.е разность между минимальным и максимальным углом),
    // переопределяем углы через asin(sin()), считаем среднее по "Mean of circular quantities"
    const int max_scatter = 356;
    if((*min_max_pair.second).getHzAngle() - (*min_max_pair.first).getHzAngle() > max_scatter)
    {

        meas_chrc.EV_hz = meanSkoFcn::calc_circle_mean(measure_vector,meanSkoFcn::HZ_A);
        meas_chrc.sko_hz = meanSkoFcn::calc_circle_sko(measure_vector,meas_chrc.EV_hz,meanSkoFcn::HZ_A);
        if(meas_chrc.EV_hz < 0) meas_chrc.EV_hz = 360.0+meas_chrc.EV_hz;
    }
    else
    {
        meas_chrc.EV_hz = meanSkoFcn::calc_mean(measure_vector,meanSkoFcn::HZ_A);
        meas_chrc.sko_hz = meanSkoFcn::calc_sko(measure_vector,meas_chrc.EV_hz,meanSkoFcn::HZ_A);
    }

    meas_chrc.EV_v = meanSkoFcn::calc_mean(measure_vector,meanSkoFcn::V_A);
    meas_chrc.sko_v = meanSkoFcn::calc_sko(measure_vector,meas_chrc.EV_v,meanSkoFcn::V_A);

    return meas_chrc;

}

void MainWindow::makeMeasure(bool do_save,QSharedPointer<Theodolite> theodolite,QTextEdit* text_edit,QComboBox* combo_box
                             ,QLineEdit* meanHz_line_edit,QLineEdit* meanV_line_edit,QLineEdit* skoHz_line_edit
                             ,QLineEdit* skoV_line_edit,QLineEdit* SKO3xHz_line_edit,QLineEdit* SKO3xV_line_edit)

{
    try
    {
        connectTheodolit();
        if(!theodolite->isConnected()) return;

        quint16 count_of_measures = ui->MesCountSpinBox->value();
        if(!count_of_measures)
        {
            text_edit->append("Задано неверное число измерений\n");
            return;
        }

        double limit_SKOHz = ui->SKOHzLimitSpinBox->value();
        if(!limit_SKOHz)
        {
            text_edit->append("Не задано предельное СКО по горизонтали\n");
            return;
        }
        double limit_SKOV = ui->SKOVLimitSpinBox->value();
        if(!limit_SKOV)
        {
            text_edit->append("Не задано предельное СКО по вертикали\n");
            return;
        }


        if(ui->checkCompensator->isChecked())
        {

            if(!ui->IncliationSKOSpinBox->value())
            {
                text_edit->append("Не задано предельное СКО наклона\n");
                return;
            }

            if(!theodolite->checkCompensator(ui->IncliationSKOSpinBox->value()))
            {
                QApplication::beep();
                text_edit->append("Превышено предельное СКО наклона\n");
                return;
            }

        }
        text_edit->append("Начинаю измерения\n");



        MeasuresFromTheodolite measure_vector;

        if(combo_box->currentText() == "Компенсатор")
        {
            measure_vector = theodolite->startMeasures(count_of_measures,THD_INCL);
        }
        else
        {
            measure_vector = theodolite->startMeasures(count_of_measures,THD_MES);
        }


        for(auto&i:measure_vector)
        {
            GMS first_angle(i.getHzAngle());
            GMS second_angle(i.getVAngle());
            text_edit->append(QString("%1").arg(QString::number(i.getHzAngle(),'f',10),-20,' ')+"\t"+QString::number(first_angle.getGradus())+" "
                              +QString::number(first_angle.getMinutes())+" "+
                              QString::number(first_angle.getSeconds(),'f',3)+"\n");
            text_edit->append(QString("%1").arg(QString::number(i.getVAngle(),'f',10),-20,' ')+"\t"+QString::number(second_angle.getGradus())+" "
                              +QString::number(second_angle.getMinutes())+" "+
                              QString::number(second_angle.getSeconds(),'f',3)+"\n");
        }

        text_edit->append("FACE: "+QString::number(measure_vector[0].getCircle()));
        text_edit->append("Число некорректных измерений: "+QString::number(count_of_measures-measure_vector.size())+"\n");

        MeasureCharacteristics meas_chrc = calculateMeasureCharact(measure_vector);
        GMS sko_hz_angle(meas_chrc.sko_hz);
        GMS sko_v_angle(meas_chrc.sko_v);
        GMS EV_hz_angle(meas_chrc.EV_hz);
        GMS EV_v_angle(meas_chrc.EV_v);



        bool sko_limit = false;
        if(sko_hz_angle.getSeconds() > limit_SKOHz || sko_v_angle.getSeconds() > limit_SKOV)
        {
            QApplication::beep();
            text_edit->append("Внимание! Превышено предельное СКО\n");
            sko_limit = true;
        }


        meanHz_line_edit->setText(QString::number(EV_hz_angle.getGradus())+" "+QString::number(EV_hz_angle.getMinutes())+" "+QString::number(EV_hz_angle.getSeconds(),'f',3));
        meanV_line_edit->setText(QString::number(EV_v_angle.getGradus())+" "+QString::number(EV_v_angle.getMinutes())+" "+QString::number(EV_v_angle.getSeconds(),'f',3));
        skoHz_line_edit->setText(QString::number(sko_hz_angle.getSeconds(),'f',3));
        skoV_line_edit->setText(QString::number(sko_v_angle.getSeconds(),'f',3));
        SKO3xHz_line_edit->setText(QString::number(3*sko_hz_angle.getSeconds(),'f',3));
        SKO3xV_line_edit->setText(QString::number(3*sko_v_angle.getSeconds(),'f',3));

        auto getThdName = [&](){
            return theodolite->objectName() == (theodolite_1->objectName()) ? "Теодолит №1": "Теодолит №2";
        };

        const QString protocol_text = QString::number(num_of_measures++)+"\t"+tr(getThdName())+"\t"+"КРУГ("+QString::number(measure_vector[0].getCircle())+")\t"
                +combo_box->currentText()+"\t"+zeroPadding(meas_chrc.EV_hz,10)+"\t"+zeroPadding(meas_chrc.EV_v,10)+"\t"
                +zeroPadding(EV_hz_angle.getGradus())+" "+QString::number(EV_hz_angle.getMinutes())+" "+QString::number(EV_hz_angle.getSeconds(),'f',3)+"\t"
                +zeroPadding(EV_v_angle.getGradus())+" "+QString::number(EV_v_angle.getMinutes())+" "+QString::number(EV_v_angle.getSeconds(),'f',3)+"\t"
                +zeroPadding(sko_hz_angle.getSeconds(),3)+"\t"+zeroPadding(sko_v_angle.getSeconds(),3)+"\n";


        if(!sko_limit&&measures_started&&do_save)
        {
            saveProtocolText(protocol_text);
            notePerformOfOperation(MEASURE_ONE_THD);

        }

    }

    catch(std::exception& e)
    {
        QMessageBox::information(NULL,"Ошибка",e.what());
        return;
    }
    catch(...)
    {
        QMessageBox::information(NULL,"Ошибка","Неизвестная ошибка");
        return;
    }
}



void MainWindow::makeMeasureNoSave()
{
    QPushButton* pb = dynamic_cast<QPushButton*>(sender());
    if(!pb->objectName().contains("2"))
    {
        makeMeasure(false,theodolite_1,ui->textEdit,ui->ReportComboBox,ui->meanHzLineEdit,ui->meanVLineEdit,
                    ui->SKOHzLineEdit,ui->SKOVLineEdit,ui->SKO3xHzLineEdit,ui->SKO3xVLineEdit);
    }
    else
    {
        makeMeasure(false,theodolite_2,ui->textEdit_2,ui->ReportComboBox_2,ui->meanHzLineEdit_2,ui->meanVLineEdit_2,
                    ui->SKOHzLineEdit_2,ui->SKOVLineEdit_2,ui->SKO3xHzLineEdit_2,ui->SKO3xVLineEdit_2);
    }

}


void MainWindow::makeMeasureAndSave()
{
    if(!measures_started)
    {
        ui->textEdit->append("Измерения не начаты.");
        ui->textEdit_2->append("Измерения не начаты.");
        return;
    }
    QPushButton* pb = dynamic_cast<QPushButton*>(sender());
    if(!pb->objectName().contains("2"))
    {
        makeMeasure(true,theodolite_1,ui->textEdit,ui->ReportComboBox,ui->meanHzLineEdit,ui->meanVLineEdit,
                    ui->SKOHzLineEdit,ui->SKOVLineEdit,ui->SKO3xHzLineEdit,ui->SKO3xVLineEdit);
        return;
    }
    makeMeasure(true,theodolite_2,ui->textEdit_2,ui->ReportComboBox_2,ui->meanHzLineEdit_2,ui->meanVLineEdit_2,
                ui->SKOHzLineEdit_2,ui->SKOVLineEdit_2,ui->SKO3xHzLineEdit_2,ui->SKO3xVLineEdit_2);

}




void MainWindow::parallelHandlerNosave()
{
    if(prepareParralelMSeasure())
    {
        parallelConnection();
        if(theodolite_1->isConnected() && theodolite_2->isConnected())
        {
            std::thread thd1(&MainWindow::parallelMakeMeasureUncheck,this,theodolite_1,ui->ReportComboBox,ui->MesCountSpinBox->value(), false);
            thd1.detach();
            std::thread thd2(&MainWindow::parallelMakeMeasureUncheck,this,theodolite_2,ui->ReportComboBox_2,ui->MesCountSpinBox->value(),false);
            thd2.detach();
        }
    }

}


void MainWindow::parallelHandlerDosave()
{
    if(!measures_started)
    {
        ui->textEdit->append("Измерения не начаты.");
        ui->textEdit_2->append("Измерения не начаты.");
        return;
    }

    if(prepareParralelMSeasure()) return;


    parallelConnection();
    if(!theodolite_1->isConnected() || !theodolite_2->isConnected()) return;

    auto thd1_future = std::async(std::launch::async,&MainWindow::parralelMakeMeasureCheck,this,theodolite_1,ui->ReportComboBox,ui->MesCountSpinBox->value());
    auto thd2_future = std::async(std::launch::async,&MainWindow::parralelMakeMeasureCheck,this,theodolite_2,ui->ReportComboBox_2,ui->MesCountSpinBox->value());

    DFTP thd1_data = thd1_future.get();
    DFTP thd2_data = thd2_future.get();

    if(!thd1_data.isEmpty() && !thd2_data.isEmpty())
    {
        double limit_SKOHz = ui->SKOHzLimitSpinBox->value();
        double limit_SKOV = ui->SKOVLimitSpinBox->value();

        if(thd1_data.sko_hz.getSeconds() < limit_SKOHz && thd2_data.sko_hz.getSeconds() < limit_SKOHz)
        {
            if(thd1_data.sko_v.getSeconds() < limit_SKOV && thd2_data.sko_v.getSeconds() < limit_SKOV)
            {
                saveProtocolText(thd1_data.protocol_text);
                saveProtocolText(thd2_data.protocol_text);
                notePerformOfOperation(MEASURE_PARRALEL);

            }
        }

    }





}

/*так как работать с УИ в потоке нехорошо, сигналим данные для него в обработчик, который вызывает функцию, которая эти данные на УИ
выведет*/
void MainWindow::dataFromThreadHandler(const DFT &data)
{
    if(data.device_name == theodolite_1->objectName())
    {

        setDataFromThread(data,ui->textEdit,ui->meanHzLineEdit,ui->meanVLineEdit,
                          ui->SKOHzLineEdit,ui->SKOVLineEdit,ui->SKO3xHzLineEdit,ui->SKO3xVLineEdit);
    }
    else
    {
        setDataFromThread(data,ui->textEdit_2,ui->meanHzLineEdit_2,ui->meanVLineEdit_2,
                          ui->SKOHzLineEdit_2,ui->SKOVLineEdit_2,ui->SKO3xHzLineEdit_2,ui->SKO3xVLineEdit_2);
    }
}




void MainWindow::setDataFromThread(const DFT& data,QTextEdit* text_edit,
                                   QLineEdit* meanHz_line_edit,QLineEdit* meanV_line_edit,QLineEdit* skoHz_line_edit
                                   ,QLineEdit* skoV_line_edit,QLineEdit*SKO3xHz_line_edit,QLineEdit*SKO3xV_line_edit)
{
    /*если превышено ско компенсатора, то сюда придёт структура с пустыми данными об измерениях*/
    if(data.notContainsMeasures())
    {
        text_edit->append("Превышено СКО компенсатора\n");
        return;
    }
    for(auto&i:data.measure_vector)
    {
        GMS first_angle(i.getHzAngle());
        GMS second_angle(i.getVAngle());
        text_edit->append(QString("%1").arg(QString::number(i.getHzAngle(),'f',10),-20,' ')+"\t"+QString::number(first_angle.getGradus())+" "
                          +QString::number(first_angle.getMinutes())+" "+
                          QString::number(first_angle.getSeconds(),'f',3)+"\n");
        text_edit->append(QString("%1").arg(QString::number(i.getVAngle(),'f',10),-20,' ')+"\t"+QString::number(second_angle.getGradus())+" "
                          +QString::number(second_angle.getMinutes())+" "+
                          QString::number(second_angle.getSeconds(),'f',3)+"\n");
    }



    text_edit->append("FACE: "+QString::number(data.measure_vector[0].getCircle()));
    text_edit->append("Число некорректных измерений: "+QString::number(ui->MesCountSpinBox->value()-data.measure_vector.size())+"\n");

    if(data.sko_Hz.getSeconds()>ui->SKOHzLimitSpinBox->value() || data.sko_V.getSeconds()>ui->SKOVLimitSpinBox->value())
    {
        text_edit->append("Внимание! Превышено предельное СКО\n");
    }


    meanHz_line_edit->setText(QString::number(data.mean_Hz.getGradus())+" "+QString::number(data.mean_Hz.getMinutes())+" "+QString::number(data.mean_Hz.getSeconds(),'f',3));
    meanV_line_edit->setText(QString::number(data.mean_V.getGradus())+" "+QString::number(data.mean_V.getMinutes())+" "+QString::number(data.mean_V.getSeconds(),'f',3));
    skoHz_line_edit->setText(QString::number(data.sko_Hz.getSeconds(),'f',3));
    skoV_line_edit->setText(QString::number(data.sko_V.getSeconds(),'f',3));
    SKO3xHz_line_edit->setText(QString::number(3*data.sko_Hz.getSeconds(),'f',3));
    SKO3xV_line_edit->setText(QString::number(3*data.sko_V.getSeconds(),'f',3));
    if(!measures_started)
    {
        text_edit->append("Внимание! Измерения не сохранены, т.к они не были начаты\n");
    }
}





void MainWindow::parallelConnection()
{
    if(theodolite_1->isConnected() && theodolite_2->isConnected())
    {
        return;
    }


    else
    {
        if(!theodolite_1->isConnected())
        {
            if(!theodolite_1->enableConnection(COM_13))
            {
                ui->textEdit->append("Подключение не удалось\n");
                return;
            }
        }

        if(!theodolite_2->isConnected())
        {
            if(!theodolite_2->enableConnection(COM_12))
            {
                ui->textEdit_2->append("Подключение не удалось\n");
                return;
            }
        }
    }
}

bool MainWindow::prepareParralelMSeasure()
{
    const int needed_count_of_cores = 3;
    if(std::thread::hardware_concurrency() < needed_count_of_cores)
    {
        ui->textEdit->append("Данный процессор не позволяет произвести параллельные измерения");
        ui->textEdit_2->append("Данный процессор не позволяет произвести параллельные измерения\n");
        return false;
    }

    if(!ui->SKOHzLimitSpinBox->value())
    {
        ui->textEdit->append("Не задано предельное СКО по горизонтали\n");
        ui->textEdit_2->append("Не задано предельное СКО по горизонтали\n");
        return false;
    }

    if(!ui->SKOVLimitSpinBox->value())
    {
        ui->textEdit->append("Не задано предельное СКО по вертикали\n");
        ui->textEdit_2->append("Не задано предельное СКО по вертикали\n");
        return false;
    }


    if(!ui->MesCountSpinBox->value())
    {
        ui->textEdit->append("Задано неверное число измерений\n");
        ui->textEdit_2->append("Задано неверное число измерений\n");
        return false;
    }

    if(!ui->IncliationSKOSpinBox->value() && ui->checkCompensator->isChecked())
    {
        ui->textEdit->append("Не задано предельное СКО наклона\n");
        ui->textEdit_2->append("Не задано предельное СКО наклона\n");
        return false;
    }

    ui->textEdit->append("Начинаю измерения\n");
    ui->textEdit_2->append("Начинаю измерения\n");
    return true;
}

/*Параллельное измерение без проверки результатов от обоих теодолитов, т.е, если одно не выполнилось, а второе выполнилось, оно все равно
запишется*/
void MainWindow::parallelMakeMeasureUncheck(QSharedPointer<Theodolite> theodolite,QComboBox*combo_box,const int&count_of_measures,bool do_save)
{
    try
    {

        if(ui->checkCompensator->isChecked())
        {
            if(!theodolite->checkCompensator(ui->IncliationSKOSpinBox->value()))
            {
                DFT data;
                data.device_name = theodolite->objectName();
                emit readyDataFromThread(data);
                QApplication::beep();
                return;
            }

        }

        MeasuresFromTheodolite measure_vector;

        if(combo_box->currentText() == "Компенсатор")
        {
            measure_vector = theodolite->startMeasures(count_of_measures,THD_INCL);
        }
        else
        {
            measure_vector = theodolite->startMeasures(count_of_measures,THD_MES);
        }

        MeasureCharacteristics meas_chrc = calculateMeasureCharact(measure_vector);
        GMS sko_hz_angle(meas_chrc.sko_hz);
        GMS sko_v_angle(meas_chrc.sko_v);
        GMS EV_hz_angle(meas_chrc.EV_hz);
        GMS EV_v_angle(meas_chrc.EV_v);

        bool sko_limit = false;
        const double limit_SKOHz = ui->SKOHzLimitSpinBox->value();
        const double limit_SKOV = ui->SKOVLimitSpinBox->value();

        if(sko_hz_angle.getSeconds() > limit_SKOHz || sko_v_angle.getSeconds() > limit_SKOV)
        {
            QApplication::beep();
            sko_limit = true;
        }

        DFT data;
        data.device_name = theodolite->objectName();
        data.mean_Hz = EV_hz_angle;
        data.mean_V = EV_v_angle;
        data.sko_Hz = sko_hz_angle;
        data.sko_V = sko_v_angle;
        data.measure_vector = measure_vector;
        emit readyDataFromThread(data);


        if(!sko_limit && measures_started && do_save)
        {
            auto getDeviceName = [&](){
                return theodolite->objectName()==(theodolite_1->objectName()) ? "Теодолит №1": "Теодолит №2";
            };
            const QString protocol_text=QString::number(++num_of_measures)+"\t"+tr(getDeviceName())+"\t"+"КРУГ("
                    +QString::number(measure_vector[0].getCircle())+")\t"+combo_box->currentText()
                    +"\t"+zeroPadding(meas_chrc.EV_hz,10)+"\t"+zeroPadding(meas_chrc.EV_v,10)+"\t"
                    +zeroPadding(EV_hz_angle.getGradus())+" "+QString::number(EV_hz_angle.getMinutes())+" "+QString::number(EV_hz_angle.getSeconds(),'f',3)+"\t"
                    +zeroPadding(EV_v_angle.getGradus())+" "+QString::number(EV_v_angle.getMinutes())+" "+QString::number(EV_v_angle.getSeconds(),'f',3)+"\t"
                    +zeroPadding(sko_hz_angle.getSeconds(),3)+"\t"+zeroPadding(sko_v_angle.getSeconds(),3)+"\n";

            measure_mutex.lock();
            saveProtocolText(protocol_text);
            measure_mutex.unlock();
        }
    }
    catch(std::exception& e)
    {
        QMessageBox::information(NULL,"Ошибка",e.what());
        return;
    }
    catch(...)
    {
        QMessageBox::information(NULL,"Ошибка","Неизвестная ошибка");
        return;
    }

}

/*Параллельное измерение с проверкой результатов. Если измерения одного из теодолитов выходят за предельное СКО - не будет записано измерения ни от
одного из них*/

DataFromThreadtoProtocol MainWindow::parralelMakeMeasureCheck(QSharedPointer<Theodolite>theodolite, QComboBox*combo_box, const int&count_of_measures)
{

    if(ui->checkCompensator->isChecked())
    {

        if(!theodolite->checkCompensator(ui->IncliationSKOSpinBox->value()))
        {
            DFT data;
            data.device_name = theodolite->objectName();
            emit readyDataFromThread(data);
            QApplication::beep();
            DFTP protocol_data;
            return protocol_data;
        }
    }

    MeasuresFromTheodolite measure_vector;

    if(combo_box->currentText() == "Компенсатор")
    {
        measure_vector = theodolite->startMeasures(count_of_measures,THD_INCL);
    }
    else
    {
        measure_vector = theodolite->startMeasures(count_of_measures,THD_MES);
    }

    MeasureCharacteristics meas_chrc = calculateMeasureCharact(measure_vector);
    GMS sko_hz_angle(meas_chrc.sko_hz);
    GMS sko_v_angle(meas_chrc.sko_v);
    GMS EV_hz_angle(meas_chrc.EV_hz);
    GMS EV_v_angle(meas_chrc.EV_v);

    const double limit_SKOHz = ui->SKOHzLimitSpinBox->value();
    const double limit_SKOV = ui->SKOVLimitSpinBox->value();

    if(sko_hz_angle.getSeconds() > limit_SKOHz || sko_v_angle.getSeconds() > limit_SKOV)
    {
        QApplication::beep();
    }

    DFT data;
    data.device_name = theodolite->objectName();
    data.mean_Hz = EV_hz_angle;
    data.mean_V = EV_v_angle;
    data.sko_Hz = sko_hz_angle;
    data.sko_V = sko_v_angle;
    data.measure_vector = measure_vector;
    emit readyDataFromThread(data);


    auto getDeviceName = [&](){
        return theodolite->objectName() == (theodolite_1->objectName()) ? "Теодолит №1": "Теодолит №2";
    };
    const QString protocol_text = QString::number(++num_of_measures)+"\t"+tr(getDeviceName())+"\t"
            +"КРУГ("+QString::number(measure_vector[0].getCircle())+")\t"+combo_box->currentText()+"\t"
            +zeroPadding(meas_chrc.EV_hz,10)+"\t"+zeroPadding(meas_chrc.EV_v,10)+"\t"
            +zeroPadding(EV_hz_angle.getGradus())+" "+QString::number(EV_hz_angle.getMinutes())+" "+QString::number(EV_hz_angle.getSeconds(),'f',3)+"\t"
            +zeroPadding(EV_v_angle.getGradus())+" "+QString::number(EV_v_angle.getMinutes())+" "+QString::number(EV_v_angle.getSeconds(),'f',3)+"\t"
            +zeroPadding(sko_hz_angle.getSeconds(),3)+"\t"+zeroPadding(sko_v_angle.getSeconds(),3)+"\n";
    DFTP protocol_data;
    protocol_data.sko_hz = sko_hz_angle;
    protocol_data.sko_v = sko_v_angle;
    protocol_data.protocol_text = protocol_text;
    return protocol_data;

}



void MainWindow::saveProtocolText(const QString& protocolText)
{

    QFile data(directory_name+report_filename);
    if (data.open(QFile::WriteOnly | QFile::Append | QIODevice::Text))
    {
        QTextStream out(&data);
        out<<protocolText;
    }
}



void MainWindow::thdWizardHandler()
{
    /*Обозначение полей между метками section*/
    /*(0,0)- порядковый номер пункта, (1,1)- измеряющий\действующий прибор*/
    /*(2,2)- объект измерения\действия, (3,3)- комментарий к проведению измерений*/
    /*ячейка объекта измерений (2,2) может содержать разделение через ",", содержащее*/
    /*объект измерения для первого и второго теодолита соответственно*/

    /*Если предыдущая инструкция не была распознана, то считываем файл инструкции заново, в надежде, что пользователь его подправил*/
    if(wizard_error)
    {
        updateParagraphsAfterError();
        wizard_error = false;
    }

    currentActionPushButton = nullptr;
    need_to_complete_operation = NO_OPERATION;

    if(ThdWizardParagraphs.isEmpty())
    {
        QMessageBox::information(NULL,"Ошибка","Руководство не считано");
        return;
    }
    if(ThdWizardParagraphs.paragraphs.size() <= ThdWizardParagraphs.curr_num)
    {
        ui->WizardTextEdit->appendPlainText("Измерения окончены.");
        ThdWizardParagraphs.curr_num=0;
        return;
    }
    /*Парсим строку инструкции из файла*/
    const QString curr_paragraph=ThdWizardParagraphs.paragraphs[ThdWizardParagraphs.curr_num];
    const QString active_device=curr_paragraph.section('\t',1,1);
    const QString comment_to_action=curr_paragraph.section('\t',3,3);
    const QString object_of_measure=curr_paragraph.section('\t',2,2);

    try
    {
        if(active_device.isEmpty())
        {
            ui->WizardTextEdit->setPlainText(comment_to_action);
            pass_next = true;
            updateLed();
        }


        else if(active_device.contains("Параллельно"))
        {
            ui->WizardTextEdit->setPlainText(comment_to_action);
            if(object_of_measure.contains("Компенсатор"))
            {

                ui->ReportComboBox->setCurrentText(object_of_measure);
                ui->ReportComboBox_2->setCurrentText(object_of_measure);
                currentActionPushButton = ui->parallelMesAndSaveButton;
                need_to_complete_operation = MEASURE_PARRALEL;
            }
            else if(object_of_measure.contains("друг"))
            {
                ui->ReportComboBox->setCurrentText("Теодолит №2");
                ui->ReportComboBox_2->setCurrentText("Теодолит №1");
                currentActionPushButton = ui->parallelMesAndSaveButton;
                need_to_complete_operation = MEASURE_PARRALEL;
            }
            else if(object_of_measure.contains("Сброс"))
            {
                currentActionPushButton = ui->parallelResetHzPushButton;
                need_to_complete_operation = RESET_HZ_PARRALEL;
            }
            else
            {
                if(ui->ReportComboBox->findText(object_of_measure.section(',',0,0))!=-1 &&
                        ui->ReportComboBox_2->findText(object_of_measure.section(',',1,1))!=-1)
                {
                    ui->ReportComboBox->setCurrentText(object_of_measure.section(',',0,0));
                    ui->ReportComboBox_2->setCurrentText(object_of_measure.section(',',1,1));
                    currentActionPushButton = ui->parallelMesAndSaveButton;
                    need_to_complete_operation = MEASURE_PARRALEL;
                }
                else
                {
                    throw ThdWizardParagraphs.curr_num;
                }
            }
            pass_next = false;
            updateLed();
        }



        else if(active_device.contains("№1"))
        {
            if(object_of_measure.contains("Сброс"))
            {
                ui->WizardTextEdit->setPlainText(comment_to_action);
                currentActionPushButton = ui->resetHzPushButton;
                need_to_complete_operation = RESET_HZ_FIRST_THD;
                pass_next = false;
                update();
            }
            else
            {
                /*Если объект операции не содержится в комбо боксе, ошибка составления программы*/
                if(ui->ReportComboBox->findText(object_of_measure)== -1)
                {
                    throw ThdWizardParagraphs.curr_num;
                }
                ui->ReportComboBox->setCurrentText(object_of_measure);
                ui->WizardTextEdit->setPlainText(comment_to_action);
                currentActionPushButton = ui->mesAndSaveButton;
                need_to_complete_operation = MEASURE_ONE_THD;
                pass_next = false;
                updateLed();
            }
        }


        else if(active_device.contains("№2"))
        {
            if(object_of_measure.contains("Сброс"))
            {
                ui->WizardTextEdit->setPlainText(comment_to_action);
                currentActionPushButton = ui->resetHzPushButton_2;
                need_to_complete_operation = RESET_HZ_SECOND_THD;
                pass_next = false;
                updateLed();
            }
            else
            {
                /*Если объект операции не содержится в комбо боксе, ошибка составления программы*/
                if(ui->ReportComboBox_2->findText(object_of_measure) == -1)
                {
                    throw ThdWizardParagraphs.curr_num;
                }
                ui->ReportComboBox_2->setCurrentText(object_of_measure);
                ui->WizardTextEdit->setPlainText(comment_to_action);
                currentActionPushButton = ui->mesAndSaveButton_2;
                need_to_complete_operation = MEASURE_ONE_THD;
                pass_next = false;
                updateLed();
            }
        }


        else if(active_device.contains("Зв"))
        {
            ui->WizardTextEdit->setPlainText(comment_to_action);
            currentActionPushButton = ui->starThdButton;
            need_to_complete_operation = MEASURE_STAR_THD;
            pass_next = false;
            updateLed();
        }
        else
        {
            throw ThdWizardParagraphs.curr_num;
        }
        ++ ThdWizardParagraphs.curr_num;
    }


    catch (quint16 parghr_number)
    {
        ui->WizardTextEdit->setPlainText("Ошибка в строке "+QString::number(parghr_number+1));
        wizard_error = true;
        pass_next = true;
        updateLed();
    }

}

void MainWindow::on_chooseFilePushButton_clicked()
{
    wizard_filename = QFileDialog::getOpenFileName(this,
                                                   tr("Open .txt"), ".",
                                                   tr(".txt files (*.txt)"));
    if(!ThdWizardParagraphs.isEmpty()) ThdWizardParagraphs.clear();
    QFile file(wizard_filename);
    QTextStream in(&file);
    if(file.open(QIODevice::ReadOnly |QIODevice::Text))
    {
        while(!in.atEnd())
        {
            ThdWizardParagraphs.paragraphs.append(in.readLine());
        }
    }
    ui->WizardTextEdit->setPlainText("Нажмите 'Далее' чтобы начать.");
    pass_next = true;
    updateLed();
}


void MainWindow::updateParagraphsAfterError()
{
    ThdWizardParagraphs.paragraphs.clear();
    QFile file(wizard_filename);
    QTextStream in(&file);
    if(file.open(QIODevice::ReadOnly |QIODevice::Text))
    {
        while(!in.atEnd())
        {
            ThdWizardParagraphs.paragraphs.append(in.readLine());
        }
    }
}

void MainWindow::notePerformOfOperation(OPERATION_TYPE type)
{
    if(need_to_complete_operation == type)
    {
        pass_next = true;
        updateLed();
    }
}


void MainWindow::on_performPushButton_clicked()
{
    if(currentActionPushButton)
    {
        currentActionPushButton->animateClick(0);
        if(pass_next) updateLed();
    }
}

void MainWindow::on_nextPushButton_clicked()
{
    if(pass_next) thdWizardHandler();
}





void MainWindow::updateLed()
{
    if(pass_next) ui->led->setState(true);
    else ui->led->setState(false);
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete ui;
}


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <meanskofcn.h>




MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    qRegisterMetaType <DataFromThread> ();
    settings = new QSettings(QDir::currentPath() + "/" + "config.ini", QSettings::IniFormat, this);

    setStyle();
    initializeValidators();
    initializeConnectionTable();
    initializeComboBoxes();

    theodolite1.reset(new Theodolite());
    theodolite1->setObjectName("theodolite1");
    if (!theodolite1->connectLibrary("GeoComS2K.dll"))
    {
        ui->textEdit->append("Неверно задан путь к библиотеке Geocom\n");
    }

    theodolite2.reset(new Theodolite());
    theodolite2->setObjectName("theodolite2");
    if (!theodolite2->connectLibrary("GeoComS2K2.dll"))
    {
        ui->textEdit_2->append("Неверно задан путь к библиотеке Geocom\n");
    }

    starThdDialog = new Dialog(this);

    connect(ui->openDirectoryButton, &QPushButton::clicked, this, &MainWindow::on_openDirectoryButton_clicked);
    connect(ui->chooseDirectoryButton, &QPushButton::clicked, this, &MainWindow::on_chooseDirectoryButton_clicked);
    connect(ui->startMeasuresButton, &QPushButton::clicked, this, &MainWindow::on_startMeasuresButton_clicked);
    connect(ui->endMeasuresButton, &QPushButton::clicked, this, &MainWindow::on_endMeasuresButton_clicked);

    connect(ui->resetHzPushButton, &QPushButton::clicked, this, &MainWindow::resetOrientationHandler);
    connect(ui->resetHzPushButton_2, &QPushButton::clicked, this, &MainWindow::resetOrientationHandler);

    connect(theodolite1.data(), &Theodolite::connected, this, &MainWindow::reportAboutConnection);
    connect(theodolite2.data(), &Theodolite::connected, this, &MainWindow::reportAboutConnection);

    connect(theodolite1.data(), &Theodolite::errorReport, this, &MainWindow::reportAboutError);
    connect(theodolite2.data(), &Theodolite::errorReport, this, &MainWindow::reportAboutError);

    connect(ui->starThdButton, &QPushButton::clicked, this, &MainWindow::on_starThdButton_clicked);

    connect(ui->mesButton, &QPushButton::clicked, this, [this]() {makeMeasureMain(theodolite1, ui->ReportComboBox, false);});
    connect(ui->mesButton_2, &QPushButton::clicked,this, [this]() {makeMeasureMain(theodolite2, ui->ReportComboBox_2, false);});

    connect(ui->mesAndSaveButton, &QPushButton::clicked, this, [this]() {makeMeasureMain(theodolite1, ui->ReportComboBox, true);});
    connect(ui->mesAndSaveButton_2, &QPushButton::clicked, this,[this]() {makeMeasureMain(theodolite2, ui->ReportComboBox_2, true);});

    connect(this,&MainWindow::readyDataFromThread, this, &MainWindow::dataFromThreadHandler, Qt::QueuedConnection);
    connect(ui->parallelMesPushButton, &QPushButton::clicked, this, &MainWindow::parallelHandlerNoSave);
    connect(ui->parallelMesAndSaveButton, &QPushButton::clicked, this, &MainWindow::parallelHandlerDoSave);

    connect(starThdDialog, &Dialog::starThdDataReady, this, &MainWindow::saveStarThdData);

    connect(ui->parallelResetHzPushButton, &QPushButton::clicked, this, &MainWindow::parallelResetOrientation);
    connect(ui->chooseFilePushButton, &QPushButton::clicked, this, &MainWindow::on_chooseFilePushButton_clicked);
    connect(ui->performPushButton, &QPushButton::clicked, this, &MainWindow::on_performPushButton_clicked);
    connect(ui->nextPushButton, &QPushButton::clicked, this, &MainWindow::on_nextPushButton_clicked);

    loadSettings();

}


void MainWindow::initializeValidators()
{
    QRegularExpression rxAngle("[0-3]{1}[0-9]{2} [0-9]{2} [0-9]{2}");
    QValidator *gmsValidator = new QRegularExpressionValidator(rxAngle, this);
    ui->HzAngleValueLineEdit->setValidator(gmsValidator);
    ui->HzAngleValueLineEdit_2->setValidator(gmsValidator);

}


void MainWindow::notifyBoth(const QString& message)
{
    ui->textEdit->append(message);
    ui->textEdit_2->append(message);
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
    QStringList reportList;

    reportList << "Теодолит №2"<< "Компенсатор" << "Теодолит Зв" << "Куб прибора А" << "Куб прибора Б"
               << "Куб прибора В" << "Куб прибора Г" << "Куб прибора Д" << "Куб стенда А" << "Куб стенда Б"
               << "Куб стенда В" << "Куб стенда Г" << "Куб стенда Д" << "Лекальную линейку" << "Плоское зеркало";

    ui->ReportComboBox->insertItems(0, reportList);
    reportList.pop_front();
    reportList.prepend("Теодолит №1");
    ui->ReportComboBox_2->insertItems(0, reportList);


    QStringList comPorts;
    for (quint32 i = COM_1; i <= COM_24; i++)
    {
        comPorts.append(QString("COM_%1").arg(i + 1));
    }
    ui->comPortComboBox->insertItems(0, comPorts);
    ui->comPortComboBox_2->insertItems(0, comPorts);
}

void MainWindow::initializeConnectionTable()
{
    for (int i = 0 ; i < 2; i++)
    {
        ui->tableWidget->setItem(0,i, new QTableWidgetItem);
        ui->tableWidget->item(0, i)->setText("Отключен");
        ui->tableWidget->item(0, i)->setBackground(QColor(255, 0, 0, 70));
    }

}

void MainWindow::saveSettings()
{
    settings->setValue("directoryName", directoryName);
    settings->setValue("thd1_inclination", ui->HzAngleValueLineEdit->text());
    settings->setValue("thd2_inclination", ui->HzAngleValueLineEdit_2->text());
    settings->setValue("check_compensator", ui->checkCompensator->isChecked());
    settings->setValue("thd1Port", ui->comPortComboBox->currentIndex());
    settings->setValue("thd2Port", ui->comPortComboBox_2->currentIndex());
    settings->sync();
}



void MainWindow::loadSettings()
{
    directoryName = settings->value("directoryName", "").toString();
    ui->directoryNameLineEdit->setText(settings->value("directoryName"," Директория не выбрана...").toString());
    ui->HzAngleValueLineEdit->setText(settings->value("thd1_inclination", "000 00 00").toString());
    ui->HzAngleValueLineEdit_2->setText(settings->value("thd2_inclination", "000 00 00").toString());
    ui->checkCompensator->setChecked(settings->value("check_compensator", "true").toBool());
    ui->comPortComboBox->setCurrentIndex(settings->value("thd1Port", "3").toInt());
    ui->comPortComboBox_2->setCurrentIndex(settings->value("thd2Port", "7").toInt());
}



void MainWindow::resetOrientation(QSharedPointer<Theodolite> theodolite, QLineEdit* getAngleLineEdit, QTextEdit* theodolitesTextEdit)
{
    if (!theodolite->isConnected())
    {
        theodolitesTextEdit->append("Теодолит не подключен");
        return;
    }
    QString gmsAngle = getAngleLineEdit->text();
    if (!GMS::checkStringGMSAngle(gmsAngle))
    {
        theodolitesTextEdit->append("Угол введен неверно.");
        return;
    }
    QStringList splitGms = gmsAngle.split(" ");
    GMS transAngle;
    transAngle.setGradus(splitGms.at(0).toInt());
    transAngle.setMinutes(splitGms.at(1).toInt());
    transAngle.setSeconds(splitGms.at(2).toDouble());

    double orientationAngle = transAngle.transToRad();
    theodolite->resetHz(orientationAngle);


}

void MainWindow::parallelResetOrientation()
{
    std::thread thd1(&MainWindow::resetOrientation, this, theodolite1, ui->HzAngleValueLineEdit, ui->textEdit);
    thd1.detach();
    std::thread thd2(&MainWindow::resetOrientation, this, theodolite2, ui->HzAngleValueLineEdit_2, ui->textEdit_2);
    thd2.detach();
    notePerformOfOperation(RESET_HZ_PARRALEL);
}


void MainWindow::resetOrientationHandler()
{
    QPushButton* pb = dynamic_cast<QPushButton*>(sender());
    if (!pb->objectName().contains("2"))
    {
        resetOrientation(theodolite1, ui->HzAngleValueLineEdit ,ui->textEdit);
        notePerformOfOperation(RESET_HZ_FIRST_THD);
    }
    else
    {
        resetOrientation(theodolite2, ui->HzAngleValueLineEdit_2, ui->textEdit_2);
        notePerformOfOperation(RESET_HZ_SECOND_THD);
    }

}


void MainWindow::on_chooseDirectoryButton_clicked()
{
    directoryName = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                      "/home",
                                                      QFileDialog::ShowDirsOnly
                                                      | QFileDialog::DontResolveSymlinks);
    ui->directoryNameLineEdit->setText(directoryName);

}

void MainWindow::on_openDirectoryButton_clicked()
{
    auto url = QUrl::fromLocalFile(directoryName);
    QDesktopServices::openUrl(url);
}





void MainWindow::on_startMeasuresButton_clicked()
{
    if (directoryName.isEmpty())
    {
        notifyBoth("Директория не выбрана!");
        return;
    }

    QFile directory (directoryName);
    if (!directory.exists())
    {
        notifyBoth("Директория не существует!");
        return;
    }

    if (measuresStarted)
    {
        notifyBoth("Завершите предыдущие измерения прежде чем начать новые");
        return;
    }

    ui->textEdit->clear();
    ui->textEdit_2->clear();


    QDate currentDate = QDate::currentDate();
    QTime currentTime = QTime::currentTime();
    reportFilename = "/" + currentDate.toString("yy.MM.dd")
            + "_" + currentTime.toString("hh-mm-ss") + "_Measures"+".txt";
    protocolFilename = "/" + currentDate.toString("yy.MM.dd")
            + "_" + currentTime.toString("hh-mm-ss") + "_Protocol"+".txt";
    measuresStarted = true;
    notifyBoth("Измерения начаты\n");

}


/*Измерения с записью в файл окончены*/
void MainWindow::on_endMeasuresButton_clicked()
{
    measuresStarted = false;
    ui->textEdit->append("Измерения закончены\n");
    numMeasures = 0;

    QFile data(directoryName + protocolFilename);
    if (data.open(QFile::WriteOnly | QFile::Append | QIODevice::Text))
    {
        QTextStream out(&data);
        out << ui->textEdit->toPlainText();
        out << ui->textEdit_2->toPlainText();
    }
    reportFilename.clear();
    protocolFilename.clear();
}




void MainWindow::on_starThdButton_clicked()
{
    starThdDialog->exec();
}



void MainWindow::reportAboutConnection(const QString& message)
{

    int column = 0;
    if (!(sender()->objectName() == theodolite1->objectName()))
    {
        column = 1;
    }
    ui->textEdit->append(message);
    ui->textEdit->append("Теодолит №1 успешно подключен\n");
    ui->tableWidget->item(0, column)->setBackground(QColor(0, 255, 0, 70));
    ui->tableWidget->item(0, column)->setText("Подключен");

}



void MainWindow::reportAboutError(const QString& errorMessage)
{
    if (sender()->objectName() == theodolite1->objectName())
    {
        ui->textEdit->append("Теодолит №1: произошла ошибка\n");
        ui->textEdit->append(errorMessage);
        return;
    }
    else
    {
        ui->textEdit_2->append("Теодолит №2: произошла ошибка\n");
        ui->textEdit_2->append(errorMessage);
    }

}


QString MainWindow::makeProtocol(const QString& deviceName, double circle, const QString& measureType, const MeasureCharacteristics& measChrc,
                                 const GMS& EvHzAngle, const GMS& EvVAngle, const GMS& skoHzAngle, const GMS& skoVAngle)
{

    QString protocolText = QString ("%1\t%2\tКРУГ(%3)\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12\t%13\t%14\n")
            .arg(numMeasures++)
            .arg(deviceName).arg(circle)
            .arg(measureType)
            .arg(zeroPadding(measChrc.EvHz, 10))
            .arg(zeroPadding(measChrc.EvV ,10))
            .arg(zeroPadding(EvHzAngle.getGradus()))
            .arg(EvHzAngle.getMinutes()).arg(EvHzAngle.getSeconds(), 0, 'f', 3)
            .arg(zeroPadding(EvVAngle.getGradus()))
            .arg(EvVAngle.getMinutes()).arg(EvVAngle.getSeconds(), 0, 'f', 3)
            .arg(zeroPadding(skoHzAngle.getSeconds(), 3))
            .arg(zeroPadding(skoVAngle.getSeconds(), 3));

    return protocolText;
}

QString MainWindow::getDeviceName(QSharedPointer <Theodolite> theodolite)
{
    return theodolite->objectName() == (theodolite1->objectName()) ? "Теодолит №1": "Теодолит №2";
}

void MainWindow::saveStarThdData(StarThdData starData)
{
    if (!measuresStarted)
    {
        QMessageBox::information(nullptr, "Ошибка", "Измерения не начаты");
        return;
    }
    QString HzAngle = starData.HzAngle;
    QString VAngle = starData.VAngle;

    if (!GMS::checkStringGMSAngle(HzAngle)
            || !GMS::checkStringGMSAngle(VAngle))
    {
        QMessageBox::information(nullptr, "Ошибка", "Углы заданы неверно");
        return;
    }
    QStringList splitHzAngle = HzAngle.split(" ");
    QStringList splitVAngle = VAngle.split(" ");


    GMS HzAngleGms;
    HzAngleGms.setGradus(splitHzAngle.at(0).toInt());
    HzAngleGms.setMinutes(splitHzAngle.at(1).toInt());
    HzAngleGms.setSeconds(splitHzAngle.at(2).toDouble());

    GMS VAngleGms;
    VAngleGms.setGradus(splitVAngle.at(0).toInt());
    VAngleGms.setMinutes(splitVAngle.at(1).toInt());
    VAngleGms.setSeconds(splitVAngle.at(2).toDouble());


    double hzAngleGrad = HzAngleGms.transToGrad();
    double vAngleGrad = VAngleGms.transToGrad();
    const QString protocolText = QString::number(numMeasures++) + "\t"
            + tr("Зв.Теодолит") + "\t" + starData.cirle
            +"\t" + starData.MeasureObj + "\t"
            + zeroPadding(hzAngleGrad,10) + "\t"
            + zeroPadding(vAngleGrad,10) + "\t"
            + zeroPadding(HzAngleGms.getGradus()) + " " +
            QString::number(HzAngleGms.getMinutes()) + " " +
            QString::number(HzAngleGms.getSeconds(),'f',3) + "\t"
            + zeroPadding(VAngleGms.getGradus()) + " "   +
            QString::number(VAngleGms.getMinutes()) +" " +
            QString::number(VAngleGms.getSeconds(),'f',3) + "\t"+ "\n";

    saveProtocolText(protocolText);
    notePerformOfOperation(MEASURE_STAR_THD);


}



MeasureCharacteristics MainWindow::calculateMeasureCharact(const MeasuresFromTheodolite& measureVector)
{
    MeasureCharacteristics measChrc;
    auto minMaxPair = std::minmax_element(measureVector.begin(), measureVector.end(),
                                          [](auto& a, auto& b){return a.getHzAngle() < b.getHzAngle();});

    // если находимся в области нуля(т.е разность между минимальным и максимальным углом),
    // переопределяем углы через asin(sin()), считаем среднее по "Mean of circular quantities"
    const int maxScatter = 356;
    if ((*minMaxPair.second).getHzAngle() - (*minMaxPair.first).getHzAngle() > maxScatter)
    {

        measChrc.EvHz = meanSkoFcn::calcCircleMean(measureVector, meanSkoFcn::HZ_A);
        measChrc.skoHz = meanSkoFcn::calcCircleSko(measureVector, measChrc.EvHz, meanSkoFcn::HZ_A);
        if (measChrc.EvHz < 0)
            measChrc.EvHz = 360.0 + measChrc.EvHz;
    }
    else
    {
        measChrc.EvHz = meanSkoFcn::calcMean(measureVector, meanSkoFcn::HZ_A);
        measChrc.skoHz = meanSkoFcn::calcSko(measureVector, measChrc.EvHz, meanSkoFcn::HZ_A);
    }

    measChrc.EvV = meanSkoFcn::calcMean(measureVector, meanSkoFcn::V_A);
    measChrc.skoV = meanSkoFcn::calcSko(measureVector, measChrc.EvV, meanSkoFcn::V_A);

    return measChrc;

}




void MainWindow::trySaveProtocol(bool save)
{
    if (save && measuresStarted)
    {
        auto result = watcher.result();
        if (!result.isEmpty())
        {
            double limitSkohz = ui->SKOHzLimitSpinBox->value();
            double limitSkov = ui->SKOVLimitSpinBox->value();

            if (result.skoHz.getSeconds() < limitSkohz)
            {
                if (result.skoV.getSeconds() < limitSkov)
                {
                    saveProtocolText(result.protocolText);
                    notePerformOfOperation(MEASURE_ONE_THD);
                }
            }
        }
    }
}

void MainWindow::makeMeasureMain(QSharedPointer<Theodolite> thd, QComboBox* reportComboBox, bool save)
{
    if (watcher.isRunning())
    {
        watcher.waitForFinished();
    }
    QObject::connect(&watcher, &QFutureWatcher<DFTP>::finished, [this, save]()
    {
        trySaveProtocol(save);
    });
    auto future = QtConcurrent::run(this, &MainWindow::parallelMeasure, thd, reportComboBox, ui->MesCountSpinBox->value());
    watcher.setFuture(future);
}



void MainWindow::parallelHandlerNoSave()
{
    if (prepareMeasure(ui->textEdit)
            && prepareMeasure(ui->textEdit_2))
    {
        connectTheodolite(theodolite1.data(), ui->textEdit, ui->comPortComboBox);
        connectTheodolite(theodolite2.data(), ui->textEdit_2, ui->comPortComboBox_2);

        if (theodolite1->isConnected()
                && theodolite2->isConnected())
        {
            QtConcurrent::run(this, &MainWindow::parallelMeasure, theodolite1, ui->ReportComboBox, ui->MesCountSpinBox->value());
            QtConcurrent::run(this, &MainWindow::parallelMeasure, theodolite2, ui->ReportComboBox_2, ui->MesCountSpinBox->value());
        }
    }

}


void MainWindow::parallelHandlerDoSave()
{
    if (!measuresStarted)
    {
        notifyBoth("Измерения не начаты.");
        return;
    }

    if (!prepareMeasure(ui->textEdit)
            && !prepareMeasure(ui->textEdit_2))
    {
        return;
    }
    connectTheodolite(theodolite1.data(), ui->textEdit, ui->comPortComboBox);
    connectTheodolite(theodolite2.data(), ui->textEdit_2, ui->comPortComboBox_2);

    if (!theodolite1->isConnected()
            || !theodolite2->isConnected())
        return;

    auto thd1Future = QtConcurrent::run(this, &MainWindow::parallelMeasure, theodolite1, ui->ReportComboBox, ui->MesCountSpinBox->value());
    auto thd2Future = QtConcurrent::run(this, &MainWindow::parallelMeasure, theodolite2, ui->ReportComboBox_2, ui->MesCountSpinBox->value());

    DFTP thd1Data = thd1Future.result();
    DFTP thd2Data = thd2Future.result();

    if (!thd1Data.isEmpty()
            && !thd2Data.isEmpty())
    {
        double limitSkohz = ui->SKOHzLimitSpinBox->value();
        double limitSkov = ui->SKOVLimitSpinBox->value();

        if (thd1Data.skoHz.getSeconds() < limitSkohz
                && thd2Data.skoHz.getSeconds() < limitSkohz)
        {
            if (thd1Data.skoV.getSeconds() < limitSkov
                    && thd2Data.skoV.getSeconds() < limitSkov)
            {
                saveProtocolText(thd1Data.protocolText);
                saveProtocolText(thd2Data.protocolText);
                notePerformOfOperation(MEASURE_PARRALEL);
            }
        }
    }
}


void MainWindow::dataFromThreadHandler(const DFT& data)
{
    if (data.deviceName == theodolite1->objectName())
    {

        setDataFromThread(data, ui->textEdit, ui->meanHzLineEdit, ui->meanVLineEdit,
                          ui->SKOHzLineEdit, ui->SKOVLineEdit, ui->SKO3xHzLineEdit, ui->SKO3xVLineEdit);
    }
    else
    {
        setDataFromThread(data, ui->textEdit_2, ui->meanHzLineEdit_2, ui->meanVLineEdit_2,
                          ui->SKOHzLineEdit_2, ui->SKOVLineEdit_2, ui->SKO3xHzLineEdit_2, ui->SKO3xVLineEdit_2);
    }
}




void MainWindow::setDataFromThread(const DFT& data, QTextEdit* textEdit,
                                   QLineEdit* meanhzLineEdit, QLineEdit* meanvLineEdit, QLineEdit* skohzLineEdit
                                   , QLineEdit* skovLineEdit, QLineEdit* SKO3xHzLineEdit, QLineEdit* SKO3xVLineEdit)
{
    /*если превышено ско компенсатора, то сюда придёт структура с пустыми данными об измерениях*/
    if (data.notContainsMeasures())
    {
        textEdit->append("Превышено СКО компенсатора\n");
        return;
    }
    for (const auto& i : data.measureVector)
    {
        GMS firstAngle(i.getHzAngle());
        GMS secondAngle(i.getVAngle());
        textEdit->append(QString("%1").arg(QString::number(i.getHzAngle(),'f', 10), -20,' ') + "\t"
                         + QString::number(firstAngle.getGradus()) + " "
                         + QString::number(firstAngle.getMinutes()) + " "
                         + QString::number(firstAngle.getSeconds(), 'f', 3) + "\n");
        textEdit->append(QString("%1").arg(QString::number(i.getVAngle(), 'f', 10), -20,' ') + "\t"
                         + QString::number(secondAngle.getGradus()) +" "
                         + QString::number(secondAngle.getMinutes()) + " "
                         + QString::number(secondAngle.getSeconds(),'f',3) + "\n");
    }

    textEdit->append("FACE: " + QString::number(data.measureVector[0].getCircle()));
    textEdit->append("Число некорректных измерений: " + QString::number(ui->MesCountSpinBox->value() - data.measureVector.size()) + "\n");

    if (data.skoHz.getSeconds() > ui->SKOHzLimitSpinBox->value() || data.skoV.getSeconds() > ui->SKOVLimitSpinBox->value())
    {
        textEdit->append("Внимание! Превышено предельное СКО\n");
    }


    meanhzLineEdit->setText(QString::number(data.meanHz.getGradus()) + " " +
                            QString::number(data.meanHz.getMinutes()) + " " +
                            QString::number(data.meanHz.getSeconds(), 'f', 3));
    meanvLineEdit->setText(QString::number(data.meanV.getGradus()) + " "  +
                           QString::number(data.meanV.getMinutes()) + " " +
                           QString::number(data.meanV.getSeconds(), 'f', 3));
    skohzLineEdit->setText(QString::number(data.skoHz.getSeconds(), 'f', 3));
    skovLineEdit->setText(QString::number(data.skoV.getSeconds(),'f', 3));
    SKO3xHzLineEdit->setText(QString::number(3 * data.skoHz.getSeconds(), 'f', 3));
    SKO3xVLineEdit->setText(QString::number(3 * data.skoV.getSeconds(), 'f', 3));
    if (!measuresStarted)
    {
        textEdit->append("Внимание! Измерения не сохранены, т.к они не были начаты\n");
    }
}





void MainWindow::connectTheodolite(Theodolite* thd, QTextEdit* edit, QComboBox* portComboBox)
{
    if (thd->isConnected())
    {
        return;
    }
    else
    {
        COM_PORT thdPort = static_cast <COM_PORT> (portComboBox->currentIndex());
        if (!thd->connect(thdPort))
        {
            edit->append("Подключение не удалось\n");
        }
    }
}

bool MainWindow::prepareMeasure(QTextEdit* edit)
{

    if (qFuzzyCompare(ui->SKOHzLimitSpinBox->value(), 0))
    {
        edit->append("Не задано предельное СКО по горизонтали\n");
        return false;
    }
    if (qFuzzyCompare(ui->SKOVLimitSpinBox->value(), 0))
    {
        edit->append("Не задано предельное СКО по вертикали\n");
        return false;
    }

    if (!ui->MesCountSpinBox->value())
    {
        edit->append("Задано неверное число измерений\n");
        return false;
    }

    if (qFuzzyCompare(ui->IncliationSKOSpinBox->value() , 0)
            && ui->checkCompensator->isChecked())
    {
        edit->append("Не задано предельное СКО наклона\n");
        return false;
    }
    edit->append("Начинаю измерения\n");
    return true;
}



DataFromThreadtoProtocol MainWindow::parallelMeasure(QSharedPointer<Theodolite> theodolite, QComboBox* comboBox, const int countOfMeasures)
{

    if (ui->checkCompensator->isChecked())
    {
        if (!theodolite->checkCompensator(ui->IncliationSKOSpinBox->value()))
        {
            DFT data;
            data.deviceName = theodolite->objectName();
            emit readyDataFromThread(data);
            QApplication::beep();
            DFTP protocolData;
            return protocolData;
        }
    }

    MeasuresFromTheodolite measureVector;

    if (comboBox->currentText() == "Компенсатор")
    {
        measureVector = theodolite->startMeasures(countOfMeasures, THD_INCL);
    }
    else
    {
        measureVector = theodolite->startMeasures(countOfMeasures, THD_MES);
    }

    MeasureCharacteristics measChrc = calculateMeasureCharact(measureVector);
    GMS skoHzAngle(measChrc.skoHz);
    GMS skoVAngle(measChrc.skoV);
    GMS EvHzAngle(measChrc.EvHz);
    GMS EvVAngle(measChrc.EvV);

    const double limitSkohz = ui->SKOHzLimitSpinBox->value();
    const double limitSkov = ui->SKOVLimitSpinBox->value();

    if (skoHzAngle.getSeconds() > limitSkohz
            || skoVAngle.getSeconds() > limitSkov)
    {
        QApplication::beep();
    }

    DFT data;
    data.deviceName = theodolite->objectName();
    data.meanHz = EvHzAngle;
    data.meanV = EvVAngle;
    data.skoHz = skoHzAngle;
    data.skoV = skoVAngle;
    data.measureVector = measureVector;
    emit readyDataFromThread(data);

    const QString measureType = comboBox->currentText();
    QString protocolText = makeProtocol(getDeviceName(theodolite), measureVector[0].getCircle(), measureType, measChrc,
            EvHzAngle, EvVAngle, skoHzAngle, skoVAngle);

    DFTP protocol_data;
    protocol_data.skoHz = skoHzAngle;
    protocol_data.skoV = skoVAngle;
    protocol_data.protocolText = protocolText;
    return protocol_data;

}



void MainWindow::saveProtocolText(const QString& protocolText)
{

    QFile data(directoryName + reportFilename);
    if (data.open(QFile::WriteOnly | QFile::Append | QIODevice::Text))
    {
        QTextStream out(&data);
        out << protocolText;
    }
}

void MainWindow::configurateOperationOneThd(const QString& commentToAction, OPERATION_TYPE type, QPushButton* currentButton)
{
    ui->WizardTextEdit->setPlainText(commentToAction);
    currentActionPushButton = currentButton;
    needToCompeleOperation = type;
    passNext = false;
    updateLed();
}

void MainWindow::thdWizardHandler()
{
    /*Обозначение полей между метками section*/
    /*(0,0)- порядковый номер пункта, (1,1)- измеряющий\действующий прибор*/
    /*(2,2)- объект измерения\действия, (3,3)- комментарий к проведению измерений*/
    /*ячейка объекта измерений (2,2) может содержать разделение через ",", содержащее*/
    /*объект измерения для первого и второго теодолита соответственно*/

    /*Если предыдущая инструкция не была распознана, то считываем файл инструкции заново, в надежде, что пользователь его подправил*/
    if (wizardError)
    {
        updateParagraphsAfterError();
        wizardError = false;
    }

    currentActionPushButton = nullptr;
    needToCompeleOperation = NO_OPERATION;

    if (ThdWizardParagraphs.isEmpty())
    {
        QMessageBox::information(nullptr, "Ошибка", "Руководство не считано");
        return;
    }
    if (ThdWizardParagraphs.paragraphs.size() <= ThdWizardParagraphs.currNum)
    {
        ui->WizardTextEdit->appendPlainText("Измерения окончены.");
        ThdWizardParagraphs.currNum = 0;
        return;
    }
    /*Парсим строку инструкции из файла*/
    const QString currParagraph = ThdWizardParagraphs.paragraphs[ThdWizardParagraphs.currNum];
    const QString activeDevice = currParagraph.section('\t', 1, 1);
    const QString commentToAction = currParagraph.section('\t', 3, 3);
    const QString objectOfMeasure = currParagraph.section('\t', 2, 2);

    try
    {
        if (activeDevice.isEmpty())
        {
            ui->WizardTextEdit->setPlainText(commentToAction);
            passNext = true;
            updateLed();
        }
        else if (activeDevice.toLower().contains("параллельно"))
        {
            ui->WizardTextEdit->setPlainText(commentToAction);
            if (objectOfMeasure.toLower().contains("компенсатор"))
            {
                ui->ReportComboBox->setCurrentText(objectOfMeasure);
                ui->ReportComboBox_2->setCurrentText(objectOfMeasure);
                currentActionPushButton = ui->parallelMesAndSaveButton;
                needToCompeleOperation = MEASURE_PARRALEL;
            }
            else if (objectOfMeasure.toLower().contains("друг"))
            {
                ui->ReportComboBox->setCurrentText("Теодолит №2");
                ui->ReportComboBox_2->setCurrentText("Теодолит №1");
                currentActionPushButton = ui->parallelMesAndSaveButton;
                needToCompeleOperation = MEASURE_PARRALEL;
            }
            else if (objectOfMeasure.toLower().contains("cброс"))
            {
                currentActionPushButton = ui->parallelResetHzPushButton;
                needToCompeleOperation = RESET_HZ_PARRALEL;
            }
            else
            {
                if (ui->ReportComboBox->findText(objectOfMeasure.section(',', 0, 0)) != -1
                        && ui->ReportComboBox_2->findText(objectOfMeasure.section(',', 1, 1)) != -1)
                {
                    ui->ReportComboBox->setCurrentText(objectOfMeasure.section(',', 0, 0));
                    ui->ReportComboBox_2->setCurrentText(objectOfMeasure.section(',', 1, 1));
                    currentActionPushButton = ui->parallelMesAndSaveButton;
                    needToCompeleOperation = MEASURE_PARRALEL;
                }
                else
                {
                    throw ThdWizardParagraphs.currNum;
                }
            }
            passNext = false;
            updateLed();
        }
        else if (activeDevice.contains("№1"))
        {
            if (objectOfMeasure.toLower().contains("cброс"))
            {
                configurateOperationOneThd(commentToAction, RESET_HZ_FIRST_THD, ui->resetHzPushButton);
            }
            else
            {
                /*Если объект операции не содержится в комбо боксе, ошибка составления программы*/
                if (ui->ReportComboBox->findText(objectOfMeasure) == -1)
                {
                    throw ThdWizardParagraphs.currNum;
                }

                ui->ReportComboBox->setCurrentText(objectOfMeasure);
                configurateOperationOneThd(commentToAction, MEASURE_ONE_THD, ui->mesAndSaveButton);
            }
        }
        else if (activeDevice.contains("№2"))
        {
            if (objectOfMeasure.toLower().contains("cброс"))
            {
                configurateOperationOneThd(commentToAction, RESET_HZ_SECOND_THD, ui->resetHzPushButton_2);
            }
            else
            {
                /*Если объект операции не содержится в комбо боксе, ошибка составления программы*/
                if (ui->ReportComboBox_2->findText(objectOfMeasure) == -1)
                {
                    throw ThdWizardParagraphs.currNum;
                }
                ui->ReportComboBox_2->setCurrentText(objectOfMeasure);
                configurateOperationOneThd(commentToAction, MEASURE_ONE_THD, ui->mesAndSaveButton_2);
            }
        }


        else if (activeDevice.toLower().contains("зв"))
        {
            configurateOperationOneThd(commentToAction, MEASURE_STAR_THD, ui->starThdButton);
        }
        else
        {
            throw ThdWizardParagraphs.currNum;
        }
        ++ThdWizardParagraphs.currNum;
    }


    catch (qint32 parghrNumber)
    {
        ui->WizardTextEdit->setPlainText("Ошибка в строке " + QString::number(parghrNumber + 1));
        wizardError = true;
        passNext = true;
        updateLed();
    }

}

void MainWindow::on_chooseFilePushButton_clicked()
{
    wizardFilename = QFileDialog::getOpenFileName(this,
                                                  tr("Open .txt"), ".",
                                                  tr(".txt files (*.txt)"));
    if (!ThdWizardParagraphs.isEmpty()) ThdWizardParagraphs.clear();
    QFile file(wizardFilename);
    QTextStream in(&file);
    if (file.open(QIODevice::ReadOnly |QIODevice::Text))
    {
        while(!in.atEnd())
        {
            ThdWizardParagraphs.paragraphs.append(in.readLine());
        }
    }
    ui->WizardTextEdit->setPlainText("Нажмите 'Далее' чтобы начать.");
    passNext = true;
    updateLed();
}


void MainWindow::updateParagraphsAfterError()
{
    ThdWizardParagraphs.paragraphs.clear();
    QFile file(wizardFilename);
    QTextStream in(&file);
    if (file.open(QIODevice::ReadOnly |QIODevice::Text))
    {
        while (!in.atEnd())
        {
            ThdWizardParagraphs.paragraphs.append(in.readLine());
        }
    }
}

void MainWindow::notePerformOfOperation(OPERATION_TYPE type)
{
    if (needToCompeleOperation == type)
    {
        passNext = true;
        updateLed();
    }
}


void MainWindow::on_performPushButton_clicked()
{
    if (currentActionPushButton)
    {
        currentActionPushButton->animateClick();
        if (passNext)
            updateLed();
    }
}

void MainWindow::on_nextPushButton_clicked()
{
    if (passNext)
        thdWizardHandler();
}



void MainWindow::updateLed()
{
    if (passNext)
        ui->led->setState(true);
    else ui->led->setState(false);
}


MainWindow::~MainWindow()
{
    saveSettings();
    delete ui;
}



void MainWindow::on_ConnectButton_clicked()
{
    connectTheodolite(theodolite1.data(), ui->textEdit, ui->comPortComboBox);
}

void MainWindow::on_ConnectButton_2_clicked()
{
    connectTheodolite(theodolite2.data(), ui->textEdit_2, ui->comPortComboBox_2);
}

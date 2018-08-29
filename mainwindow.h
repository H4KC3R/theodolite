#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "theodolite.h"
#include<QSettings>
#include <mutex>
#include<QLineEdit>
#include<QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <dialog.h>
#include <gms.h>
#include <QDir>
#include <QDebug>
#include <numeric>
#include <QFile>
#include <QTextStream>
#include <QTime>
#include <QDate>
#include <QFileDialog>
#include <QDesktopServices>
#include <thread>
#include <algorithm>
#include <QMessageBox>
#include <future>
#include <QPainter>
#include <QPen>



struct DataFromThread
{
    QString deviceName;
    MeasuresFromTheodolite measureVector;
    GMS meanHz;
    GMS meanV;
    GMS skoHz;
    GMS skoV;

    bool notContainsMeasures() const
    {
        return measureVector.isEmpty();
    }
};
Q_DECLARE_METATYPE(DataFromThread)



struct DataFromThreadtoProtocol
{
    GMS skoHz;
    GMS skoV;
    QString protocol_text;

    bool isEmpty() const
    {
        return skoHz.isEmpty()
                && skoV.isEmpty()
                && protocol_text.isEmpty();
    }
};

//структура содержащая пункты инструкции и номер текущего пункта
struct WizardParagraphs
{
    QVector<QString> paragraphs;
    qint32 currNum = 0;
    void clear()
    {
        paragraphs.clear();
        currNum = 0;
    }

    bool isEmpty() const
    {
        return paragraphs.isEmpty()
                && !currNum;
    }
};



struct MeasureCharacteristics
{
    double skoHz;
    double skoV;
    double EvHz;
    double EvV;
};



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    typedef DataFromThread DFT;
    typedef DataFromThreadtoProtocol DFTP;

    enum OPERATION_TYPE
    {
        NO_OPERATION,
        RESET_HZ_PARRALEL,
        RESET_HZ_FIRST_THD,
        RESET_HZ_SECOND_THD,
        MEASURE_ONE_THD,
        MEASURE_PARRALEL,
        MEASURE_STAR_THD
    };


public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private:

    void notifyBoth(const QString& message);

    void setStyle();

    void connectTheodolit();

    void makeMeasureMain();

    void parallelHandlerNosave();

    void parallelHandlerDosave();

    void resetOrientationHandler();

    void parallelResetOrientation();

    void on_startMeasuresButton_clicked();

    void on_endMeasuresButton_clicked();

    void on_chooseFilePushButton_clicked();

    void on_performPushButton_clicked();

    void on_nextPushButton_clicked();

    void on_chooseDirectoryButton_clicked();

    void on_openDirectoryButton_clicked();

    void on_starThdButton_clicked();

    void updateLed();

    void initializeValidators();

    void initializeComboBoxes();

    void initializeConnectionTable();

    MeasureCharacteristics calculateMeasureCharact (const MeasuresFromTheodolite& measureVector);

    void dataFromThreadHandler(const DFT& data);

    bool prepareParralelMSeasure();

    void parallelConnection();

    void reportAboutConnection(const QString &message);

    void reportAboutError(const QString& errorMessage);

    void setDataFromThread(const DFT& data, QTextEdit* text_edit, QLineEdit* meanHz_line_edit, QLineEdit* meanV_line_edit, QLineEdit* skoHz_line_edit
                              , QLineEdit* skoV_line_edit, QLineEdit* SKO3xHz_line_edit, QLineEdit* SKO3xV_line_edit);

    DFTP parralelMakeMeasureCheck(QSharedPointer <Theodolite> theodolite, QComboBox* combo_box, const int count_of_measures);

    void parallelMakeMeasureUncheck(QSharedPointer <Theodolite> theodolite, QComboBox* combo_box, const int count_of_measures, bool do_save);

    void resetOrientation(QSharedPointer<Theodolite>, QLineEdit*, QTextEdit*);

    void makeMeasure(bool do_save, QSharedPointer <Theodolite> theodolite, QTextEdit* text_edit, QComboBox* combo_box
                     ,QLineEdit* meanHz_line_edit,QLineEdit* meanV_line_edit,QLineEdit* skoHz_line_edit
                     ,QLineEdit* skoV_line_edit,QLineEdit*SKO3xHz_line_edit,QLineEdit* SKO3xV_line_edit);

    void saveStarThdData(StarThdData star_data);

    QString makeProtocol(const QString& deviceName, double circle, const QString& measureType, const MeasureCharacteristics& meas_chrc,
                      const GMS& EV_hz_angle, const GMS& EV_v_angle, const GMS& sko_hz_angle, const GMS& sko_v_angle);
    QString getDeviceName(QSharedPointer <Theodolite> theodolite);

    void saveProtocolText(const QString& protocol_text);

    void saveSettings();

    void loadSettings();

    void thdWizardHandler();

    void updateParagraphsAfterError();

    void notePerformOfOperation(enum OPERATION_TYPE type);


    Ui::MainWindow *ui;
    std::mutex measureMutex;
    QSharedPointer <Theodolite> theodolite1;
    QSharedPointer <Theodolite> theodolite2;
    Dialog* starThdDialog;
    QString reportFilename;
    QString protocolFilename;
    QString directoryName;
    QString wizardFilename;
    QSettings* settings;
    qint32 numMeasures = 0;
    WizardParagraphs ThdWizardParagraphs; //структура содержащая пункты инструкции и номер текущего пункта
    QPushButton* currentActionPushButton; // кнопка, которая должна быть нажата для выполения текущей инструкции
    bool wizardError = false;    //ошибка считывания файла инструкций
    bool measuresStarted = false;    //начаты ли измерений с записью в файл
    bool passNext = false;   //выполнена ли текущая операция, можно ли двигаться дальше
    qint32 needToCompeleOperation = 0;   //тип операции, которую нужно выполнить



signals:
    void readyDataFromThread(DFT data);


};









#endif // MAINWINDOW_H

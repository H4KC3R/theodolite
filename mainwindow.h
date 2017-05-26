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
    QString device_name;
    MeasuresFromTheodolite measure_vector;
    GMS mean_Hz;
    GMS mean_V;
    GMS sko_Hz;
    GMS sko_V;

    bool notContainsMeasures() const
    {
        return measure_vector.isEmpty();
    }
};
Q_DECLARE_METATYPE(DataFromThread)



struct DataFromThreadtoProtocol
{
    GMS sko_hz;
    GMS sko_v;
    QString protocol_text;

    bool isEmpty() const
    {
        return sko_hz.isEmpty()&&sko_v.isEmpty()&&protocol_text.isEmpty();
    }
};

//структура содержащая пункты инструкции и номер текущего пункта
struct WizardParagraphs
{
    QVector<QString> paragraphs;
    quint16 curr_num = 0;
    void clear()
    {
        paragraphs.clear();
        curr_num = 0;
    }

    bool isEmpty() const
    {
        return paragraphs.isEmpty()&&!curr_num;
    }
};



struct MeasureCharacteristics
{
    double sko_hz;
    double sko_v;
    double EV_hz;
    double EV_v;
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
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


private:
    void setStyle();
    void connectTheodolit();
    void makeMeasureNoSave();
    void makeMeasureAndSave();
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

    MeasureCharacteristics calculateMeasureCharact(MeasuresFromTheodolite measure_vector);

    void dataFromThreadHandler(const DFT& data);
    bool prepareParralelMSeasure();
    void parallelConnection();
    void reportAboutConnection(QString message);
    void reportAboutError (QString er_message);

    void setDataFromThread(const DFT& data, QTextEdit* text_edit, QLineEdit* meanHz_line_edit, QLineEdit* meanV_line_edit, QLineEdit* skoHz_line_edit
                              , QLineEdit* skoV_line_edit, QLineEdit*SKO3xHz_line_edit, QLineEdit*SKO3xV_line_edit);
    DFTP parralelMakeMeasureCheck(QSharedPointer<Theodolite>theodolite, QComboBox*combo_box, const int&count_of_measures);
    void parallelMakeMeasureUncheck(QSharedPointer<Theodolite>theodolite, QComboBox*combo_box, const int&count_of_measures, bool do_save);

    void resetOrientation(QSharedPointer<Theodolite>, QLineEdit*, QTextEdit*);
    void makeMeasure(bool do_save,QSharedPointer<Theodolite> theodolite,QTextEdit* text_edit,QComboBox* combo_box
                     ,QLineEdit* meanHz_line_edit,QLineEdit* meanV_line_edit,QLineEdit* skoHz_line_edit
                     ,QLineEdit* skoV_line_edit,QLineEdit*SKO3xHz_line_edit,QLineEdit*SKO3xV_line_edit);

    void saveStarThdData(starThdData star_data);
    void saveProtocolText(const QString& protocol_text);
    void saveSettings();
    void loadSettings();

    void thdWizardHandler();
    void updateParagraphsAfterError();
    void notePerformOfOperation(enum OPERATION_TYPE type);


    Ui::MainWindow *ui;
    std::mutex measure_mutex;
    QSharedPointer <Theodolite> theodolite_1;
    QSharedPointer <Theodolite> theodolite_2;
    Dialog* starThdDialog;
    QString report_filename;
    QString protocol_filename;
    QString directory_name;
    QString wizard_filename;
    QSettings* settings;
    quint16 num_of_measures = 0;
    WizardParagraphs ThdWizardParagraphs; //структура содержащая пункты инструкции и номер текущего пункта
    QPushButton* currentActionPushButton; // кнопка, которая должна быть нажата для выполения текущей инструкции
    bool wizard_error = false;    //ошибка считывания файла инструкций
    bool measures_started = false;    //начаты ли измерений с записью в файл
    bool pass_next = false;   //выполнена ли текущая операция, можно ли двигаться дальше
    quint16 need_to_complete_operation = 0;   //тип операции, которую нужно выполнить





signals:
    void readyDataFromThread(DFT data);


};









#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QString filePathGlobal;
    QString comFileNameGlobal, comFileNameOptFreqGlobal;
    QString comDescriptorGlobal, comDescriptorOptFreqGlobal;
    QString userNameGlobal;
    QString radialDistOptFreqG;
    QString globalSPFileName;
    QString comDescriptorSPGlobal;
    QString globalAtom1, globalAtom2;
    QString globalRadialDistanceSP;
    QString multiplicity1Global;
    QString multiplicity2Global;
    QString comFileNameSingle1Global, comFileNameSingle2Global;
    QString comDescriptorSingle1Global, comDescriptorSingle2Global;
    QString molecularMultiplicityGlobal;
    QString globalMethod;
    int indivIterationCountGlobal;

    //QString comFileNameSPGlobal;
    int globalSPCounter;


private slots:

    void on_pushButton1_clicked();

    void on_pushButton2_clicked();

    void on_buttonStart_clicked();

    void on_atom1Edit_textEdited(const QString &arg1);

    void on_atom2Edit_textEdited(const QString &arg1);

    void on_atom1Edit_textChanged(const QString &arg1);

    void on_label_17_linkActivated(const QString &link);

    void on_pushButton_clicked();

    void performIndividualSP(QString filePath, QString comFileName, QString comDescriptor, QString userName, QString atom1, QString multiplicity);

    void performSP(QString filePath, QString comFileName, QString comDescriptor, QString radialDistance, QString userName, QString atom1, QString atom2);

    void performPES(QString filePath, QString comFileName, QString comDescriptor, QString radialDistance, QString userName, QString atom1, QString atom2, QString initialValue, QString numSteps, QString stepSize);

    void performOptFreq(QString filePath, QString comFileName, QString comDescriptor, QString radialDistance, QString userName, QString atom1, QString atom2);

    void on_horizontalSlider_valueChanged(int value);

    void on_radialDistSlider_valueChanged(int value);

    int checkIfDone();

    int makeFile(QString fName, QString fContent);

    QString getUser();

    QString sendCmd(QString myCmd);

    void on_atom1Multiplicity_textChanged(const QString &arg1);

    void on_dropDownMethod_currentIndexChanged(int index);

    void on_buttonReset_clicked();

public slots:
    void mutex1();          //mutex thread for Pot'l Energy Scan (PES)
    void mutexOptFreq();    //mutex thread for OptFreq
    void mutexSP();         //mutex thread for Single Point
    void mutexIndiv();      //mutex thread for Individual

private:
    Ui::MainWindow *ui;
    QTimer *timer1;
    QTimer *timerOptFreq;
    QTimer *timerSP;        //timer for SP
    QTimer *timerSingle;   //timer for indiv atoms
    QMovie *movie;

};
#endif // MAINWINDOW_H

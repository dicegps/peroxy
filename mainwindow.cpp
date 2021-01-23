/*****************************************************************************************************
 * ###################################################################################################
 *
 * Copyright 2021 diceGPS
 * All Rights Reserved
 *
 * Licensed under the MIT License
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * AMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * To be used for educational purposes only
 *  - Not recommended for use in industry -
 *
 * *Note: that this software is customized for on HPC systems with Sbatch/Slurm
 *
 *
 *
 *
 * #############################################################################################
 * ********************************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QProcess"
#include "QProcessEnvironment"
#include "QFile"
#include "QTextStream"
#include "QDir"
#include "QThread"
#include "QMessageBox"
#include "definitions.h"
#include "QTimer"
#include "clockthread1.h"
#include <QMovie>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //initialize global variables
    radialDistOptFreqG = "0.5";
    globalSPCounter = 0;
    globalSPFileName = "";
    globalMethod = "b3lyp/6-31g*";
    userNameGlobal = getUser();
    filePathGlobal = "/home/" + userNameGlobal +"/3411L/PeroxyOutput/";
    indivIterationCountGlobal = 0;

    timer1 = new QTimer(this);
    connect(timer1, SIGNAL(timeout()),this,SLOT(mutex1()));

    timerOptFreq = new QTimer(this);
    connect(timerOptFreq, SIGNAL(timeout()),this,SLOT(mutexOptFreq()));

    timerSP = new QTimer(this);
    connect(timerSP, SIGNAL(timeout()),this,SLOT(mutexSP()));

    timerSingle = new QTimer(this);
    connect(timerSingle, SIGNAL(timeout()),this,SLOT(mutexIndiv()));

    movie = new QMovie(":/resources/img/loadingicon.gif");

    ui->loadingLabel->setMovie(movie);
    movie->start();
    movie->stop();
    setWindowIcon(QIcon(":/resources/img/atomicon2.png"));


}

MainWindow::~MainWindow()
{
    delete ui;
}


int makedir(QString dirPath)
{
    if (!QDir(dirPath).exists())
    {
        QDir().mkdir(dirPath);
        return 1;
    }
    else
    {
        return 0;
    }
}

int MainWindow::makeFile(QString fName, QString fContent)
{

    QFile file("/home/"+userNameGlobal+"/3411L/PeroxyOutput/" + fName);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        // text stream
        QTextStream stream(&file);

        stream << fContent;

        file.close();
        return 1;
    }
    return 0;
}

QString MainWindow::getUser()
{
    QProcess *proc = new QProcess();
    proc->start("whoami");      //actual
    proc->waitForFinished();

    QString result=proc->readAllStandardOutput();
    result = result.mid(0, result.length()-1);
    return result;
}

QString getInfo(QString filePath, QString fileName, QString wildcard)
{
    QFile myFile(filePath + fileName);
    QTextStream in (&myFile);
    QString line;
    if(myFile.exists() && myFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        do {
            line = in.readLine();
            if (line.contains(wildcard, Qt::CaseSensitive)) {
                myFile.close();
                return line;
            }
        } while (!line.isNull());
    }
    else
        return "File Error 233";
}

QString getInfoOptFreq(QString filePath, QString fileName, QString wildcard)
{
    QFile myFile(filePath + fileName);
    QTextStream in (&myFile);
    QString line;
    QString buf1;
    int lineCount = 0;
    bool found = false;

    if(myFile.exists() && myFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        do {
            line = in.readLine();
            if (line.contains(wildcard, Qt::CaseSensitive)) {
                //myFile.close();

                //return line;
                found = true;

            }
            if (found)
            {
                if (lineCount <3)
                {
                    if (lineCount == 0) buf1 = buf1 + "\n" + line +  " (cm-1)";
                    if (lineCount == 1) buf1 =  buf1 + "\n" + line + " (AMU)";
                    if (lineCount == 2) buf1 = buf1 + "\n" + line + " (mDyne/A)";


                    lineCount++;
                }
                else
                {
                    myFile.close();
                    return buf1;
                }
            }
        } while (!line.isNull());
    }
    else
        return "File Error 233";
}

QString getInfoPES(QString filePath, QString fileName, QString wildcard)
{
    QFile myFile(filePath + fileName);
    QTextStream in (&myFile);
    QString line;
    QString buf1;
    int lineCount = 0;
    bool found = false;

    if(myFile.exists() && myFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        do {
            line = in.readLine();
            if (line.contains(wildcard, Qt::CaseSensitive)) {
                //myFile.close();

                //return line;
                found = true;

            }
            if (found)
            {
                if (lineCount <13)
                {
                    buf1 = buf1 + "\n" + line;
                    lineCount++;
                }
                else
                {
                    myFile.close();
                    return buf1;
                }
            }
        } while (!line.isNull());
    }
    else
        return "File Error 233";
}

QString MainWindow::sendCmd(QString myCmd)
{
    // QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QProcess *proc = new QProcess();
    proc->setWorkingDirectory("/home/"+userNameGlobal+"/3411L/PeroxyOutput/");
    proc->start(myCmd);      //actual
    proc->waitForFinished();

    QString result=proc->readAllStandardOutput();
    return result;

}


QString sbatch(QString workPath)
{
    // QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QProcess *proc = new QProcess();
    proc->setWorkingDirectory(workPath);
    proc->start("/home/share/slurm/19.05.3/bin/sbatch g09.sub");      //actual
    proc->waitForFinished();

    QString result=proc->readAllStandardOutput();
    return result;
    //return "Success";
}


void MainWindow::on_pushButton1_clicked()
{
    ui->textEdit1->setText(sbatch("/home/"+userNameGlobal+"/3411L/PeroxyOutput"));
}

void MainWindow::performOptFreq(QString filePath, QString comFileName, QString comDescriptor, QString radialDistance, QString userName, QString atom1, QString atom2)
{
    QString squeueOut = "";
    QString magicLine = "";
    QString parsedOutput = "";

    makedir(filePath);
    makeFile(comFileName + ".com","%NProcShared=2\n#"+globalMethod+" opt freq\n\nOpt Freq (charge, multiplicity)\n\n0 "+ molecularMultiplicityGlobal + "\n"+atom1+"\n"+atom2+" 1 R\n\nR="+radialDistance+"\n\n");
    makeFile("g09.sub","#!/bin/bash\n#SBATCH --qos pq_chm3411l\n#SBATCH --account acc_chm3411l\n#SBATCH --partition classroom\n#SBATCH -N 1\n#SBATCH --cpus-per-task=2\n#SBATCH --job-name=" + comDescriptor + "\n#SBATCH --output=out_%j.log\n\n. $MODULESHOME/../global/profile.modules\nmodule load singularity-3.5.3\nmodule load gaussian-09\nexport OMP_NUM_THREADS=$SLURM_CPUS_ON_NODE\nexport gName=`id -gn $USER`\npid=${gName##*_}\n\nif [ ! -d \"/scratch/$pid/gaussianScratch\" ]; then\n  echo \"Gaussian Scratch directory doesn't exist. Creating directory...\"\n  mkdir /scratch/$pid/gaussianScratch\n  chmod g+w /scratch/$pid/gaussianScratch\nfi\nexport GAUSS_SCRDIR=/scratch/$pid/gaussianScratch\n\npwd; hostname; date\n \necho \"Running program on $SLURM_CPUS_ON_NODE CPU cores\"\n\nSINGULARITYENV_GAUSS_SCRDIR=/scratch/$pid/gaussianScratch singularity exec -e /home/slurmsample/singularity/g09.sif g09 " + comFileName + "\n\ndate");
    sbatch("/home/"+userNameGlobal+"/3411L/PeroxyOutput");

    filePathGlobal = filePath;
    comFileNameOptFreqGlobal = comFileName;
    comDescriptorOptFreqGlobal = comDescriptor;
    timerOptFreq->start(1000);
}

void MainWindow::performIndividualSP(QString filePath, QString comFileName, QString comDescriptor, QString userName, QString atom1, QString multiplicity)
{
    QString squeueOut = "";
    QString magicLine = "";
    QString parsedOutput = "";

    makedir(filePath);
    makeFile(comFileName + ".com","%NProcShared=2\n#"+globalMethod+"\n\nIndividual Single Point Calculation (charge, multiplicity)\n\n0 "+multiplicity+"\n"+atom1+"\n\n");
    makeFile("g09.sub","#!/bin/bash\n#SBATCH --qos pq_chm3411l\n#SBATCH --account acc_chm3411l\n#SBATCH --partition classroom\n#SBATCH -N 1\n#SBATCH --cpus-per-task=2\n#SBATCH --job-name=" + comDescriptor + "\n#SBATCH --output=out_%j.log\n\n. $MODULESHOME/../global/profile.modules\nmodule load singularity-3.5.3\nmodule load gaussian-09\nexport OMP_NUM_THREADS=$SLURM_CPUS_ON_NODE\nexport gName=`id -gn $USER`\npid=${gName##*_}\n\nif [ ! -d \"/scratch/$pid/gaussianScratch\" ]; then\n  echo \"Gaussian Scratch directory doesn't exist. Creating directory...\"\n  mkdir /scratch/$pid/gaussianScratch\n  chmod g+w /scratch/$pid/gaussianScratch\nfi\nexport GAUSS_SCRDIR=/scratch/$pid/gaussianScratch\n\npwd; hostname; date\n \necho \"Running program on $SLURM_CPUS_ON_NODE CPU cores\"\n\nSINGULARITYENV_GAUSS_SCRDIR=/scratch/$pid/gaussianScratch singularity exec -e /home/slurmsample/singularity/g09.sif g09 " + comFileName + "\n\ndate");
    sbatch("/home/"+userNameGlobal+"/3411L/PeroxyOutput");

    timerSingle->start(1000);
}



void MainWindow::performSP(QString filePath, QString comFileName, QString comDescriptor, QString radialDistance, QString userName, QString atom1, QString atom2)
{
    QString squeueOut = "";
    QString magicLine = "";
    QString parsedOutput = "";

    makedir(filePath);
    makeFile(comFileName + ".com","%NProcShared=2\n#"+globalMethod+"\n\nSingle Point Calculation (charge, multiplicity)\n\n0 " + molecularMultiplicityGlobal +"\n"+atom1+"\n"+atom2+" 1 R\n\nR="+radialDistance+"\n\n");
    makeFile("g09.sub","#!/bin/bash\n#SBATCH --qos pq_chm3411l\n#SBATCH --account acc_chm3411l\n#SBATCH --partition classroom\n#SBATCH -N 1\n#SBATCH --cpus-per-task=2\n#SBATCH --job-name=" + comDescriptor + "\n#SBATCH --output=out_%j.log\n\n. $MODULESHOME/../global/profile.modules\nmodule load singularity-3.5.3\nmodule load gaussian-09\nexport OMP_NUM_THREADS=$SLURM_CPUS_ON_NODE\nexport gName=`id -gn $USER`\npid=${gName##*_}\n\nif [ ! -d \"/scratch/$pid/gaussianScratch\" ]; then\n  echo \"Gaussian Scratch directory doesn't exist. Creating directory...\"\n  mkdir /scratch/$pid/gaussianScratch\n  chmod g+w /scratch/$pid/gaussianScratch\nfi\nexport GAUSS_SCRDIR=/scratch/$pid/gaussianScratch\n\npwd; hostname; date\n \necho \"Running program on $SLURM_CPUS_ON_NODE CPU cores\"\n\nSINGULARITYENV_GAUSS_SCRDIR=/scratch/$pid/gaussianScratch singularity exec -e /home/slurmsample/singularity/g09.sif g09 " + comFileName + "\n\ndate");
    sbatch("/home/"+userNameGlobal+"/3411L/PeroxyOutput");


    globalSPFileName = comFileName;
    //filePathGlobal = filePath;
    comDescriptorSPGlobal = comDescriptor;

    if (globalSPCounter == 0) timerSP->start(1000);

}

void MainWindow::performPES(QString filePath, QString comFileName, QString comDescriptor, QString radialDistance, QString userName, QString atom1, QString atom2, QString initialValue, QString numSteps, QString stepSize)
{
    QString squeueOut = "";
    QString magicLine = "";
    QString parsedOutput = "";

    makedir(filePath);
    makeFile(comFileName + ".com","%NProcShared=2 \n#"+globalMethod+" opt=modredundant\n\nPotential Energy Scan (charge, multiplicity)\n\n0 " + molecularMultiplicityGlobal +"\n"+atom1+"\n"+atom2+" 1 R\n\nR="+initialValue+"\n\n1 2 s "+numSteps+" " + stepSize +"\n\n");
    makeFile("g09.sub","#!/bin/bash\n#SBATCH --qos pq_chm3411l\n#SBATCH --account acc_chm3411l\n#SBATCH --partition classroom\n#SBATCH -N 1\n#SBATCH --cpus-per-task=2\n#SBATCH --job-name=" + comDescriptor + "\n#SBATCH --output=out_%j.log\n\n. $MODULESHOME/../global/profile.modules\nmodule load singularity-3.5.3\nmodule load gaussian-09\nexport OMP_NUM_THREADS=$SLURM_CPUS_ON_NODE\nexport gName=`id -gn $USER`\npid=${gName##*_}\n\nif [ ! -d \"/scratch/$pid/gaussianScratch\" ]; then\n  echo \"Gaussian Scratch directory doesn't exist. Creating directory...\"\n  mkdir /scratch/$pid/gaussianScratch\n  chmod g+w /scratch/$pid/gaussianScratch\nfi\nexport GAUSS_SCRDIR=/scratch/$pid/gaussianScratch\n\npwd; hostname; date\n \necho \"Running program on $SLURM_CPUS_ON_NODE CPU cores\"\n\nSINGULARITYENV_GAUSS_SCRDIR=/scratch/$pid/gaussianScratch singularity exec -e /home/slurmsample/singularity/g09.sif g09 " + comFileName + "\n\ndate");
    sbatch("/home/"+userNameGlobal+"/3411L/PeroxyOutput");


    filePathGlobal = filePath;
    comFileNameGlobal = comFileName;
    comDescriptorGlobal = comDescriptor;
    timer1->start(1000);


    //return "Success";
}

void MainWindow::on_pushButton2_clicked()
{

}

QString compoundName(QString atom1, QString atom2)
{
    QString compoundedName = "";
    if (atom1 == atom2)
    {
        compoundedName = atom1 + "2_";
    }
    else
    {
        compoundedName = atom1 + atom2 + "_";
    }

    return compoundedName;
}

void MainWindow::on_buttonStart_clicked()
{
    int atomicMult1 = ui->atom1Multiplicity->value();
    int atomicMult2= ui->atom2Multiplicity->value();
    multiplicity1Global = QString::number(atomicMult1);
    multiplicity2Global = QString::number(atomicMult2);
    molecularMultiplicityGlobal = QString::number((atomicMult1 + atomicMult2) - 3);     //calculate multiplicity for entire molecule system




    //reset all the progressbars
    ui->progressBarSingle->setValue(0);
    ui->progressBarOptFreq->setValue(0);
    ui->progressBarSP->setValue(0);
    ui->progressBarScan->setValue(0);

    //reset progressbar color
    /*
     *  selection-background-color: rgb(255, 0, 127);              //purple
        selection-background-color: rgb(85, 170, 0);    //green
     */
    ui->progressBarSingle->setStyleSheet("selection-background-color: rgb(255, 0, 127)");
    ui->progressBarOptFreq->setStyleSheet("selection-background-color: rgb(255, 0, 127)");
    ui->progressBarSP->setStyleSheet("selection-background-color: rgb(255, 0, 127)");
    ui->progressBarScan->setStyleSheet("selection-background-color: rgb(255, 0, 127)");

    //clear all textboxs:
    ui->textEdit1->clear();
    ui->textEdit2->clear();
    ui->textEditAtom1Energy->clear();
    ui->textEditAtom2Energy->clear();
    ui->textEditOptFreq->clear();


    //Define Possible Elements from #1-83 (Bismuth)
    //QString possibleElements = ",Ag,Al,Ar,As,Au,B,Ba,Be,Bi,Br,C,Ca,Cd,Ce,Cl,Co,Cr,Cs,Cu,Dy,Er,Eu,F,Fe,Ga,Gd,Ge,H,He,Hf,Hg,Ho,I,In,Ir,K,Kr,La,Li,Lu,Mg,Mn,Mo,N,Na,Nb,Nd,Ne,Ni,O,Os,P,Pb,Pd,Pm,Pr,Pt,Rb,Re,Rh,Ru,S,Sb,Sc,Se,Si,Sm,Sn,Sr,Ta,Tb,Tc,Te,Ti,Tl,Tm,V,W,X,Y,Yb,Zn,Zr,";

    QString atom1 = ui->atom1Edit->text();
    QString atom2 = ui->atom2Edit->text();
    globalAtom1 = atom1;
    globalAtom2 = atom2;
    indivIterationCountGlobal = 0;

    QString comFileName = "";
    QString comDescriptor ="";
    QString radialDistance = "";
    QString filePath = "/home/"+userNameGlobal+"/3411L/PeroxyOutput/";
    QString parsedOutput1, parsedOutput2, parsedOutput3, parsedOutput4, parsedOutputPES, parsedOutputOptFreq;
    QString parsedOutputIndiv;


    if (possibleElements.contains("," + atom1 + "-") && possibleElements.contains("," + atom2 + "-"))
    {
        //Main Section Here:


        if (ui->checkBoxSP->isChecked())        //Perform SP calc?? ****************************************************
        {
            //first Round


            //round 1 - 0.5A
            comDescriptor = "sp";
            globalRadialDistanceSP = "0.5";
            globalSPFileName = compoundName(atom1, atom2) + "SP1";
            globalAtom1 = atom1;
            globalAtom2 = atom2;
            globalSPCounter = 0;
            performSP(filePathGlobal,globalSPFileName,comDescriptor,globalRadialDistanceSP,userNameGlobal, globalAtom1, globalAtom2);

        }

        //is PES Scan checkbox checked?? #####################################################################
        if (ui->checkBoxPES->isChecked())
        {
            QString initialValue = QString::number(ui->spinBoxInitial->value());
            QString numSteps = QString::number(ui->spinBoxSteps->value());
            QString stepSize = QString::number(ui->spinBoxSize->value());
            comFileName = compoundName(atom1, atom2) + "PEScan";
            comDescriptor="pes1";


            performPES(filePathGlobal, comFileName, comDescriptor, radialDistance, userNameGlobal, atom1, atom2, initialValue, numSteps, stepSize);

        }


        //is Opt Freq checkbox checked?? ####################################################################
        if (ui->checkBoxOptFreq->isChecked())
        {
            comDescriptor = "optf1";
            comFileName = compoundName(atom1, atom2) + "OptFreq";

            performOptFreq(filePath,comFileName,comDescriptor,radialDistOptFreqG,userNameGlobal,atom1,atom2);
        }

        //is Individual Atom Analysis checkbox checked? ####################################################
        if (ui->checkBoxIndividual->isChecked())
        {


            comFileNameSingle1Global = globalAtom1 + "_individualSP";
            comFileNameSingle2Global = globalAtom2 + "_individualSP";

            comDescriptorSingle1Global = "indps";
            comDescriptorSingle2Global = "indps2";
            // parsedOutputIndiv = performIndividualSP(filePath,comFileName,comDescriptor,userNameGlobal,atom1,multiplicity1);
            performIndividualSP(filePathGlobal, comFileNameSingle1Global,comDescriptorSingle1Global,userNameGlobal,globalAtom1,multiplicity1Global);


        }

        movie->start();
        ui->buttonStart->setStyleSheet(("background-color: rgb(255, 0, 127)"));
        ui->buttonStart->setText("Processing...");
        ui->buttonStart->setDisabled(true);

    }
    else
    {
        QMessageBox Msgbox;
        Msgbox.setText("Invalid Element Symbol.  Check Your Inputs and Try Again");
        Msgbox.exec();
    }



}

void MainWindow::on_atom1Edit_textEdited(const QString &arg1)
{
    QString tempText = ui->atom1Edit->text();
    QString buffer1 = "";
    if (tempText.length() == 1)
    {
        tempText = tempText.toUpper();
        ui->atom1Edit->setText(tempText);

    }
    ui->atom1Edit->setStyleSheet("QLineEdit { background: rgb(255, 255, 255); }");

    if (possibleElements.contains("," + tempText + "-"))
    {

        if (tempText.length() == 1)
        {
            buffer1 = possibleElements.mid(possibleElements.indexOf("," + tempText + "-")+3, 1);
            ui->atom1Multiplicity->setValue(buffer1.toInt());
            //ui->textEdit2->setText(buffer1);
        }
        if (tempText.length()==2)
        {
            buffer1 = possibleElements.mid(possibleElements.indexOf("," + tempText + "-")+4, 1);
            ui->atom1Multiplicity->setValue(buffer1.toInt());
            //ui->textEdit2->setText(buffer1);
        }
        //
        ui->molecularMultiplicityBox->setValue((ui->atom1Multiplicity->value()+ui->atom2Multiplicity->value())-3);
    }
    else
    {
        ui->atom1Edit->setStyleSheet("QLineEdit { background: rgb(255, 0, 0); }");
    }

}

void MainWindow::on_atom2Edit_textEdited(const QString &arg1)
{
    QString tempText = ui->atom2Edit->text();
    QString buffer1 = "";
    if (tempText.length() == 1)
    {
        tempText = tempText.toUpper();
        ui->atom2Edit->setText(tempText);
    }

    ui->atom2Edit->setStyleSheet("QLineEdit { background: rgb(255, 255, 255); }");

    if (possibleElements.contains("," + tempText + "-"))
    {
        if (tempText.length() == 1)
        {
            buffer1 = possibleElements.mid(possibleElements.indexOf("," + tempText + "-")+3, 1);
            ui->atom2Multiplicity->setValue(buffer1.toInt());
            //ui->textEdit2->setText(buffer1);
        }
        if (tempText.length()==2)
        {
            buffer1 = possibleElements.mid(possibleElements.indexOf("," + tempText + "-")+4, 1);
            ui->atom2Multiplicity->setValue(buffer1.toInt());
            //ui->textEdit2->setText(buffer1);
        }
        ui->molecularMultiplicityBox->setValue((ui->atom1Multiplicity->value()+ui->atom2Multiplicity->value())-3);
    }
    else
    {
        ui->atom2Edit->setStyleSheet("QLineEdit { background: rgb(255, 0, 0); }");
    }
}

void MainWindow::on_atom1Edit_textChanged(const QString &arg1)
{

}

void MainWindow::on_label_17_linkActivated(const QString &link)
{
    QMessageBox Msgbox;
    Msgbox.setText("Ready");
    Msgbox.exec();
}

void MainWindow::on_pushButton_clicked()
{
    //QLabel *processLabel = new QLabel(this);


    movie->start();
}

void MainWindow::mutex1()
{
    QString squeueOut = sendCmd("/home/share/slurm/19.05.3/bin/squeue -u " + userNameGlobal);
    if (squeueOut.contains(comDescriptorGlobal))
    {
        if (ui->progressBarScan->value() <98)
            ui->progressBarScan->setValue(ui->progressBarScan->value()+1);
    }
    else
    {
        timer1->stop();
        ui->progressBarScan->setValue(100);
        ui->progressBarScan->setStyleSheet("selection-background-color: rgb(85, 170, 0);");
        QString rawData = getInfoPES(filePathGlobal,comFileNameGlobal + ".log", "Surface Scan");
        ui->textEdit1->setText(rawData);

        if (checkIfDone())
        {
            movie->stop();
            ui->loadingLabel->setMovie(movie);
        }

    }

}

void MainWindow::mutexOptFreq()
{
    QString squeueOut = sendCmd("/home/share/slurm/19.05.3/bin/squeue -u " + userNameGlobal);
    if (squeueOut.contains(comDescriptorOptFreqGlobal))
    {
        if (ui->progressBarOptFreq->value() <98)
            ui->progressBarOptFreq->setValue(ui->progressBarOptFreq->value()+1);
    }
    else
    {
        timerOptFreq->stop();
        ui->progressBarOptFreq->setValue(100);
        ui->progressBarOptFreq->setStyleSheet("selection-background-color: rgb(85, 170, 0);");
        QString rawData = getInfoOptFreq(filePathGlobal,comFileNameOptFreqGlobal + ".log", "Frequencies -- ");
        ui->textEditOptFreq->setText(rawData);

        if (checkIfDone())
        {
            movie->stop();
            ui->loadingLabel->setMovie(movie);
        }

    }
}

void MainWindow::mutexSP()
{
    QString squeueOut = sendCmd("/home/share/slurm/19.05.3/bin/squeue -u " + userNameGlobal);
    if (squeueOut.contains(comDescriptorSPGlobal))
    {
        if (ui->progressBarSP->value() <98)
            ui->progressBarSP->setValue(ui->progressBarSP->value()+1);
    }
    else
    {

        globalSPCounter++;
        QString suffix = QString::number(globalSPCounter);

        QString rawData = getInfo(filePathGlobal,globalSPFileName + ".log", "SCF Done");
        QString parsedOutput = globalRadialDistanceSP + "Å: " + rawData.mid(rawData.indexOf("=")+2, 15);


        comDescriptorSPGlobal = comDescriptorSPGlobal + suffix;
        globalSPFileName = globalSPFileName + suffix;


        switch(globalSPCounter)
        {
        case 1:
            globalRadialDistanceSP = "1.0";
            break;
        case 2:
            globalRadialDistanceSP = "1.5";
            break;
        case 3:
            globalRadialDistanceSP = "2.0";
            break;
        }
        ui->textEdit2->setText(ui->textEdit2->toPlainText() + parsedOutput + "\n");
        if (globalSPCounter < 4)
        {

            performSP(filePathGlobal,globalSPFileName,comDescriptorSPGlobal,globalRadialDistanceSP,userNameGlobal,globalAtom1,globalAtom2);

        }
        else
        {
            ui->progressBarSP->setValue(100);
            ui->progressBarSP->setStyleSheet("selection-background-color: rgb(85, 170, 0);");
            timerSP->stop();

            if (checkIfDone())
            {
                movie->stop();
                ui->loadingLabel->setMovie(movie);
            }
        }

    }
}

void MainWindow::mutexIndiv()
{
    QString squeueOut = sendCmd("/home/share/slurm/19.05.3/bin/squeue -u " + userNameGlobal);
    if (squeueOut.contains(comDescriptorSingle1Global) or squeueOut.contains(comDescriptorSingle2Global))
    {
        if (ui->progressBarSingle->value() <98)
            ui->progressBarSingle->setValue(ui->progressBarSingle->value()+2);
    }
    else
    {

        //globalSPCounter++;
        QString suffix = QString::number(globalSPCounter);
        QString rawData;

        if (indivIterationCountGlobal == 0) rawData = getInfo(filePathGlobal,comFileNameSingle1Global + ".log", "SCF Done");
        if (indivIterationCountGlobal == 2) rawData = getInfo(filePathGlobal,comFileNameSingle2Global + ".log", "SCF Done");

        QString parsedOutput = rawData.mid(rawData.indexOf("=")+2, 15);
        if ((globalAtom1 == globalAtom2) && (indivIterationCountGlobal == 0))
        {
            ui->textEditAtom1Energy->setText(parsedOutput);
            ui->textEditAtom2Energy->setText(parsedOutput);
            ui->progressBarSingle->setValue(100);
            ui->progressBarSingle->setStyleSheet("selection-background-color: rgb(85, 170, 0);");
            timerSingle->stop();

            if (checkIfDone())
            {
                movie->stop();

                ui->loadingLabel->setMovie(movie);
            }
        }

        if ((globalAtom1 != globalAtom2) && (indivIterationCountGlobal == 0))
        {
            ui->textEditAtom1Energy->setText(parsedOutput);
        }

        if (indivIterationCountGlobal ==1 )
        {
            performIndividualSP(filePathGlobal,comFileNameSingle2Global,comDescriptorSingle2Global,userNameGlobal,globalAtom2,multiplicity2Global);
        }

        if (indivIterationCountGlobal == 2)
        {
            ui->textEditAtom2Energy->setText(parsedOutput);

            ui->progressBarSingle->setValue(100);
            ui->progressBarSingle->setStyleSheet("selection-background-color: rgb(85, 170, 0);");
            timerSingle->stop();

            if (checkIfDone())
            {
                movie->stop();

                ui->loadingLabel->setMovie(movie);
            }
        }

        indivIterationCountGlobal++;

    }
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{

}

void MainWindow::on_radialDistSlider_valueChanged(int value)
{
    int val = ui->radialDistSlider->value();
    if (val == 1)   radialDistOptFreqG = "0.5";
    if (val == 2)   radialDistOptFreqG = "1.0";
    if (val == 3)   radialDistOptFreqG = "1.5";
    if (val == 4)   radialDistOptFreqG = "2.0";
    ui->radialDistLabel->setText(radialDistOptFreqG + "Å");
}

int MainWindow::checkIfDone()
{
    if ((ui->progressBarOptFreq->value() == 100) && (ui->progressBarSP->value() == 100) && (ui->progressBarScan->value() == 100) && (ui->progressBarSingle->value() == 100))
    {
        ui->buttonStart->setStyleSheet(("background-color: rgb(85, 170, 0);"));
        ui->buttonStart->setText("Start");
        ui->buttonStart->setDisabled(false);
        return 1;

    }
    else
        return 0;
}

void MainWindow::on_atom1Multiplicity_textChanged(const QString &arg1)
{

}

void MainWindow::on_dropDownMethod_currentIndexChanged(int index)
{
    switch (index)
    {
        case 0:
            globalMethod = "b3lyp/6-31g*";
        break;
    case 1:
        globalMethod = "b3lyp/6-31g**";
        break;
    }
}

void MainWindow::on_buttonReset_clicked()
{
    //MainWindow::setWindowIcon(QIcon(":/resources/img/playbtnscaled.png"));

}

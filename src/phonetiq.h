#ifndef PHONETIQ_H
#define PHONETIQ_H

#include <QWidget>
#include <QLineEdit>
#include <QTextBrowser>
#include <QGridLayout>
#include <QMainWindow>
#include <QMenuBar>
#include <QDir>
#include <QMessageBox>
#include <QBuffer>
#include <QXmlSimpleReader>
#include <QTcpSocket>
#include <QApplication>
#include <QProgressBar>
#include <QProcess>

#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include <QCheckBox>
#include <QPixmap>
#include <QSettings>

#ifdef QTOPIA
#include <QSoftMenuBar>
#include <QtopiaApplication>
#endif

#include "JarScanner.h"

#define UNCOMPARABLE_CHARS 0x12345678

struct VmStat
{
    bool vmInstalled;
    bool libsInstalled;
};

class VmTab : public QWidget
{
    Q_OBJECT
public:
    VmTab(QWidget *parent = 0);
    VmStat checkVm(bool show_msg);
private:
    QPushButton *bInstall, *bLibs, *bRemove, *bCheck, *bDemo;
    QLabel *lStatus;
    QGridLayout *layout;
    QProgressBar *progress;
    void showErr(QString);
    bool download(QString url, QString destPath, QString filename);
    bool untar(QString file, QString display_fname, QString dest_dir);
    void setStatus(QString txt);
    bool checkLibs(QString &msg);
    bool instlibs(QString msg);
private slots:
    void installClicked();
    void libsClicked();
    void removeClicked();
    void checkClicked();
    void demoClicked();
};

class MidletsTab : public QWidget
{
    Q_OBJECT
public:
    MidletsTab(QWidget *parent = 0);
private:
    QListWidget *lw; //
    QPushButton *bRefresh;
    QLabel *lStatus;
    QGridLayout *layout;
    QCheckBox *chb_qvga;
    QCheckBox *chb_fix;
    QCheckBox *chb_quitonhide;
    QPushButton *bStop;
    JarScanner *js;
    bool abort;
    void updateConfig(bool qvga, bool fix_scr, bool quitonhide);
    void showErr(QString); //
private slots:
    void refreshClicked(); //
    void stopClicked();
    void runMidlet();
    void removeMidlet();
    void installMidlet();
    void jarItemChosen(QString &fname);
};

class PhonetiqMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    PhonetiqMainWindow(QWidget* parent = 0, Qt::WindowFlags f = 0);

private:
    QAction *runAction, *removeAction, *installAction;

    QTabWidget *tabWidget;
    VmTab *tabVm;
    MidletsTab *tabMidlets;
    //QGridLayout *layout;
    QLineEdit *edit;
    QTextBrowser *browser;
    QProgressBar *progress;
    QFile dictFile;

private slots:
    void menuShow();
};

#endif

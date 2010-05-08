#include "phonetiq.h"

#define BASE_DIR Qtopia::packagePath() + "/"
#define JAVA_DIR QString("java/")

#define ARCHIVE_URL QString("http://dl.linuxphone.ru/openmoko/qtmoko/packages/phoneme-neo_latest_armel.tar.gz")
#define ARCHIVE_FILE QString("phoneme-neo_latest_armel.tar.gz")

#define DEMO_URL QString("http://touchsenku.googlecode.com/files/Generic-AnyPhone-touchSenku.jar")
#define DEMO_FILE QString("Generic-AnyPhone-touchSenku.jar")

PhonetiqMainWindow::PhonetiqMainWindow(QWidget* parent, Qt::WindowFlags f) : QMainWindow(parent, f)
{
    setWindowTitle("PhonetiQ");

#ifdef QTOPIA
    QMenu* m = QSoftMenuBar::menuFor(this);
#else
    QMenu *m = menuBar()->addMenu("&File");
    resize(480, 640);
#endif

    //--------------------------------------

    tabWidget = new QTabWidget(this);
    tabVm = new VmTab(this);
    tabMidlets = new MidletsTab(this);
    tabWidget->addTab(tabVm, "PhoneME");
    tabWidget->addTab(tabMidlets, "MIDlets");

    VmStat vs = tabVm->checkVm(false);
    if ((vs.vmInstalled) && (vs.libsInstalled))
        tabWidget->setCurrentWidget(tabMidlets);

    //--------------------------------------

    runAction = m->addAction(tr("Run midlet"), tabMidlets, SLOT(runMidlet()));
    removeAction = m->addAction(tr("Remove midlet"), tabMidlets, SLOT(removeMidlet()));
    installAction = m->addAction(tr("Install midlet"), tabMidlets, SLOT(installMidlet()));
    m->addAction(tr("Quit"), this, SLOT(close()));

    connect(m, SIGNAL(aboutToShow()), this, SLOT(menuShow()));

    //connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged()));
    //layout = new QGridLayout(this);
    //layout->addWidget(tabWidget);
    //setLayout(layout);
    setCentralWidget(tabWidget);
}

void PhonetiqMainWindow::menuShow()
{
    if (tabWidget->currentIndex() == 1)
    {
        runAction->setVisible(true);
        removeAction->setVisible(true);
        installAction->setVisible(true);
    }
    else
    {
        runAction->setVisible(false);
        removeAction->setVisible(false);
        installAction->setVisible(false);
    }
}

VmStat VmTab::checkVm(bool show_msg)
{
    QString msg, msg1, msg2;

    //QDir qd;
    bool vmInstalled = QFile::exists(BASE_DIR + JAVA_DIR + "/bin/arm/runMidlet");
    if (vmInstalled)
        msg1 = tr("VM is installed");
    else
        msg1 = tr("VM is NOT installed");
    bInstall->setEnabled(!vmInstalled);
    bRemove->setEnabled(vmInstalled);

    QString mlibs;
    bool libsInstalled = checkLibs(mlibs);
    if (libsInstalled)
        msg2 = tr("Dependencies are installed");
    else
        msg2 = tr("Some dependencies are NOT installed:") + " " + mlibs;
    bLibs->setEnabled(!libsInstalled);

    msg = msg1 + "\n" + msg2;
    setStatus(msg);
    if (show_msg)
    {
        QMessageBox::information(this, "PhonetiQ", msg);
    }

    VmStat res;
    res.vmInstalled = vmInstalled;
    res.libsInstalled = libsInstalled;
    return res;
}

bool VmTab::checkLibs(QString &msg)
{
    msg = "";
    QFile qf;
    bool sql = qf.exists("/usr/lib/libsqlite3.so.0");
    if (!sql)
        msg += "libsqlite3-0";
    return sql;
}

void VmTab::setStatus(QString txt)
{
    lStatus->setText(txt);
}

VmTab::VmTab(QWidget *parent)
    : QWidget(parent)
{
    bInstall = new QPushButton(this);
    bInstall->setText(tr("Download and install VM"));
    connect(bInstall, SIGNAL(clicked()), this, SLOT(installClicked()));

    bLibs = new QPushButton(this);
    bLibs->setText(tr("Install dependencies"));
    connect(bLibs, SIGNAL(clicked()), this, SLOT(libsClicked()));

    bRemove = new QPushButton(this);
    bRemove->setText(tr("Uninstall VM"));
    connect(bRemove, SIGNAL(clicked()), this, SLOT(removeClicked()));

    bCheck = new QPushButton(this);
    bCheck->setText(tr("Check VM status"));
    connect(bCheck, SIGNAL(clicked()), this, SLOT(checkClicked()));

    bDemo = new QPushButton(this);
    bDemo->setText(tr("Download demo game"));
    connect(bDemo, SIGNAL(clicked()), this, SLOT(demoClicked()));

    lStatus = new QLabel(this);
    lStatus->setAlignment(Qt::AlignCenter);
    lStatus->setWordWrap(true);
    setStatus("");

    progress = new QProgressBar(this);
    progress->setVisible(false);

    layout = new QGridLayout(this);

    layout->addWidget(bInstall, 0, 0);
    layout->addWidget(bLibs, 1, 0);
    layout->addWidget(bDemo, 2, 0);
    layout->addWidget(bRemove, 3, 0);
    layout->addWidget(bCheck, 4, 0);
    layout->addWidget(lStatus, 5, 0);
    layout->addWidget(progress, 6, 0);

    setLayout(layout);
}

void VmTab::demoClicked()
{
    if(QMessageBox::question(this, "PhonetiQ",
            tr("Do you want to download a demo game 'TouchSenku' from Google Code?"),
            QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
    {
        return;
    }

    this->setEnabled(false);
    progress->setVisible(true);
    if(download(DEMO_URL, QDir::homePath()+"/"+DEMO_FILE, DEMO_FILE))
    {
        QMessageBox::information(this, "PhonetiQ", "The game was downloaded successfully. Please don't forget to install it via 'MIDlets' tab.");
    }
    else
    {
        QMessageBox::information(this, "PhonetiQ", "Download failed.");
    }
    progress->setVisible(false);
    this->setEnabled(true);

    checkVm(false);
}

void VmTab::installClicked()
{
    if(QMessageBox::question(this, "PhonetiQ",
            tr("You are about to download and install PhoneME (Java ME Virtual Machine). Please make sure you have Internet connection and press 'yes' to confirm download (~2MB)."),
            QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
    {
        return;
    }

    this->setEnabled(false);
    progress->setVisible(true);
    if(download(ARCHIVE_URL, BASE_DIR + ARCHIVE_FILE, ARCHIVE_FILE))
    {
        if (untar(BASE_DIR + ARCHIVE_FILE, ARCHIVE_FILE, BASE_DIR))
        {
            libsClicked();
        }
    }
    progress->setVisible(false);
    this->setEnabled(true);

    checkVm(true);
}

void VmTab::libsClicked()
{
    QString msg;
    if (!checkLibs(msg))
    {
        if(QMessageBox::question(this, "PhonetiQ",
            tr("Do you want to install dependencies now via") + " 'apt-get install " + msg + "'?",
            QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {
            this->setEnabled(false);
            progress->setVisible(true);
            instlibs(msg);
            progress->setVisible(false);
            if (!checkLibs(msg))
                QMessageBox::information(this, "PhonetiQ", "Installation of libraries (" + msg + ") has failed. You can try to install them manually.");
            this->setEnabled(true);

            checkVm(true);
        }
    }
}

void VmTab::removeClicked()
{
    if(QMessageBox::question(this, "PhonetiQ",
            tr("Do you want to remove PhoneME (Java ME Virtual Machine)?"),
            QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
    {
        return;
    }

    QProcess rmproc;
    //rm -r /some/dir/
    rmproc.start("rm", QStringList() << "-r" << BASE_DIR + JAVA_DIR);
    if(!rmproc.waitForStarted())
    {
        showErr(tr("Unable to start 'rm'"));
        return;
    }
    while(rmproc.state() == QProcess::Running)
    {
        QApplication::processEvents();
        rmproc.waitForFinished(100);
    }

    checkVm(true);
}

void VmTab::checkClicked()
{
    checkVm(true);
}

void VmTab::showErr(QString err)
{
    QMessageBox::critical(this, "PhonetiQ", err);
}

void MidletsTab::showErr(QString err) //
{
    QMessageBox::critical(this, "PhonetiQ", err);
}

bool VmTab::download(QString url, QString destPath, QString display_fname)
{
    setStatus(tr("Downloading") + " " + display_fname);

    QString host = url;
    QString reqPath;
    int port = 80;

    if(url.startsWith("http://"))
    {
        host.remove(0, 7);
    }

    int colonIndex = host.indexOf(':');
    int slashIndex = host.indexOf('/');
    if(slashIndex < 0)
    {
        return false;
    }
    reqPath = host.right(host.length() - slashIndex).replace(" ", "%20");
    host = host.left(slashIndex);
    if(colonIndex > 0)
    {
        QString portStr = host.right(host.length() - colonIndex - 1);
        host = host.left(colonIndex);
        port = portStr.toInt(0, 10);
    }

connectx:
    QTcpSocket sock(this);
    sock.setReadBufferSize(65535);
    sock.connectToHost(host, port);
    if(!sock.waitForConnected(5000))
    {
        showErr(sock.errorString());
        return false;
    }

    QByteArray req("GET ");
    req.append(reqPath);
    req.append(" HTTP/1.1\r\nHost: ");
    req.append(host);
    req.append(':');
    req.append(QByteArray::number(port));
    req.append("\r\n\r\n");

    sock.write(req);
    sock.flush();
    sock.waitForBytesWritten();

    int contentLen = 0;
    bool html = false;
    QByteArray line;
    for(;;)
    {
        line = sock.readLine();
        if(line.isEmpty())
        {
            if(sock.waitForReadyRead(5000))
            {
                continue;
            }
            break;
        }
        if(line.trimmed().isEmpty())
        {
            break;
        }
        html = html | (line.indexOf("Content-Type: text/html") == 0);
        if(line.indexOf("Content-Length: ") == 0)
        {
            contentLen = line.remove(0, 16).trimmed().toInt(0, 10);
        }
    }

    if(html)
    {
        QByteArray text = sock.readAll();
        sock.close();
        if(text.length() == 0)
        {
            QMessageBox::critical(this, "PhonetiQ",
                                  tr("No response from ") + host);
            return false;
        }
        text.replace("</br>", "\n");
        if(QMessageBox::information(this, "PhonetiQ", text,
                                  QMessageBox::Retry | QMessageBox::Cancel) == QMessageBox::Retry)
        {
            goto connectx;
        }

        return false;
    }

    QFile f(destPath);
    if(!f.open(QFile::WriteOnly))
    {
        QMessageBox::critical(this, "PhonetiQ",
                              tr("Unable to save file:\r\n\r\n") + f.errorString());
        sock.close();
        return false;
    }

#ifdef QTOPIA
     QtopiaApplication::setPowerConstraint(QtopiaApplication::DisableSuspend);
#endif

    if(contentLen <= 0)
    {
        QMessageBox::critical(this, "PhonetiQ", tr("Couldnt read content length"));
        contentLen = 0x7fffffff;
    }
    progress->setMaximum(contentLen);
    progress->setValue(0);
    int remains = contentLen;

    char buf[65535];
    int count;
    for(;;)
    {
        QApplication::processEvents();
        count = sock.read(buf, 65535);
        if(count < 0)
        {
            break;
        }
        f.write(buf, count);
        f.flush();
        remains -= count;
        if(remains <= 0)
        {
            break;
        }
        progress->setValue(contentLen - remains);
    }
    f.close();
    sock.close();

#ifdef QTOPIA
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);
#endif

    return true;
}

bool VmTab::untar(QString file, QString display_fname, QString dest_dir)
{
    setStatus(tr("Unpacking") + " " + display_fname);

    progress->setMaximum(20);
    progress->setValue(0);

    QProcess gzip;
    //tar xzvf myarchive.tar.gz -C /to/dir
    gzip.start("tar", QStringList() << "xzvf" << file << "-C" << dest_dir);
    if(!gzip.waitForStarted())
    {
        showErr(tr("Unable to start 'tar'"));
        return false;
    }
    while(gzip.state() == QProcess::Running)
    {
        progress->setValue((progress->value() + 1) % 20);
        QApplication::processEvents();
        gzip.waitForFinished(100);
    }

    QFile::remove(file);
    return true;
}

bool VmTab::instlibs(QString msg)
{
    QProcess p;

    //------------------
    setStatus(tr("Executing") + " " + "'apt-get update'");

    progress->setMaximum(200);
    progress->setValue(0);

    p.start("apt-get", QStringList() << "-y" << "update");
    if(!p.waitForStarted())
    {
        showErr(tr("Unable to start 'apt-get update'"));
        return false;
    }
    while(p.state() == QProcess::Running)
    {
        progress->setValue((progress->value() + 1) % 200);
        QApplication::processEvents();
        p.waitForFinished(100);
    }
    //------------------

    setStatus(tr("Executing") + " " + "'apt-get install'");

    progress->setMaximum(200);
    progress->setValue(0);

    p.start("apt-get -y install " + msg);
    if(!p.waitForStarted())
    {
        showErr(tr("Unable to start 'apt-get install'"));
        return false;
    }
    while(p.state() == QProcess::Running)
    {
        progress->setValue((progress->value() + 1) % 200);
        QApplication::processEvents();
        p.waitForFinished(100);
    }

    return true;
}

//----------------------------------------------------------------------------

MidletsTab::MidletsTab(QWidget *parent)
    : QWidget(parent)
{
    lw = new QListWidget(this);
    //connect(lw, SIGNAL(clicked(QModelIndex)), this, SLOT(listClicked()));

    chb_qvga = new QCheckBox(this);
    chb_qvga->setText("QVGA");
    chb_fix = new QCheckBox(this);
    chb_fix->setText("Fixed scr size");
    chb_quitonhide = new QCheckBox(this);
    chb_quitonhide->setText("Quit on AUX");

    //connect(chb_qvga, SIGNAL(toggled(bool)), this, SLOT(chbToggled()));
    //connect(chb_fix, SIGNAL(toggled(bool)), this, SLOT(chbToggled()));
    //connect(chb_quitonhide, SIGNAL(toggled(bool)), this, SLOT(chbToggled()));

    lStatus = new QLabel(this);
    lStatus->setAlignment(Qt::AlignCenter);
    lStatus->setText(tr("List is not loaded"));

    bRefresh = new QPushButton(this);
    bRefresh->setText(tr("Refresh list"));
    connect(bRefresh, SIGNAL(clicked()), this, SLOT(refreshClicked()));

    bStop = new QPushButton(this);
    bStop->setText(tr("Stop midlet"));
    connect(bStop, SIGNAL(clicked()), this, SLOT(stopClicked()));
    bStop->setVisible(false);

    layout = new QGridLayout(this);

    layout->addWidget(lw, 0, 0, 1, 3);
    layout->addWidget(lStatus, 1, 0, 1, 3);
    layout->addWidget(bRefresh, 2, 0, 1, 3);
    layout->addWidget(chb_qvga, 3, 0);
    layout->addWidget(chb_fix, 3, 1);
    layout->addWidget(chb_quitonhide, 3, 2);
    layout->addWidget(bStop, 4, 0, 1, 3);

    setLayout(layout);
}

void MidletsTab::refreshClicked()
{
    updateConfig(false, false, false);

    QProcess script(this);
    script.start(BASE_DIR + JAVA_DIR + "/neo/listMidlets.sh");
    if(!script.waitForStarted())
    {
        showErr(tr("Unable to start 'listMidlets.sh'"));
    }
    else
    {
        while( !script.waitForFinished(100) )
        {
            QApplication::processEvents();
        }

        QString txt = script.readAll();
        lw->clear();
        int co=0;
        int p1=-1, p2=0, p3=0, p4=0;
        while (true)
        {
            p1 = txt.indexOf("Suite:", p1+1);
            if (p1<0) break;
            p1 += QString("Suite:").length() + 1;
            p2 = txt.indexOf("\n", p1);
            p3 = txt.indexOf("Name:", p2);
            p3 += QString("Name:").length() + 1;
            p4 = txt.indexOf("\n", p3);

            QString id = txt.mid(p1, p2-p1);
            QString name = txt.mid(p3, p4-p3);
            //qDebug() << id << " | " << name << "\n";

            QListWidgetItem *element = new QListWidgetItem(name);
            element->setData(Qt::UserRole, id.toInt());
            lw->addItem(element);
            co++;
        }

        lStatus->setText(tr("Number of installed midlets:") + " " + QString::number(co));
    }
}

void MidletsTab::stopClicked()
{
    abort=true;
}

void MidletsTab::runMidlet()
{
    if (bStop->isVisible())
        return;

    QListWidgetItem *sel = lw->currentItem();

    if(sel == NULL)
    {
        return;
    }

    lw->setVisible(false);
    lStatus->setVisible(false);
    bRefresh->setVisible(false);
    chb_qvga->setVisible(false);
    chb_fix->setVisible(false);
    chb_quitonhide->setVisible(false);
    bStop->setVisible(true);

    updateConfig(chb_qvga->isChecked(), chb_fix->isChecked(), chb_quitonhide->isChecked());

    abort=false;
    int id = sel->data(Qt::UserRole).toInt();
    QProcess script(this);
    script.start(BASE_DIR + JAVA_DIR + "/neo/runMidlet.sh " + QString::number(id));
    if(!script.waitForStarted())
    {
        showErr(tr("Unable to start 'runMidlet.sh'"));
    }
    else
    {
        while( !script.waitForFinished(500) )
        {
            QApplication::processEvents();
            if (abort)
            {
                script.kill();
                break;
            }
        }
    }
    
    lw->setVisible(true);
    lStatus->setVisible(true);
    bRefresh->setVisible(true);
    chb_qvga->setVisible(true);
    chb_fix->setVisible(true);
    chb_quitonhide->setVisible(true);
    bStop->setVisible(false);
}

void MidletsTab::removeMidlet()
{
    if (bStop->isVisible())
        return;

    QListWidgetItem *sel = lw->currentItem();

    if(sel == NULL)
    {
        return;
    }

    if(QMessageBox::question(this, "PhonetiQ",
            tr("Do you want to remove midlet") + " '" + sel->text() + "'?",
            QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
    {
        return;
    }

    updateConfig(false, false, false);

    int id = sel->data(Qt::UserRole).toInt();
    QProcess script(this);
    script.start(BASE_DIR + JAVA_DIR + "/neo/removeMidlet.sh " + QString::number(id));
    if(!script.waitForStarted())
    {
        showErr(tr("Unable to start 'removeMidlet.sh'"));
    }
    else
    {
        while( !script.waitForFinished(100) )
        {
            QApplication::processEvents();
        }
        refreshClicked();
    }
}

void MidletsTab::installMidlet()
{
    if (bStop->isVisible())
    return;

    js = new JarScanner(this);
    connect(js, SIGNAL(itemChosen(QString&)), this, SLOT(jarItemChosen(QString&)));
    js->show();
    js->scan();
}

void MidletsTab::jarItemChosen(QString &fname)
{
    js->close();

    if (fname.isEmpty())
    {
        QMessageBox::information(this, "PhonetiQ", "JAR file was not selected.");
        return;
    }

    updateConfig(false, false, false);

    QProcess script(this);
    script.start(BASE_DIR + JAVA_DIR + "/neo/installMidlet.sh file:///" + fname);
    if(!script.waitForStarted())
    {
        showErr(tr("Unable to start 'installMidlet.sh'"));
    }
    else
    {
        while( !script.waitForFinished(100) )
        {
            QApplication::processEvents();
        }
        refreshClicked();
    }
}

void MidletsTab::updateConfig(bool qvga, bool fix_scr, bool quitonhide)
{
    QSettings *runConf = new QSettings(BASE_DIR + JAVA_DIR + "/neo/run.conf", QSettings::IniFormat);

    runConf->beginGroup("Display");
    runConf->setValue("qvga", qvga);
    runConf->setValue("forceFullscreen", qvga);
    runConf->setValue("fixed", fix_scr);
    runConf->endGroup();

    runConf->beginGroup("Touchscreen");
    runConf->setValue("calibrate", qvga);
    runConf->endGroup();

    runConf->beginGroup("Font");
    runConf->setValue("noAntiAliasing", fix_scr);
    runConf->endGroup();

    runConf->beginGroup("Common");
    runConf->setValue("quitOnHide", quitonhide);
    runConf->endGroup();

    delete runConf;
}

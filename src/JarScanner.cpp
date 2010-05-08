#include "JarScanner.h"
#include <Qtopia>
#include <QDebug>

JarScanner::JarScanner(QWidget *parent) : QDialog(parent)
{
    setFixedSize(440,420);
    setModal(true);
    setWindowTitle(tr("JAR files scanner"));

    lw = new QListWidget(this);

    label = new QLabel(this);
    label->setText("");
    label->setVisible(false);

    progress = new QProgressBar(this);
    progress->setVisible(false);

    bInstall = new QPushButton(this);
    bInstall->setMinimumWidth(150);
    bInstall->setText(tr("Install"));

    layout=new QGridLayout(this);
    layout->addWidget(lw,0,0);
    layout->addWidget(label,1,0,Qt::AlignCenter);
    layout->addWidget(progress,2,0);
    layout->addWidget(bInstall,3,0,Qt::AlignCenter);

    connect (bInstall,SIGNAL(clicked()),this,SLOT(installClicked()));
}

//TODO: dialog close event handler

JarScanner::~JarScanner()
{
}

bool JarScanner::isDirectory(QString path)
{
    return path != NULL && (
#ifdef Q_WS_WIN
    (path.length() >= 3) && (path.at(1) == ':')
#else
    path.startsWith('/')
#endif
    || path.startsWith("http://"));
}

void JarScanner::installClicked()
{
    QListWidgetItem *sel = lw->currentItem();

    if(sel == NULL)
    {
        return;
    }

    QString dir = "";
    for(int i = 0; i < lw->count(); i++)
    {
        QListWidgetItem *item = lw->item(i);
        QString path = item->text();
        if (isDirectory(path))
        {
            if (item == sel)
            {
                dir = "";
                break;
            }
            dir = path;
        }
        if (item == sel)
            break;
    }

    QString filename = "";
    if (!dir.isEmpty())
        filename = dir + "/" + sel->text();

    emit itemChosen(filename);
}

void JarScanner::scan()
{
    label->setVisible(true);
    progress->setVisible(true);
    bInstall->setVisible(false);

    maxScanLevel = 0;

    //while(lw->count() > 2)
    //{
    //    delete(lw->takeItem(2));
    //}

    abort = false;

    progress->setMinimum(0);
    progress->setMaximum(0x8fffffff);

scan_files:

    lw->clear();

#ifdef Q_WS_WIN
    scanDir("c:\\", 0, maxScanLevel, 0, 0x1fffffff, false);
#else
    // For the first time scan /home/root/Documents and dont scan other dirs if
    // something found there.
    if(maxScanLevel == 0)
    {
        scanDir(QDir::homePath(), 0, 0, 0, 0x1fffffff, true);
        scanDir(QDir::homePath() + "/" + "Documents", 0, 2, 0, 0x2fffffff, true);
        scanDir("/media/card/Documents", 0, 2, 0, 0x3fffffff, true);
    }
    else
    {
        scanDir("/", 0, 0, 0, 0x1fffffff, true);
        scanDir("/mnt", 0, maxScanLevel + 1, 0, 0x2fffffff, false);
        scanDir("/media", 0, maxScanLevel + 1, 0, 0x3fffffff, false);
        scanDir(QDir::homePath(), 0, maxScanLevel, 0, 0x4fffffff, true);
        scanDir(QDir::homePath() + "/" + "Documents", 0, maxScanLevel+2, 0, 0x5fffffff, true);
        scanDir("/media/card/Documents", 0, maxScanLevel+2, 0, 0x6fffffff, true);
        scanDir("/root", 0, maxScanLevel, 0, 0x7fffffff, true);
    }
#endif

    if(!abort)
    {
        QString pref = "";
        if (lw->count() == 0)
            pref = tr("No JAR files found.");
        else
            pref = tr("A number of JAR files found.");
        bool more = QMessageBox::question(this, "PhonetiQ", pref + " " + tr("Scan more?"),
                        QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;
        if (more)
        {
            maxScanLevel++;
            goto scan_files;
        }
    }

    label->setVisible(false);
    progress->setVisible(false);
    bInstall->setVisible(true);
}

// Add item with directory name in list view.
QListWidgetItem* JarScanner::getDirItem(QListView *lw, QString dir)
{
    QListWidgetItem *res = new QListWidgetItem(dir);
    res->setIcon(QApplication::style()->standardIcon(QStyle::SP_DirIcon, 0, lw));
    return res;
}

int JarScanner::scanDir(QString const& path, int level, int maxLevel, int min, int max, bool followSymLinks)
{
    if(abort)
    {
        return 0;
    }

    QDir dir(path);
    QFileInfoList list = dir.entryInfoList(QDir::AllEntries, QDir::Name);

    int found = 0;
    int index = lw->count();
    for(int i = 0; i < list.count(); i++)
    {
        QFileInfo fi = list.at(i);
        if(!fi.isFile())
        {
            continue;
        }
        QString fileName = fi.fileName();
        if(fileName.endsWith(".jar", Qt::CaseInsensitive))
        {
            lw->addItem(fileName);
            found++;
        }
    }

    if(found)
    {
        lw->insertItem(index, getDirItem(lw, path));
    }

    if(level >= maxLevel)
    {
        return found;
    }

    for(int i = 0; i < list.count(); i++)
    {
        QFileInfo fi = list.at(i);
        if(fi.isFile() || fi.fileName() == "." || fi.fileName() == ".." ||
           (!followSymLinks && fi.isSymLink()))
        {
            continue;
        }

        int unit = (max - min) / list.count();
        int value = min + i * unit;
        if(progress->value() == value)
        {
            value++;
        }
        progress->setValue(value);
        label->setText(fi.absolutePath());
        QApplication::processEvents();

        found += scanDir(fi.filePath(), level + 1, maxLevel, value, value + unit, true);
    }
    return found;
}

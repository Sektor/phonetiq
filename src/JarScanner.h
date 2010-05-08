#ifndef JAR_SCANNER_H
#define JAR_SCANNER_H

#include <QDialog>
#include <QPushButton>
#include <QGridLayout>
#include <QListWidget>
#include <QProgressBar>
#include <QLabel>

class JarScanner : public QDialog
{
    Q_OBJECT
    private:
        QListWidget *lw;
        QProgressBar *progress;
        QPushButton *bInstall;
        QGridLayout *layout;
        QLabel *label;
        int maxScanLevel;
        bool abort;
        int scanDir(QString const& path, int level, int maxLevel, int min, int max, bool followSymLinks);
        QListWidgetItem* getDirItem(QListView *lw, QString dir);
        bool isDirectory(QString path);
    public:
        JarScanner(QWidget *);
        ~JarScanner();
        void scan();
    private slots:
        void installClicked();
    signals:
        void itemChosen(QString &fname);
};

#endif //JAR_SCANNER_H

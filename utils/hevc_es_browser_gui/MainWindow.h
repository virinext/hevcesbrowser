#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <QMainWindow>
#include <QMimeData>


class MainWindow: public QMainWindow
{
  Q_OBJECT
  public:
    MainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);
    void openFile(const QString &fileName);

  public slots:
    void slotOpen();
    void slotShowWarningsViewer();
    void slotShowInfoViewer();
    void slotShowHDRInfoViewer();
    void slotAbout();


private:
    void process(const QString &fileName);
    virtual void closeEvent(QCloseEvent *);
    void saveCustomData();
    void readCustomData();

    QWidget      *m_pwarnViewer;
    QWidget      *m_pinfoViewer;
    QWidget      *m_phdrInfoViewer;
};


#endif

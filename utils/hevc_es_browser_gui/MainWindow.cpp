#include "MainWindow.h"

#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QFileDialog>
#include <QMessageBox>
#include <QScopedArrayPointer>
#include <QDebug>
#include <QScrollArea>
#include <QLabel>
#include <QFile>
#include <QByteArray>
#include <QSettings>
#include <QDir>
#include <QMenu>
#include <QMenuBar>
#include <QProgressBar>
#include <QCoreApplication>


#include "CentralWidget.h"
#include "WarningsViewer.h"
#include "version_info.h"

#include <HevcParser.h>

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags):
  QMainWindow(parent, flags)
  ,m_pwarnViewer(new WarningsViewer)
{
  QToolBar *ptb = addToolBar("Menu");

  QAction *pactOpen = ptb -> addAction("Open...");
  pactOpen -> setShortcut(QKeySequence("Ctrl+O"));
  connect(pactOpen, SIGNAL(triggered()), SLOT(slotOpen()));

  QAction *pactShowWarn = ptb -> addAction("Warnings...");
  connect(pactShowWarn, SIGNAL(triggered()), SLOT(slotShowWarningsViewer()));


  QMenu *pmenu = menuBar() -> addMenu("&File");
  pmenu -> addAction(pactOpen);
  addAction (pactOpen);

  pmenu -> addSeparator();
  pmenu -> addAction(pactShowWarn);


  pmenu -> addSeparator();
  pmenu -> addAction("Exit", this, SLOT(close()));

  pmenu = menuBar() -> addMenu("&Help");
  pmenu -> addAction ("About HEVCESBrowser...", this, SLOT(slotAbout()));


  CentralWidget *pwgt = new CentralWidget(this);
  setCentralWidget(pwgt);
  
  readCustomData();
}


void MainWindow::process(const QString &fileName)
{
  QFile file(fileName);


  if(!file.open(QIODevice::ReadOnly))
  {
    QMessageBox::critical(this, "File opening problem", "Problem with open file `" + fileName + "`for reading");
    return;
  }

  HEVC::Parser *pparser = HEVC::Parser::create();
  CentralWidget *pcntwgt = dynamic_cast<CentralWidget *>(centralWidget());
  pparser -> addConsumer(pcntwgt -> m_pcomInfoViewer.data());
  pparser -> addConsumer(dynamic_cast<WarningsViewer *> (m_pwarnViewer));

  std::size_t position = 0;

  QProgressBar *pprogressBar = new QProgressBar(NULL);
  pprogressBar -> setWindowTitle("Opening...");
  pprogressBar -> setMinimum(0);
  pprogressBar -> setMaximum(file.size());
  pprogressBar -> setValue(0);

  Qt::WindowFlags flags = pprogressBar -> windowFlags();

  pprogressBar -> show();

  while(!file.atEnd())
  {
    QByteArray arr = file.read(4 * (1 << 20));
    std::size_t parsed = pparser -> process((const uint8_t *)arr.data(), arr.size(), position);
    position += parsed;

    file.seek(position);
    pprogressBar -> setValue(position);
    QCoreApplication::processEvents();
  }

  if(pcntwgt -> m_pcomInfoViewer -> rowCount() > 0)
  {
    QString lastOffset = pcntwgt -> m_pcomInfoViewer -> item(pcntwgt -> m_pcomInfoViewer -> rowCount() - 1, 0) -> text();
    std::size_t start = lastOffset.indexOf('(');
    std::size_t end = lastOffset.lastIndexOf(')');
    lastOffset = lastOffset.mid(start + 1);
    lastOffset.remove(")");
    std::size_t lastNalUOffset = lastOffset.toULongLong(NULL);
    pcntwgt -> m_pcomInfoViewer -> item(pcntwgt -> m_pcomInfoViewer -> rowCount() - 1, 1) -> setText(QString::number(position - lastNalUOffset));
  }

  pprogressBar -> close();
  delete pprogressBar;

  HEVC::Parser::release(pparser);
}


void MainWindow::slotOpen()
{
  QString dir = QDir::currentPath();
  QSettings settings("HEVCESBrowser", "HEVCESBrowser");

  if(settings.value("MainWindow/PrevDir").toString().length())
  {
    dir = settings.value("MainWindow/PrevDir").toString();

  }
  
  QString fileName = QFileDialog::getOpenFileName(this, "HEVC ES File", dir);
  if(!fileName.isEmpty())
  {
    CentralWidget *pcntwgt = dynamic_cast<CentralWidget *>(centralWidget());
    pcntwgt -> m_pcomInfoViewer -> clear();
    dynamic_cast<WarningsViewer *> (m_pwarnViewer) -> clear();

    process(fileName);
    QFileInfo info(fileName);
    settings.setValue("MainWindow/PrevDir", info.absoluteDir().absolutePath());
  }
}



void MainWindow::slotShowWarningsViewer()
{
  m_pwarnViewer -> show();
  m_pwarnViewer -> raise();
}


void MainWindow::closeEvent(QCloseEvent *pevent)
{
  saveCustomData();
  CentralWidget *pcntwgt = dynamic_cast<CentralWidget *>(centralWidget());
  pcntwgt -> m_pcomInfoViewer -> saveCustomData();
  
  m_pwarnViewer -> close();
  
  QWidget::closeEvent(pevent);
}


void MainWindow::saveCustomData()
{
  QSettings settings("HEVCESBrowser", "HEVCESBrowser");
  settings.setValue("MainWindow/geometry", saveGeometry());
}


void MainWindow::readCustomData()
{
  QSettings settings("HEVCESBrowser", "HEVCESBrowser");
  restoreGeometry(settings.value("MainWindow/geometry").toByteArray()); 
}

void MainWindow::slotAbout()
{
  QString message;
  message = "<center><b>HEVCESBrowser</b></center>";
  message += "<center>virinext@gmail.com</center>";
  message += QString("<center>Version: ") + VERSION_STR + "</center>";
  message += "<center>GUI Based on Qt</center>";
  QMessageBox::about(this, "About", message);
}

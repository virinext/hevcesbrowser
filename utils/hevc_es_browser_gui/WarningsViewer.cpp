#include "WarningsViewer.h"

#include <QSettings>


WarningsViewer::WarningsViewer(QWidget *pwgt):
  QTableWidget(0, 2, pwgt)
{
  QStringList labels;
  labels << "Offset" << "Message";
  setHorizontalHeaderLabels(labels);

  readCustomData();
}

void WarningsViewer::onNALUnit(std::shared_ptr<HEVC::NALUnit> pNALUnit, const HEVC::Parser::Info *pInfo)
{
}


void WarningsViewer::onWarning(const std::string &warning, const HEVC::Parser::Info *pInfo)
{
  int row = rowCount();
  insertRow(row);

  setItem(row, 0, new QTableWidgetItem(QString("0x"+ QString::number(pInfo -> m_position, 16) + " (" + QString::number(pInfo -> m_position) + ")")));
  setItem(row, 1, new QTableWidgetItem(QString(warning.c_str()))); 
}

void WarningsViewer::clear()
{
  setRowCount(0);
}


void WarningsViewer::closeEvent(QCloseEvent *pevent)
{
  saveCustomData();

  QWidget::closeEvent(pevent);
}




void WarningsViewer::saveCustomData()
{
  QSettings settings("HEVCESBrowser", "HEVCESBrowser");
  settings.setValue("WarningsViewer/geometry", saveGeometry());
  settings.setValue("WarningsViewer/OffsetWidth", columnWidth(0));
  settings.setValue("WarningsViewer/MessageWidth", columnWidth(1));
}


void WarningsViewer::readCustomData()
{
  QSettings settings("HEVCESBrowser", "HEVCESBrowser");
  restoreGeometry(settings.value("WarningsViewer/geometry").toByteArray());
  
  if(settings.contains("WarningsViewer/OffsetWidth"))
    setColumnWidth(0, settings.value("WarningsViewer/OffsetWidth").toInt());

  if(settings.contains("WarningsViewer/MessageWidth"))
    setColumnWidth(1, settings.value("WarningsViewer/MessageWidth").toInt());
}
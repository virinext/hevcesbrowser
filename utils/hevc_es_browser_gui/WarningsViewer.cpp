#include "WarningsViewer.h"

#include <algorithm>

#include <QSettings>
#include <QHBoxLayout>
#include <QVBoxLayout>


WarningsViewer::WarningsViewer(QWidget *pwgt)
{
  m_ptable = new QTableWidget(0, 2);

  QStringList labels;
  labels << "Offset" << "Message";
  m_ptable -> setHorizontalHeaderLabels(labels);

  m_filter = new QComboBox;
  m_filter -> addItem("All");
  m_filter -> addItem("Value out of range");
  m_filter -> addItem("Reference struct not present");
  m_filter -> addItem("Profile conformance");


  QHBoxLayout *phblay = new QHBoxLayout;
  phblay -> addStretch(1);
  phblay -> addWidget(m_filter);

  QVBoxLayout *pvblay = new QVBoxLayout;
  pvblay -> addLayout(phblay);
  pvblay -> addWidget(m_ptable);

  setLayout(pvblay);

  readCustomData();

  QObject::connect(m_filter, SIGNAL(currentIndexChanged(int)), this, SLOT(filterItemChanged(int)));
}

void WarningsViewer::onNALUnit(std::shared_ptr<HEVC::NALUnit> pNALUnit, const HEVC::Parser::Info *pInfo)
{
}


void WarningsViewer::onWarning(const std::string &warning, const HEVC::Parser::Info *pInfo, HEVC::Parser::WarningType type)
{
  Warning warn;
  warn.m_position = pInfo -> m_position;
  warn.m_message = QString(warning.c_str());
  warn.m_type = type;

  m_warnings.push_back(warn);

  std::sort(m_warnings.begin(), m_warnings.end());

  if(isVisible())
    update();

}

void WarningsViewer::clear()
{
  m_warnings.clear();
  m_ptable -> setRowCount(0);
}


void WarningsViewer::showEvent(QShowEvent* pevent)
{
  update();
  QWidget::showEvent(pevent);
}

void WarningsViewer::update()
{
  m_ptable -> setRowCount(0);

  HEVC::Parser::WarningType filterType;

  if(m_filter -> currentText() == "Value out of range")
    filterType = HEVC::Parser::OUT_OF_RANGE;
  else if(m_filter -> currentText() == "Reference struct not present")
    filterType = HEVC::Parser::REFERENCE_STRUCT_NOT_PRESENT;
  else if(m_filter -> currentText() == "Profile conformance")
    filterType = HEVC::Parser::PROFILE_CONFORMANCE;
  else
    filterType = HEVC::Parser::NONE;

  for(std::size_t i=0; i<m_warnings.size(); i++)
  {
    if(filterType == m_warnings[i].m_type || filterType == HEVC::Parser::NONE)
    {
      int row = m_ptable -> rowCount();
      m_ptable -> insertRow(row);

      m_ptable -> setItem(row, 0, new QTableWidgetItem(QString("0x"+ QString::number(m_warnings[i].m_position, 16) + " (" + QString::number(m_warnings[i].m_position) + ")")));
      m_ptable -> setItem(row, 1, new QTableWidgetItem(m_warnings[i].m_message));

      m_ptable -> item(row, 0) -> setFlags(m_ptable -> item(row, 0) -> flags() & ~Qt::ItemIsEditable);
      m_ptable -> item(row, 1) -> setFlags(m_ptable -> item(row, 1) -> flags() & ~Qt::ItemIsEditable);
    }
  }
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
  settings.setValue("WarningsViewer/OffsetWidth", m_ptable -> columnWidth(0));
  settings.setValue("WarningsViewer/MessageWidth", m_ptable -> columnWidth(1));
}


void WarningsViewer::readCustomData()
{
  QSettings settings("HEVCESBrowser", "HEVCESBrowser");
  restoreGeometry(settings.value("WarningsViewer/geometry").toByteArray());

  if(settings.contains("WarningsViewer/OffsetWidth"))
    m_ptable -> setColumnWidth(0, settings.value("WarningsViewer/OffsetWidth").toInt());

  if(settings.contains("WarningsViewer/MessageWidth"))
    m_ptable -> setColumnWidth(1, settings.value("WarningsViewer/MessageWidth").toInt());
}


void WarningsViewer::filterItemChanged(int)
{
  update();
}

bool WarningsViewer::Warning::operator < (const Warning& rhs) const
{
  return m_position < rhs.m_position;
}

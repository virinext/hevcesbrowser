#include "CentralWidget.h"

#include <QHBoxLayout>
#include <QHeaderView>

#include <QMetaType>

#include <QSplitter>
#include <QSettings>

#include "types.h"

//typedef

CentralWidget::CentralWidget(QWidget *pwgt):
  QWidget(pwgt)
  ,m_pcomInfoViewer(new CommonInfoViewer)
  ,m_psyntaxViewer(new SyntaxViewer)
  ,m_phexViewer(new QHexView)
{
    m_psplitterH = new QSplitter;
    m_psplitterH->addWidget(m_pcomInfoViewer.data());
    m_psplitterH->addWidget(m_phexViewer.data());

    m_psplitterH->setOrientation(Qt::Vertical);

    m_psplitterV = new QSplitter;
    m_psplitterV->addWidget(m_psplitterH);
    m_psplitterV->addWidget(m_psyntaxViewer.data());
    m_psplitterV->setOrientation(Qt::Horizontal);

    QHBoxLayout *phblay = new QHBoxLayout;
    phblay -> addWidget(m_psplitterV);

    m_psyntaxViewer -> header()->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_psyntaxViewer -> header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_psyntaxViewer -> header()->setStretchLastSection(false);

    QObject::connect(m_pcomInfoViewer.data(),
      SIGNAL(naluSelected(std::shared_ptr<HEVC::NALUnit>, ParserInfo)),
      m_psyntaxViewer.data(),
      SLOT(onNalUChanged(std::shared_ptr<HEVC::NALUnit>, ParserInfo)));

    QObject::connect(m_pcomInfoViewer.data(),
      SIGNAL(parameterSetsChanged(const VPSMap &, const SPSMap &, const PPSMap &)),
      m_psyntaxViewer.data(),
      SLOT(setParameretsSets(const VPSMap &, const SPSMap &, const PPSMap &)));

    QObject::connect(m_pcomInfoViewer.data(),
      SIGNAL(naluSelected(std::shared_ptr<HEVC::NALUnit>, ParserInfo)),
      this,
      SLOT(setHexOffset(std::shared_ptr<HEVC::NALUnit>, ParserInfo)));


    setLayout(phblay);

    readCustomData();
}


CentralWidget::~CentralWidget()
{
  saveCustomData();
}


void CentralWidget::setHexOffset(std::shared_ptr<HEVC::NALUnit>, ParserInfo info)
{
  m_phexViewer -> showFromOffset(info.m_position);
  m_phexViewer->setFocus();

}


void CentralWidget::saveCustomData()
{
  QSettings settings("HEVCESBrowser", "HEVCESBrowser");
  settings.setValue("CentralWidget/HoriozntalSplitter/geometry", m_psplitterH -> saveState());
  settings.setValue("CentralWidget/VerticalSplitter/geometry", m_psplitterV -> saveState());
}


void CentralWidget::readCustomData()
{
  QSettings settings("HEVCESBrowser", "HEVCESBrowser");
  m_psplitterH -> restoreState(settings.value("CentralWidget/HoriozntalSplitter/geometry").toByteArray());
  m_psplitterV -> restoreState(settings.value("CentralWidget/VerticalSplitter/geometry").toByteArray());
}

#include "CentralWidget.h"

#include <QHBoxLayout>
#include <QHeaderView>

#include <QMetaType>

#include "types.h"

//typedef 

CentralWidget::CentralWidget(QWidget *pwgt):
  QWidget(pwgt)
  ,m_pcomInfoViewer(new CommonInfoViewer)
  ,m_psyntaxViewer(new SyntaxViewer)
{
    QHBoxLayout *phblay = new QHBoxLayout;
    phblay -> addWidget(m_pcomInfoViewer.data(), 2);
    phblay -> addWidget(m_psyntaxViewer.data());
    
    m_psyntaxViewer -> setMinimumWidth(375); 

    m_psyntaxViewer -> header()->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_psyntaxViewer -> header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_psyntaxViewer -> header()->setStretchLastSection(false); 

    QObject::connect(m_pcomInfoViewer.data(),
      SIGNAL(naluSelected(std::shared_ptr<HEVC::NALUnit>)),
      m_psyntaxViewer.data(),
      SLOT(onNalUChanged(std::shared_ptr<HEVC::NALUnit>)));

    QObject::connect(m_pcomInfoViewer.data(),
      SIGNAL(parameterSetsChanged(const VPSMap &, const SPSMap &, const PPSMap &)),
      m_psyntaxViewer.data(),
      SLOT(setParameretsSets(const VPSMap &, const SPSMap &, const PPSMap &)));

    setLayout(phblay);
}

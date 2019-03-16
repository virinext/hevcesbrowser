#ifndef CENTRAL_WIDGET_H_
#define CENTRAL_WIDGET_H_

#include <QWidget>
#include <QSharedPointer>
#include <QSplitter>

#include <QHexView.h>

#include "CommonInfoViewer.h"
#include "SyntaxViewer.h"

class CentralWidget: public QWidget
{
  Q_OBJECT

  public:
    CentralWidget(QWidget *pwgt = NULL);
    ~CentralWidget();

    QSharedPointer<CommonInfoViewer>               m_pcomInfoViewer;
    QSharedPointer<SyntaxViewer>                   m_psyntaxViewer;
    QSharedPointer<QHexView>                       m_phexViewer;


    void saveCustomData();
  private:
  	void readCustomData();

  private slots:
    void setHexOffset(std::shared_ptr<HEVC::NALUnit>, ParserInfo);

  private:
    QSplitter                                      *m_psplitterV;
    QSplitter                                      *m_psplitterH;
};


#endif

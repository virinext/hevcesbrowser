#ifndef CENTRAL_WIDGET_H_
#define CENTRAL_WIDGET_H_

#include <QWidget>
#include <QSharedPointer>

#include "CommonInfoViewer.h"
#include "SyntaxViewer.h"

class CentralWidget: public QWidget
{
  Q_OBJECT
  
  public:
    CentralWidget(QWidget *pwgt = NULL);
    QSharedPointer<CommonInfoViewer>               m_pcomInfoViewer;
    QSharedPointer<SyntaxViewer>                   m_psyntaxViewer;
};


#endif

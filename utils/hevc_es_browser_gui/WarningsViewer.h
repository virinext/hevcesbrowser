#ifndef WARNINGS_VIEWER_H_
#define WARNINGS_VIEWER_H_

#include <QTableWidget>
#include <HevcParser.h>


class WarningsViewer:
  public QTableWidget,
  public HEVC::Parser::Consumer
{
  public:
    WarningsViewer(QWidget *pwgt = NULL);
    
    void clear();
    
    virtual void onNALUnit(std::shared_ptr<HEVC::NALUnit> pNALUnit, const HEVC::Parser::Info *pInfo);

  private:
    void readCustomData();
    void saveCustomData();

    void closeEvent(QCloseEvent* pevent);
};


#endif

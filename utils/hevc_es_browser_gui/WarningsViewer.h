#ifndef WARNINGS_VIEWER_H_
#define WARNINGS_VIEWER_H_

#include <QTableWidget>
#include <QWidget>
#include <QComboBox>
#include <QShowEvent>

#include <HevcParser.h>


class WarningsViewer:
  public QWidget,
  public HEVC::Parser::Consumer
{
  Q_OBJECT
  public:
    WarningsViewer(QWidget *pwgt = NULL);

    void clear();

    virtual void onNALUnit(std::shared_ptr<HEVC::NALUnit> pNALUnit, const HEVC::Parser::Info *pInfo);
    virtual void onWarning(const std::string &warning, const HEVC::Parser::Info *pInfo, HEVC::Parser::WarningType);

  public slots:
    void filterItemChanged(int);


  protected:
    virtual void showEvent(QShowEvent* pevent);

  private:
    struct Warning
    {
      uint32_t                    m_position;
      QString                     m_message;
      HEVC::Parser::WarningType   m_type;
      bool operator < (const Warning& rhs) const;
    };

    std::vector<Warning>        m_warnings;
    QTableWidget               *m_ptable;
    QComboBox                  *m_filter;


    void update();

    void readCustomData();
    void saveCustomData();

    void closeEvent(QCloseEvent* pevent);

};


#endif

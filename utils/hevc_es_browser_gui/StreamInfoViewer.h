#ifndef STREAM_INFO_VIEWER_H_
#define STREAM_INFO_VIEWER_H_


#include <QListWidget>
#include <QShowEvent>

#include <HevcParser.h>

class StreamInfoViewer:
  public QListWidget,
  public HEVC::Parser::Consumer
{
  public:
    StreamInfoViewer(QWidget *pwgt = NULL);

    void clear();

    virtual void onNALUnit(std::shared_ptr<HEVC::NALUnit> pNALUnit, const HEVC::Parser::Info *pInfo);
    virtual void onWarning(const std::string &warning, const HEVC::Parser::Info *pInfo, HEVC::Parser::WarningType);

  protected:
    virtual void showEvent(QShowEvent* pevent);
  private:
    void update();

    void readCustomData();
    void saveCustomData();

    virtual void closeEvent(QCloseEvent* pevent);

    std::size_t      m_nalusNumber;
    std::size_t      m_INumber;
    std::size_t      m_PNumber;
    std::size_t      m_BNumber;
    std::size_t      m_profile;
    std::size_t      m_level;
    std::size_t      m_tier;

    HEVC::Slice::SliceType    m_prevSliceType;
};


#endif

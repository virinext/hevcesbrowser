#include "StreamInfoViewer.h"

#include <QSettings>

#include <limits>

#include <QDebug>


StreamInfoViewer::StreamInfoViewer(QWidget *pwgt):
  QListWidget(pwgt)
{
  QStringList labels;

  readCustomData();

  clear();
}

void StreamInfoViewer::onNALUnit(std::shared_ptr<HEVC::NALUnit> pNALUnit, const HEVC::Parser::Info *pInfo)
{
  m_nalusNumber++;

  switch(pNALUnit -> m_nalHeader.type)
  {
    case HEVC::NAL_VPS:
    {
      std::shared_ptr<HEVC::VPS> pVPS = std::dynamic_pointer_cast<HEVC::VPS>(pNALUnit);

      m_profile = pVPS -> profile_tier_level.general_profile_idc;
      m_tier = pVPS -> profile_tier_level.general_tier_flag;
      m_level = pVPS -> profile_tier_level.general_level_idc;
      break;
    }

    case HEVC::NAL_IDR_W_RADL:
    case HEVC::NAL_IDR_N_LP:
    case HEVC::NAL_TRAIL_R:
    case HEVC::NAL_TSA_R:
    case HEVC::NAL_STSA_R:
    case HEVC::NAL_RADL_R:
    case HEVC::NAL_RASL_R:
    case HEVC::NAL_TRAIL_N:
    case HEVC::NAL_TSA_N:
    case HEVC::NAL_STSA_N:
    case HEVC::NAL_RADL_N:
    case HEVC::NAL_RASL_N:
    case HEVC::NAL_BLA_W_LP:
    case HEVC::NAL_BLA_W_RADL:
    case HEVC::NAL_BLA_N_LP:
    case HEVC::NAL_CRA_NUT:
    {
      std::shared_ptr<HEVC::Slice> pSlice = std::dynamic_pointer_cast<HEVC::Slice>(pNALUnit);

      if(pSlice -> dependent_slice_segment_flag)
      {
        switch(m_prevSliceType)
        {
          case HEVC::Slice::B_SLICE:
            m_BNumber++;
            break;
          case HEVC::Slice::P_SLICE:
            m_PNumber++;
            break;
          case HEVC::Slice::I_SLICE:
            m_INumber++;
            break;

          case HEVC::Slice::NONE_SLICE:
            break;
        }
      }
      else
      {
        switch(pSlice -> slice_type)
        {
          case HEVC::Slice::B_SLICE:
            m_BNumber++;
            break;
          case HEVC::Slice::P_SLICE:
            m_PNumber++;
            break;
          case HEVC::Slice::I_SLICE:
            m_INumber++;
            break;
        };

        m_prevSliceType = (HEVC::Slice::SliceType) pSlice -> slice_type;
      }
      break;
    }
    default:
      break;
  };


  if(isVisible())
    update();
}


void StreamInfoViewer::onWarning(const std::string &warning, const HEVC::Parser::Info *pInfo, HEVC::Parser::WarningType)
{
}

void StreamInfoViewer::clear()
{
  QListWidget::clear();
  m_nalusNumber = 0;
  m_INumber = 0;
  m_PNumber = 0;
  m_BNumber = 0;
  m_prevSliceType = HEVC::Slice::NONE_SLICE;
  m_profile = std::numeric_limits<std::size_t>::max();
  m_tier = std::numeric_limits<std::size_t>::max();
  m_level = std::numeric_limits<std::size_t>::max();
}


void StreamInfoViewer::closeEvent(QCloseEvent *pevent)
{
  saveCustomData();

  QWidget::closeEvent(pevent);
}


void StreamInfoViewer::showEvent(QShowEvent* pevent)
{
  update();
  QWidget::showEvent(pevent);
}


void StreamInfoViewer::update()
{
  QListWidget::clear();
  std::size_t slicesNumber = m_INumber + m_BNumber + m_PNumber;

  addItem(QString("NALUs: ") + QString::number(m_nalusNumber));
  addItem(QString("Slices: ") + QString::number(slicesNumber));
  addItem(QString("I: ") + QString::number(m_INumber) + " (" + QString::number((double)m_INumber * 100 / slicesNumber) + " %)");
  addItem(QString("P: ") + QString::number(m_PNumber) + " (" + QString::number((double)m_PNumber * 100 / slicesNumber) + " %)");
  addItem(QString("B: ") + QString::number(m_BNumber) + " (" + QString::number((double)m_BNumber * 100 / slicesNumber) + " %)");

  QString profile;
  QString level;
  QString tier;

  switch(m_profile)
  {
    case 1:
      profile = "Main";
      break;
    case 2:
      profile = "Main 10";
      break;
    case 3:
      profile = "Main Still Picture";
      break;

    case std::numeric_limits<std::size_t>::max():
      profile = "NOT PRESENT";
      break;

    default:
      profile = QString::number(m_profile) + " (UNKNOWN)";
      break;
  }

  switch(m_tier)
  {
    case 0:
      tier = "Main";
      break;
    case 1:
      tier = "High";
      break;
    case std::numeric_limits<std::size_t>::max():
      tier = "NOT PRESENT";
      break;
  }

  if(m_level != std::numeric_limits<std::size_t>::max())
    level = QString::number((double)m_level / 30);
  else
    level = "NOT PRESENT";

  addItem(QString("Profile: ") + profile);
  addItem(QString("Level: ") + level);
  addItem(QString("Tier: ") + tier);
}

void StreamInfoViewer::saveCustomData()
{
  QSettings settings("HEVCESBrowser", "HEVCESBrowser");
  settings.setValue("StreamInfoViewer/geometry", saveGeometry());
}


void StreamInfoViewer::readCustomData()
{
  QSettings settings("HEVCESBrowser", "HEVCESBrowser");
  restoreGeometry(settings.value("StreamInfoViewer/geometry").toByteArray());
}

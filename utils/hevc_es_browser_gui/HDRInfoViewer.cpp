#include "HDRInfoViewer.h"

#include <QSettings>

#include <limits>

#include <QDebug>



HDRInfoViewer::HDRInfoViewer(QWidget *pwgt):
  QListWidget(pwgt)
{
  QStringList labels;

  readCustomData();

  clear();
}

void HDRInfoViewer::onNALUnit(std::shared_ptr<HEVC::NALUnit> pNALUnit, const HEVC::Parser::Info *pInfo)
{
  switch(pNALUnit -> m_nalHeader.type)
  {
    case HEVC::NAL_SPS:
    {
      m_psps = std::dynamic_pointer_cast<HEVC::SPS>(pNALUnit);
      break;
    }

    case HEVC::NAL_SEI_PREFIX:
    {
      std::shared_ptr<HEVC::SEI> pSEI = std::dynamic_pointer_cast<HEVC::SEI>(pNALUnit);
      for(std::size_t i=0; i< pSEI -> sei_message.size(); i++)
      {
        std::size_t payloadType = 0;

        for(std::size_t i=0; i<pSEI -> sei_message[i].num_payload_type_ff_bytes; i++)
        {
          payloadType += 255;
        }

        payloadType += pSEI -> sei_message[i].last_payload_type_byte;

        switch(payloadType)
        {
          case HEVC::SeiMessage::MASTERING_DISPLAY_INFO:
            m_pMasteringDisplayInfo = std::dynamic_pointer_cast<HEVC::MasteringDisplayInfo>(pSEI -> sei_message[i].sei_payload);
            break;

          case HEVC::SeiMessage::CONTENT_LIGHT_LEVEL_INFO:
             m_pCLLInfo = std::dynamic_pointer_cast<HEVC::ContentLightLevelInfo>(pSEI -> sei_message[i].sei_payload);

          default:
            break;
        }
      }

      break;
    }

    default:
      break;
  };


  if(isVisible())
    update();
}


void HDRInfoViewer::onWarning(const std::string &warning, const HEVC::Parser::Info *pInfo, HEVC::Parser::WarningType)
{
}

void HDRInfoViewer::clear()
{
  QListWidget::clear();

  m_psps.reset();
}


void HDRInfoViewer::closeEvent(QCloseEvent *pevent)
{
  saveCustomData();

  QWidget::closeEvent(pevent);
}


void HDRInfoViewer::showEvent(QShowEvent* pevent)
{
  update();
  QWidget::showEvent(pevent);
}


QString colourPrimariesToString(int value)
{
  QString result;

  switch(value)
  {
    case 1:
      result = "bt709";
      break;
    case 2:
      result = "Unspecified";
      break;
    case 4:
      result = "bt470m";
      break;
    case 5:
      result = "bt470bg";
      break;
    case 6:
      result = "smpte170m";
      break;
    case 7:
      result = "smpte240m";
      break;
    case 8:
      result = "film";
      break;
    case 9:
      result = "bt2020";
      break;

    case 0:
    case 3:
    default:
      result = "Reserved";
      break;
  }

  return result;
}

QString transferCharacteristicsToString(int value)
{
  QString result;

  switch(value)
  {
    case 1:
      result = "bt709";
      break;
    case 2:
      result = "Unspecified";
      break;
    case 4:
      result = "bt470m";
      break;
    case 5:
      result = "bt470bg";
      break;
    case 6:
      result = "smpte170m";
      break;
    case 7:
      result = "smpte240m";
      break;
    case 8:
      result = "linear";
      break;
    case 9:
      result = "log100";
      break;
    case 10:
      result = "log316";
      break;
    case 11:
      result = "iec61966-2-4";
      break;
    case 12:
      result = "bt1361e";
      break;
    case 13:
      result = "iec61966-2-1";
      break;
    case 14:
      result = "bt2020-10";
      break;
    case 15:
      result = "bt2020-12";
      break;
    case 16:
      result = "smpte-st-2084";
      break;
    case 17:
      result = "smpte-st-428";
      break;
    case 18:
      result = "arib-std-b67";
      break;

    case 0:
    case 3:
    default:
      result = "Reserved";
      break;
  }

  return result;
}

QString matrixCoeficientsToString(int value)
{
  QString result;

  switch(value)
  {
    case 0:
      result = "GBR";
      break;
    case 1:
      result = "bt709";
      break;
    case 2:
      result = "Unspecified";
      break;
    case 4:
      result = "fcc";
      break;
    case 5:
      result = "bt470bg";
      break;
    case 6:
      result = "smpte170m";
      break;
    case 7:
      result = "smpte240m";
      break;
    case 8:
      result = "YCgCo";
      break;
    case 9:
      result = "bt2020nc";
      break;
    case 10:
      result = "bt2020c";
      break;

    case 3:
    default:
      result = "Reserved";
      break;
  }

  return result;
}

void HDRInfoViewer::update()
{
  QListWidget::clear();

  int colour_primaries = 2;
  int transfer_characteristics = 2;
  int matrix_coeffs = 2;

  if(m_psps && m_psps -> vui_parameters_present_flag &&
      m_psps -> vui_parameters.video_signal_type_present_flag &&
      m_psps -> vui_parameters.colour_description_present_flag)
  {
    colour_primaries = m_psps -> vui_parameters.colour_primaries;
    transfer_characteristics = m_psps -> vui_parameters.transfer_characteristics;
    matrix_coeffs = m_psps -> vui_parameters.matrix_coeffs;
  }

  int chroma_loc_top = 0;
  int chroma_loc_bottom = 0;
  if(m_psps && m_psps -> vui_parameters_present_flag &&
    m_psps -> vui_parameters.chroma_loc_info_present_flag)
  {
    chroma_loc_top = m_psps -> vui_parameters.chroma_sample_loc_type_top_field;
    chroma_loc_bottom = m_psps -> vui_parameters.chroma_sample_loc_type_bottom_field;
  }

  int video_full_range_flag = 0;
  if(m_psps && m_psps -> vui_parameters_present_flag &&
    m_psps -> vui_parameters.video_signal_type_present_flag)
  {
    video_full_range_flag = m_psps -> vui_parameters.video_full_range_flag;
  }

  addItem(QString("Colour primaries: ") +colourPrimariesToString(colour_primaries));
  addItem(QString("Transfer characteristics: ") + transferCharacteristicsToString(transfer_characteristics));
  addItem(QString("Matrix coefficients: ") + matrixCoeficientsToString(matrix_coeffs));
  addItem(QString("Chroma loc(top): ") + QString::number(chroma_loc_top));
  addItem(QString("Chroma loc(bot): ") + QString::number(chroma_loc_bottom));
  addItem(QString("Full range: ") + QString::number(video_full_range_flag));

  if(m_pCLLInfo)
  {
    addItem(QString("Max CLL: ") + colourPrimariesToString(m_pCLLInfo -> max_content_light_level));
    addItem(QString("Avg CLL: ") + colourPrimariesToString(m_pCLLInfo -> max_pic_average_light_level));
  }

  if(m_pMasteringDisplayInfo)
  {
    QString mdiStr = QString("G(%1,%2), B(%3,%4), R(%5,%6), WP(%7,%8),L(%9,%10)")
        .arg(m_pMasteringDisplayInfo->display_primary_x[0])
        .arg(m_pMasteringDisplayInfo->display_primary_y[0])
        .arg(m_pMasteringDisplayInfo->display_primary_x[1])
        .arg(m_pMasteringDisplayInfo->display_primary_y[1])
        .arg(m_pMasteringDisplayInfo->display_primary_x[2])
        .arg(m_pMasteringDisplayInfo->display_primary_y[2])
        .arg(m_pMasteringDisplayInfo->white_point_x)
        .arg(m_pMasteringDisplayInfo->white_point_y)
        .arg(m_pMasteringDisplayInfo->max_display_mastering_luminance)
        .arg(m_pMasteringDisplayInfo->min_display_mastering_luminance);
    addItem(QString("Mastering display: ") + mdiStr);
  }
}

void HDRInfoViewer::saveCustomData()
{
  QSettings settings("HEVCESBrowser", "HEVCESBrowser");
  settings.setValue("HDRInfoViewer/geometry", saveGeometry());
}


void HDRInfoViewer::readCustomData()
{
  QSettings settings("HEVCESBrowser", "HEVCESBrowser");
  restoreGeometry(settings.value("HDRInfoViewer/geometry").toByteArray());
}
